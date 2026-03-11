cmake_minimum_required(VERSION 3.10)
project(person_roi_trt)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

# 修改为你本机 TensorRT / CUDA 路径
set(TENSORRT_ROOT /usr/lib/x86_64-linux-gnu)
set(CUDA_ROOT /usr/local/cuda)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${CUDA_ROOT}/include
    /usr/include/x86_64-linux-gnu
)

link_directories(
    ${TENSORRT_ROOT}
    ${CUDA_ROOT}/lib64
)

add_executable(person_roi_trt
    main.cpp
    RtspCapture.cpp
    RoiManager.cpp
    TrtYoloDetector.cpp
    Visualizer.cpp
)

target_link_libraries(person_roi_trt
    ${OpenCV_LIBS}
    nvinfer
    nvinfer_plugin
    nvonnxparser
    cudart
    Threads::Threads
)

# 如果你需要 cublas/cudnn 可再加
# target_link_libraries(person_roi_trt cublas cudnn)
