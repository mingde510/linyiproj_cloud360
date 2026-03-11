// TrtYoloDetector.cpp
#include "TrtYoloDetector.hpp"
#include <NvInfer.h>
#include <cuda_runtime_api.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <algorithm>
#include <numeric>

using namespace nvinfer1;

namespace {

// Logger
class TrtLogger : public ILogger {
public:
    void log(Severity severity, const char* msg) noexcept override {
        if (severity <= Severity::kWARNING) {
            std::cout << "[TRT] " << msg << std::endl;
        }
    }
} gLogger;

// cuda check
#define CHECK_CUDA(x) do { \
    cudaError_t err = (x); \
    if (err != cudaSuccess) { \
        std::cerr << "CUDA Error: " << cudaGetErrorString(err) \
                  << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        std::exit(-1); \
    } \
} while(0)

static inline float iou(const cv::Rect2f& a, const cv::Rect2f& b){
    float inter = (a & b).area();
    float uni = a.area() + b.area() - inter;
    return uni <= 0.f ? 0.f : inter / uni;
}

// NMS
std::vector<int> nms(const std::vector<cv::Rect2f>& boxes,
                     const std::vector<float>& scores,
                     float iouThr) {
    std::vector<int> idx(boxes.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(),
              [&](int i, int j){ return scores[i] > scores[j]; });

    std::vector<int> keep;
    std::vector<char> dead(boxes.size(), 0);
    for (size_t _i = 0; _i < idx.size(); ++_i) {
        int i = idx[_i];
        if (dead[i]) continue;
        keep.push_back(i);
        for (size_t _j = _i + 1; _j < idx.size(); ++_j) {
            int j = idx[_j];
            if (dead[j]) continue;
            if (iou(boxes[i], boxes[j]) > iouThr) dead[j] = 1;
        }
    }
    return keep;
}

// 读取 engine
std::vector<char> loadFile(const std::string& path){
    std::ifstream fin(path, std::ios::binary);
    if(!fin) throw std::runtime_error("cannot open engine file");
    fin.seekg(0, std::ios::end);
    size_t sz = fin.tellg();
    fin.seekg(0, std::ios::beg);
    std::vector<char> data(sz);
    fin.read(data.data(), sz);
    return data;
}

} // namespace

TrtYoloDetector::TrtYoloDetector(const std::string& enginePath, int inputW, int inputH)
    : enginePath_(enginePath), inW_(inputW), inH_(inputH) {}

TrtYoloDetector::~TrtYoloDetector() {
    if (dInput_)  CHECK_CUDA(cudaFree(dInput_));
    if (dOutput_) CHECK_CUDA(cudaFree(dOutput_));
    if (stream_)  CHECK_CUDA(cudaStreamDestroy((cudaStream_t)stream_));

    if (context_) ((IExecutionContext*)context_)->destroy();
    if (engine_)  ((ICudaEngine*)engine_)->destroy();
    if (runtime_) ((IRuntime*)runtime_)->destroy();
}

bool TrtYoloDetector::init() {
    auto engineData = loadFile(enginePath_);
    runtime_ = createInferRuntime(gLogger);
    if(!runtime_) return false;

    engine_ = ((IRuntime*)runtime_)->deserializeCudaEngine(engineData.data(), engineData.size());
    if(!engine_) return false;

    context_ = ((ICudaEngine*)engine_)->createExecutionContext();
    if(!context_) return false;

    // binding
    auto* eng = (ICudaEngine*)engine_;
    int nb = eng->getNbBindings();
    if(nb < 2){
        std::cerr << "engine bindings < 2\n";
        return false;
    }

    inputIndex_  = -1;
    outputIndex_ = -1;
    for(int i=0;i<nb;i++){
        if(eng->bindingIsInput(i)) inputIndex_ = i;
        else outputIndex_ = i;
    }
    if(inputIndex_<0 || outputIndex_<0){
        std::cerr << "cannot find input/output binding\n";
        return false;
    }

    // input dims
    auto inDims = eng->getBindingDimensions(inputIndex_);
    // expect 1x3xH xW (explicit batch)
    if(inDims.nbDims != 4){
        std::cerr << "unexpected input dims\n";
        return false;
    }
    batch_ = inDims.d[0];
    c_     = inDims.d[1];
    inH_   = inDims.d[2];
    inW_   = inDims.d[3];

    auto outDims = eng->getBindingDimensions(outputIndex_);
    if (outDims.nbDims != 3) {
        std::cerr << "unexpected output dims, nbDims=" << outDims.nbDims << "\n";
        return false;
    }

    // 你的模型应该是 [1,5,8400]
    int d1 = outDims.d[1];
    int d2 = outDims.d[2];

    if (d1 <= 10 && d2 > 10) {      // d1=5, d2=8400
        outC_ = d1;
        outN_ = d2;
    } else {                        // fallback: [1,8400,5]
        outN_ = d1;
        outC_ = d2;
    }

    std::cout << "[INFO] TRT output: " << batch_<<"x"<<outC_<<"x"<<outN_ << "\n";
    // allocate GPU buffers
    size_t inSize  = batch_ * c_ * inH_ * inW_ * sizeof(float);
    size_t outSize = batch_ * outC_ * outN_ * sizeof(float);
    CHECK_CUDA(cudaMalloc(&dOutput_, outSize));
    hOutput_.resize(batch_ * outC_ * outN_);


    std::cout << "[INFO] TRT input: " << batch_<<"x"<<c_<<"x"<<inH_<<"x"<<inW_ << "\n";
    std::cout << "[INFO] TRT output: " << batch_<<"x"<<outN_<<"x"<<outC_ << "\n";

    CHECK_CUDA(cudaMalloc(&dInput_, inSize));
    CHECK_CUDA(cudaMalloc(&dOutput_, outSize));
    CHECK_CUDA(cudaStreamCreate((cudaStream_t*)&stream_));

    hOutput_.resize(batch_ * outN_ * outC_);

    return true;
}

std::vector<Detection> TrtYoloDetector::infer(const cv::Mat& bgr) {
    if(!context_) return {};

    // preprocess to GPU input
    preprocess(bgr, (float*)dInput_);

    void* bindings[2];
    bindings[inputIndex_]  = dInput_;
    bindings[outputIndex_] = dOutput_;

    auto* ctx = (IExecutionContext*)context_;
    bool ok = ctx->enqueueV2(bindings, (cudaStream_t)stream_, nullptr);
    if(!ok){
        std::cerr << "enqueueV2 failed\n";
        return {};
    }

    // copy output back
    size_t outSize = batch_ * outN_ * outC_;
    CHECK_CUDA(cudaMemcpyAsync(hOutput_.data(), dOutput_, outSize*sizeof(float),
                               cudaMemcpyDeviceToHost, (cudaStream_t)stream_));
    CHECK_CUDA(cudaStreamSynchronize((cudaStream_t)stream_));

    return postprocess(hOutput_.data());
}

void TrtYoloDetector::preprocess(const cv::Mat& bgr, float* gpuInput) {
    // Letterbox resize
    int w = bgr.cols, h = bgr.rows;
    float r = std::min(inW_/(float)w, inH_/(float)h);
    int newW = int(w*r);
    int newH = int(h*r);
    int padW = inW_ - newW;
    int padH = inH_ - newH;
    int padLeft = padW/2;
    int padTop  = padH/2;

    scale_ = r;
    padLeft_ = padLeft;
    padTop_  = padTop;
    origW_   = w;
    origH_   = h;

    cv::Mat resized;
    cv::resize(bgr, resized, {newW, newH});

    cv::Mat canvas(inH_, inW_, CV_8UC3, cv::Scalar(114,114,114));
    resized.copyTo(canvas(cv::Rect(padLeft, padTop, newW, newH)));

    cv::cvtColor(canvas, canvas, cv::COLOR_BGR2RGB);
    canvas.convertTo(canvas, CV_32F, 1/255.0);

    // HWC -> CHW
    std::vector<cv::Mat> chw(3);
    for(int i=0;i<3;i++){
        chw[i] = cv::Mat(inH_, inW_, CV_32F, hInput_.data() + i*inH_*inW_);
    }
    cv::split(canvas, chw);

    // copy to GPU
    CHECK_CUDA(cudaMemcpyAsync(gpuInput, hInput_.data(),
                               hInput_.size()*sizeof(float),
                               cudaMemcpyHostToDevice,
                               (cudaStream_t)stream_));
}

std::vector<Detection> TrtYoloDetector::postprocess(const float* out) {
    std::vector<cv::Rect2f> boxes;
    std::vector<float> scores;
    std::vector<int> classes;

    const float confThr = confThr_;
    const float iouThr  = iouThr_;

    // out layout: [C, N] == [5, 8400]
    // out[c*outN_ + i]

    for(int i = 0; i < outN_; ++i){
        float cx = out[0*outN_ + i];
        float cy = out[1*outN_ + i];
        float w  = out[2*outN_ + i];
        float h  = out[3*outN_ + i];
        float conf = out[4*outN_ + i];   // python中直接用，不做sigmoid

        if(conf < confThr) continue;

        // cxcywh (640像素系) -> xyxy (仍在letterbox后的输入图上)
        float x1 = cx - w * 0.5f;
        float y1 = cy - h * 0.5f;
        float x2 = cx + w * 0.5f;
        float y2 = cy + h * 0.5f;

        // clip 到 640 输入范围
        x1 = std::clamp(x1, 0.f, (float)inW_-1);
        y1 = std::clamp(y1, 0.f, (float)inH_-1);
        x2 = std::clamp(x2, 0.f, (float)inW_-1);
        y2 = std::clamp(y2, 0.f, (float)inH_-1);
        if(x2 <= x1 || y2 <= y1) continue;

        // 反 letterbox 回原图坐标（与 python 完全一致）
        x1 = (x1 - padLeft_) / scale_;
        y1 = (y1 - padTop_)  / scale_;
        x2 = (x2 - padLeft_) / scale_;
        y2 = (y2 - padTop_)  / scale_;

        x1 = std::clamp(x1, 0.f, (float)origW_-1);
        y1 = std::clamp(y1, 0.f, (float)origH_-1);
        x2 = std::clamp(x2, 0.f, (float)origW_-1);
        y2 = std::clamp(y2, 0.f, (float)origH_-1);

        boxes.emplace_back(cv::Rect2f(x1, y1, x2-x1, y2-y1));
        scores.emplace_back(conf);
        classes.emplace_back(0);  // 单类 person
    }

    if(boxes.empty()) return {};

    auto keep = nms(boxes, scores, iouThr);

    std::vector<Detection> dets;
    dets.reserve(keep.size());
    for(int k : keep){
        dets.push_back({boxes[k], scores[k], classes[k]});
    }
    return dets;
}
