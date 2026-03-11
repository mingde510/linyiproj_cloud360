#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <QCloseEvent>
#include <QMetaObject>
#include <QPixmap>
#include <QVTKWidget.h>
#include <algorithm>
#include <chrono>
#include <thread>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <QPainter>
#include <QPen>
#include <QPixmap>

Worker::Worker(SessionManager* session, Task task, QString exportName, QObject* parent)
    : QObject(parent), session_(session), task_(task), exportName_(exportName) {}

void Worker::run() {
    bool ok = false;
    QString msg;

    switch (task_) {
        case MakeBG:
            ok = session_->makeBackground([&](double e,double t){ emit progress(e,t); });
            msg = ok ? "背景建模完成" : "背景建模失败";
            break;
        case RecordFG:
            ok = session_->recordForeground([&](double e,double t){ emit progress(e,t); });
            msg = ok ? "前景录入完成" : "前景录入失败";
            break;
        case Subtract:
            ok = session_->doSubtract();
            msg = ok ? "差分完成" : "差分失败（请先制作背景 + 录入前景）";
            break;
        case ExportPCD:
            ok = session_->exportResultPCD(exportName_.toStdString());
            msg = ok ? QString("已导出 %1").arg(exportName_) : "导出失败（没有结果点云）";
            break;
    }

    emit finished(ok, msg);
}

PersonWorker::PersonWorker(std::unique_ptr<PersonDetectorApp> app, QObject* parent)
    : QObject(parent), app_(std::move(app)) {}

void PersonWorker::run() {
    if (!app_) { emit finished(); return; }
    app_->setFrameCallback([this](const cv::Mat& frame){
        cv::Mat rgb;
        if (frame.channels() == 3) {
            cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
        } else {
            rgb = frame.clone();
        }
        QImage img(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888);
        emit frameReady(img.copy()); // detach from cv::Mat memory
    });
    app_->run();
    emit finished();
}

void PersonWorker::stop() {
    if (app_) app_->stop();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->viewHorizontalLayout->setStretch(0, 1); // 左边参数区
    ui->viewHorizontalLayout->setStretch(1, 3); // 右边视频区

    connect(ui->makeBgButton, &QPushButton::clicked, this, &MainWindow::onMakeBgClicked);
    connect(ui->recordFgButton, &QPushButton::clicked, this, &MainWindow::onRecordFgClicked);
    connect(ui->subtractButton, &QPushButton::clicked, this, &MainWindow::onSubtractClicked);
    connect(ui->exportButton, &QPushButton::clicked, this, &MainWindow::onExportClicked);

    connect(ui->cbBackground, &QCheckBox::toggled, this, &MainWindow::onCloudDisplayChanged);
    connect(ui->cbForeground, &QCheckBox::toggled, this, &MainWindow::onCloudDisplayChanged);
    connect(ui->cbDiff, &QCheckBox::toggled, this, &MainWindow::onCloudDisplayChanged);
    connect(ui->cbShowPlane, &QCheckBox::toggled, this, &MainWindow::onCloudDisplayChanged);

    connect(ui->startPersonBtn, &QPushButton::clicked, this, &MainWindow::onStartPersonClicked);
    connect(ui->stopPersonBtn,  &QPushButton::clicked, this, &MainWindow::onStopPersonClicked);
    connect(ui->sourceCombo,    QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSourceChanged);

    session_ = std::make_unique<SessionManager>(collectOptionsFromUi());

    ui->personLabel->installEventFilter(this);

    // 1) 让视频label能收鼠标
    ui->personLabel->installEventFilter(this);
    ui->personLabel->setMouseTracking(true);
    ui->personLabel->setEnabled(true);
    ui->personLabel->setAttribute(Qt::WA_TransparentForMouseEvents, false);

    // 2) 接 UI 控件（你 v6 加的）
    connect(ui->btnSetAlarmRoi, &QPushButton::clicked, this, [=]{
        selectingAlarmRoi_ = true;
        alarmRoiReady_ = false;
        alarmPtsLabel_.clear();
        alarmPolyLabel_.clear();
        freezeForRoiPending_ = true;   // 等下一帧到来再冻结
        freezeForRoi_ = false;               // 冻结下一帧
        ui->alarmStatusLabel->setText("告警状态: 请选择区域（左键逐点）");
    });

    connect(ui->btnClearAlarmRoi, &QPushButton::clicked, this, [=]{
        selectingAlarmRoi_ = false;
        alarmRoiReady_ = false;
        freezeForRoi_ = false;
        alarmPtsLabel_.clear();
        alarmPolyLabel_.clear();
        ui->alarmStatusLabel->setText("告警状态: 未设定区域");
    });

    connect(ui->alarmIouSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [=](double v){ alarmIouThresh_ = (float)v; });

    initPointCloudView();
    populateSources();
    onSourceChanged(ui->sourceCombo->currentIndex());

    setUiRunning(false);
    refreshStats();
    loadConfig();
    refreshPointCloudView();
}

MainWindow::~MainWindow() {
    clearPersonThread();
    if (workerThread_) {
        workerThread_->quit();
        workerThread_->wait();
    }
    delete ui;
}

SessionManager::Options MainWindow::collectOptionsFromUi() const {
    SessionManager::Options opt;
    opt.lidar_opt.port = ui->portEdit->text().toStdString();
    opt.bg_seconds = ui->bgSecondsSpin->value();
    opt.fg_seconds = ui->fgSecondsSpin->value();

    opt.bg_opt.voxel_size = ui->voxelSpin->value();
    opt.bg_opt.diff_base = ui->diffSpin->value();
    opt.bg_opt.diff_k = 0.003;
    opt.bg_opt.nb_neighbors = 20;
    opt.bg_opt.std_ratio = 2.0;

    opt.bg_opt.stable_voxel = 0.35;
    opt.bg_opt.min_presence_ratio = 0.15;

    return opt;
}

void MainWindow::setUiRunning(bool running) {
    ui->makeBgButton->setEnabled(!running);
    ui->recordFgButton->setEnabled(!running);
    ui->subtractButton->setEnabled(!running);
    ui->exportButton->setEnabled(!running);

    ui->progressBar->setValue(0);
    ui->statusLabel->setText(running ? "运行中.." : "就绪");
}

static float iouXYXY(const QRectF& a, const QRectF& b)
{
    QRectF inter = a.intersected(b);
    if (inter.isEmpty()) return 0.f;
    float interArea = inter.width() * inter.height();
    float unionArea = a.width()*a.height() + b.width()*b.height() - interArea;
    return unionArea > 0 ? interArea / unionArea : 0.f;
}

bool MainWindow::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == ui->personLabel && selectingAlarmRoi_) {

        if (ev->type() == QEvent::MouseButtonPress) {
            auto* e = static_cast<QMouseEvent*>(ev);

            if (e->button() == Qt::LeftButton) {
                // 点击点必须落在图像显示区域内
                QPoint p = e->pos();
                if (!shownRectInLabel_.contains(p)) return true;

                alarmPtsLabel_.push_back(p);
                redrawFrozenWithRoi();
                return true;
            }

            // 右键/双击可结束闭合（可选）
            if (e->button() == Qt::RightButton) {
                if (alarmPtsLabel_.size() >= 3) {
                    selectingAlarmRoi_ = false;
                    alarmRoiReady_ = true;
                    freezeForRoi_ = false;  // 解冻恢复视频
                    ui->alarmStatusLabel->setText("告警状态: 区域已设定");
                    redrawFrozenWithRoi();  // 最后闭合一次
                }
                return true;
            }
        }

        if (ev->type() == QEvent::MouseButtonDblClick) {
            // 双击闭合
            if (alarmPtsLabel_.size() >= 3) {
                selectingAlarmRoi_ = false;
                alarmRoiReady_ = true;
                freezeForRoi_ = false;
                ui->alarmStatusLabel->setText("告警状态: 区域已设定");
                redrawFrozenWithRoi();
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, ev);
}

void MainWindow::redrawFrozenWithRoi()
{
    if (frozenFrameShown_.isNull()) return;

    QImage canvas(ui->personLabel->width(),
                  ui->personLabel->height(),
                  QImage::Format_RGB888);
    canvas.fill(Qt::black);

    QPainter painter(&canvas);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 把冻结帧画到 label 中心区域
    painter.drawImage(shownRectInLabel_.topLeft(), frozenFrameShown_);

    // 画点和线
    if (!alarmPtsLabel_.isEmpty()) {
        QPen pen(Qt::green);
        pen.setWidth(2);
        painter.setPen(pen);

        // 点
        for (int i = 0; i < alarmPtsLabel_.size(); ++i) {
            QPointF pt = alarmPtsLabel_[i];
            painter.drawEllipse(pt, 3, 3);
            if (i > 0) {
                painter.drawLine(alarmPtsLabel_[i-1], pt);
            }
        }

        // 闭合线（如果设定完成 或者你想实时预闭合也行）
        if (alarmPtsLabel_.size() >= 3) {
            painter.drawLine(alarmPtsLabel_.last(), alarmPtsLabel_.first());
            alarmPolyLabel_ = QPolygonF(alarmPtsLabel_);
        }
    }

    painter.end();

    ui->personLabel->setPixmap(QPixmap::fromImage(canvas));
}

QPolygonF MainWindow::polyLabelToImage(const QPolygonF& polyLabel,
                                      int imgW, int imgH)
{
    QPolygonF out;
    if (polyLabel.isEmpty()) return out;

    for (auto& ptL : polyLabel) {
        float x = (ptL.x() - shownRectInLabel_.x()) / shownScale_;
        float y = (ptL.y() - shownRectInLabel_.y()) / shownScale_;
        x = std::clamp(x, 0.f, (float)imgW-1);
        y = std::clamp(y, 0.f, (float)imgH-1);
        out << QPointF(x, y);
    }
    return out;
}


void MainWindow::checkAlarmWithPoly(const std::vector<QRectF>& personBoxesImg,
                                   int imgW, int imgH)
{
    if (!alarmRoiReady_ || alarmPolyLabel_.isEmpty()) return;

    QPolygonF polyImg = polyLabelToImage(alarmPolyLabel_, imgW, imgH);
    QPainterPath roiPath;
    roiPath.addPolygon(polyImg);

    bool alarmOn = false;

    for (auto& box : personBoxesImg) {
        QPainterPath boxPath;
        boxPath.addRect(box);

        QPainterPath inter = roiPath.intersected(boxPath);
        float interArea = inter.boundingRect().width() * inter.boundingRect().height();
        float boxArea = box.width() * box.height();
        float ratio = (boxArea > 0) ? interArea / boxArea : 0.f;

        if (ratio >= alarmIouThresh_) {
            alarmOn = true;
            break;
        }
    }

    if (alarmOn) {
        ui->alarmStatusLabel->setText("告警状态: ⚠ 告警中");
        // TODO: 播音/闪烁/ROS消息...
    } else {
        ui->alarmStatusLabel->setText("告警状态: 未告警");
    }
}




void MainWindow::refreshStats() {
    ui->logText->append(QString("背景帧 %1  前景帧 %2  背景点 %3  结果点 %4")
        .arg(session_->bgFrameCount())
        .arg(session_->fgFrameCount())
        .arg(session_->bgPointCount())
        .arg(session_->resultPointCount()));
    ui->subtractButton->setEnabled(session_->hasBackground() && session_->hasForeground());
    ui->exportButton->setEnabled(session_->hasResult());
}

void MainWindow::startTask(Worker::Task task, QString exportName) {
    lastTask_ = task;
    setUiRunning(true);
    session_->updateOptions(collectOptionsFromUi());

    workerThread_ = new QThread(this);
    worker_ = new Worker(session_.get(), task, exportName);
    worker_->moveToThread(workerThread_);

    connect(workerThread_, &QThread::started, worker_, &Worker::run);
    connect(worker_, &Worker::progress, this, &MainWindow::onWorkerProgress);
    connect(worker_, &Worker::finished, this, &MainWindow::onWorkerFinished);

    connect(worker_, &Worker::finished, workerThread_, &QThread::quit);
    connect(worker_, &Worker::finished, worker_, &QObject::deleteLater);
    connect(workerThread_, &QThread::finished, workerThread_, &QObject::deleteLater);

    workerThread_->start();
}

void MainWindow::onMakeBgClicked() {
    ui->logText->append("开始制作背景...");
    startTask(Worker::MakeBG);
}

void MainWindow::onRecordFgClicked() {
    ui->logText->append("开始录入前景...");
    startTask(Worker::RecordFG);
}

void MainWindow::onSubtractClicked() {
    ui->logText->append("开始背景差分...");

    RoiAnalyzer::Roi roi;
    roi.xmin = ui->xminSpin->value();
    roi.xmax = ui->xmaxSpin->value();
    roi.ymin = ui->yminSpin->value();
    roi.ymax = ui->ymaxSpin->value();
    roi.zmin = ui->zminSpin->value();
    roi.zmax = ui->zmaxSpin->value();
    session_->setRoi(roi);

    ui->subtractButton->setEnabled(false);
    ui->logText->append("开始背景差分 + ROI几何分析...");
    startTask(Worker::Subtract);
}

void MainWindow::onExportClicked() {
    QString file = QFileDialog::getSaveFileName(
        this, "导出结果点云", "result_foreground.pcd", "PCD Files (*.pcd)");
    if (file.isEmpty()) return;

    ui->logText->append("导出 PCD...");
    startTask(Worker::ExportPCD, file);
}

void MainWindow::onWorkerProgress(double elapsed, double total) {
    double ratio = (total <= 0) ? 0 : elapsed / total;
    ratio = std::min(1.0, std::max(0.0, ratio));
    ui->progressBar->setValue(int(ratio * 100));
    ui->statusLabel->setText(QString("%1/%2 s")
        .arg(elapsed,0,'f',1).arg(total,0,'f',1));
}

void MainWindow::onWorkerFinished(bool ok, QString msg) {
    setUiRunning(false);
    ui->logText->append(msg);

    if (lastTask_ == Worker::MakeBG && ok) {
        saveConfig();

        QString bgPath = QDir(configDir()).filePath("background.pcd");
        if (session_->saveBackgroundToFile(bgPath.toStdString())) {
            ui->logText->append("背景点云已保存 " + bgPath);
        } else {
            ui->logText->append("背景点云保存失败");
        }
    }

    refreshStats();
    updateButtonsState();
    ui->subtractButton->setEnabled(true);

    auto g = session_->geometryInfo();
    if (g.valid) {
        ui->lengthLabel->setText(QString("Length: %1 m").arg(g.length, 0, 'f', 3));
        ui->widthLabel->setText(QString("Width: %1 m").arg(g.width, 0, 'f', 3));
        ui->yawLabel->setText(QString("Yaw: %1 deg").arg(g.yaw_deg, 0, 'f', 2));
        ui->logText->append(QString("Plane inlier ratio: %1")
                            .arg(g.plane_inlier_ratio,0,'f',2));
    } else {
        ui->lengthLabel->setText("Length: -- m");
        ui->widthLabel->setText("Width: -- m");
        ui->yawLabel->setText("Yaw: -- deg");
        ui->logText->append("ROI内平面/OBB计算失败（ROI点太少或无平面）");
    }

    refreshPointCloudView();
}

QString MainWindow::configDir() const {
    QDir d(QCoreApplication::applicationDirPath());
    d.cdUp();  // build -> bgDiff root
    if (!d.exists("config")) d.mkpath("config");
    return d.filePath("config");
}

void MainWindow::loadConfig() {
    QString iniPath = QDir(configDir()).filePath("config.ini");
    QSettings s(iniPath, QSettings::IniFormat);
    s.setIniCodec("UTF-8");

    ui->portEdit->setText(s.value("serial/port", "/dev/ttyACM0").toString());
    ui->bgSecondsSpin->setValue(s.value("capture/bg_seconds", 5.0).toDouble());
    ui->fgSecondsSpin->setValue(s.value("capture/fg_seconds", 5.0).toDouble());
    ui->voxelSpin->setValue(s.value("filter/voxel", 0.05).toDouble());
    ui->diffSpin->setValue(s.value("subtract/base_thresh", 0.15).toDouble());

    ui->xminSpin->setValue(s.value("roi/xmin", -2.0).toDouble());
    ui->xmaxSpin->setValue(s.value("roi/xmax",  2.0).toDouble());
    ui->yminSpin->setValue(s.value("roi/ymin", -2.0).toDouble());
    ui->ymaxSpin->setValue(s.value("roi/ymax",  2.0).toDouble());
    ui->zminSpin->setValue(s.value("roi/zmin", -2.0).toDouble());
    ui->zmaxSpin->setValue(s.value("roi/zmax",  2.0).toDouble());

    ui->engineEdit->setText(s.value("person/engine", "config/best_person_simp_fp16.engine").toString());
    ui->confSpin->setValue(s.value("person/conf", 0.5).toDouble());
    ui->rtspEdit->setText(s.value("person/rtsp", "rtsp://").toString());

    ui->logText->append("配置已读取 " + iniPath);

    QString bgPath = QDir(configDir()).filePath("background.pcd");
    if (QFile::exists(bgPath) &&
        session_->loadBackgroundFromFile(bgPath.toStdString())) {
        ui->logText->append("背景点云已恢复 " + bgPath);
    } else {
        ui->logText->append("未找到背景点云或加载失败");
    }

    updateButtonsState();
}


void MainWindow::saveConfig() const {
    QString iniPath = QDir(configDir()).filePath("config.ini");
    QSettings s(iniPath, QSettings::IniFormat);
    s.setIniCodec("UTF-8");

    s.setValue("serial/port", ui->portEdit->text());
    s.setValue("capture/bg_seconds", ui->bgSecondsSpin->value());
    s.setValue("capture/fg_seconds", ui->fgSecondsSpin->value());
    s.setValue("filter/voxel", ui->voxelSpin->value());
    s.setValue("subtract/base_thresh", ui->diffSpin->value());

    s.setValue("roi/xmin", ui->xminSpin->value());
    s.setValue("roi/xmax", ui->xmaxSpin->value());
    s.setValue("roi/ymin", ui->yminSpin->value());
    s.setValue("roi/ymax", ui->ymaxSpin->value());
    s.setValue("roi/zmin", ui->zminSpin->value());
    s.setValue("roi/zmax", ui->zmaxSpin->value());

    s.setValue("person/engine", ui->engineEdit->text());
    s.setValue("person/conf", ui->confSpin->value());
    s.setValue("person/rtsp", ui->rtspEdit->text());

    s.sync();
}

void MainWindow::updateButtonsState() {
    bool hasBg = session_->hasBackground();
    bool hasFg = session_->hasForeground();
    bool hasResult = session_->hasResult();

    ui->recordFgButton->setEnabled(hasBg);
    ui->subtractButton->setEnabled(hasBg && hasFg);
    ui->exportButton->setEnabled(hasResult);
}

void MainWindow::closeEvent(QCloseEvent *e) {
    saveConfig();
    clearPersonThread();
    ui->logText->append("配置已保存");
    QMainWindow::closeEvent(e);
}

void MainWindow::initPointCloudView() {
    if (visReady_) return;
    vis_ = boost::make_shared<pcl::visualization::PCLVisualizer>("viewer", false);
    ui->vtkWidget->SetRenderWindow(vis_->getRenderWindow());
    ui->vtkWidget->GetRenderWindow()->GetInteractor()->Initialize();
    vis_->setupInteractor(ui->vtkWidget->GetInteractor(), ui->vtkWidget->GetRenderWindow());
    vis_->setBackgroundColor(0,0,0);
    vis_->addCoordinateSystem(0.5);
    vis_->initCameraParameters();
    visReady_ = true;
}

void MainWindow::refreshPointCloudView() {
    if (!visReady_) initPointCloudView();
    if (!vis_) return;

    auto bg = session_->backgroundCloud();
    auto fg = session_->foregroundCloud();
    auto diff = session_->resultFull();

    auto updateCloud = [&](const std::string& id,
                           pcl::PointCloud<PointType>::ConstPtr cloud,
                           bool enable,
                           int r, int g, int b)
    {
        if (enable && cloud && !cloud->empty()) {
            pcl::visualization::PointCloudColorHandlerCustom<PointType> color(cloud, r, g, b);
            if (vis_->contains(id)) {
                vis_->updatePointCloud<PointType>(cloud, color, id);
            } else {
                vis_->addPointCloud<PointType>(cloud, color, id);
                vis_->setPointCloudRenderingProperties(
                    pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, id);
            }
        } else if (vis_->contains(id)) {
            vis_->removePointCloud(id);
        }
    };

    updateCloud("bg_cloud", bg, ui->cbBackground->isChecked(), 200, 200, 200);
    updateCloud("fg_cloud", fg, ui->cbForeground->isChecked(), 0, 255, 0);
    updateCloud("diff_cloud", diff, ui->cbDiff->isChecked(), 255, 0, 0);

    if (ui->cbShowPlane->isChecked()) {
        auto g = session_->geometryInfo();
        auto coeffVec = session_->lastPlaneCoeff();
        if (g.valid) {
            auto hull = session_->planeHullWorld();
            if (hull && hull->size() >= 3) {
                if (vis_->contains("plane")) vis_->removeShape("plane");
                if (vis_->contains("plane_hull")) vis_->removeShape("plane_hull");
                vis_->addPolygon<pcl::PointXYZ>(hull, 0.8, 0.8, 0.8, "plane_hull");
                vis_->setShapeRenderingProperties(
                    pcl::visualization::PCL_VISUALIZER_OPACITY, 0.4, "plane_hull");
            } else {
                pcl::ModelCoefficients coeff;
                coeff.values = {coeffVec[0], coeffVec[1], coeffVec[2], coeffVec[3]};
                if (vis_->contains("plane_hull")) vis_->removeShape("plane_hull");
                if (vis_->contains("plane")) vis_->removeShape("plane");
                vis_->addPlane(coeff, "plane");
            }
        } else {
            if (vis_->contains("plane_hull")) vis_->removeShape("plane_hull");
            if (vis_->contains("plane")) vis_->removeShape("plane");
        }
    } else {
        if (vis_->contains("plane_hull")) vis_->removeShape("plane_hull");
        if (vis_->contains("plane")) vis_->removeShape("plane");
    }

    if (ui->vtkWidget->GetRenderWindow()) {
        ui->vtkWidget->GetRenderWindow()->Render();
    }
    ui->vtkWidget->update();
}

void MainWindow::onCloudDisplayChanged() {
    refreshPointCloudView();
}

void MainWindow::populateSources() {
    ui->sourceCombo->clear();
    bool anyCam = false;
    for (int i = 0; i < 4; ++i) {
        cv::VideoCapture cap(i);
        if (cap.isOpened()) {
            ui->sourceCombo->addItem(QString("Camera %1").arg(i), i);
            anyCam = true;
        }
    }
    if (!anyCam) {
        ui->sourceCombo->addItem("Camera 0", 0);
    }
    ui->sourceCombo->addItem("RTSP", -1);
}

void MainWindow::onSourceChanged(int idx) {
    Q_UNUSED(idx);
    bool isRtsp = (ui->sourceCombo->currentText() == "RTSP");
    ui->rtspLabel->setVisible(isRtsp);
    ui->rtspEdit->setVisible(isRtsp);
}

void MainWindow::onStartPersonClicked() {
    clearPersonThread();

    PersonDetectorApp::Options opt;
    QString srcText = ui->sourceCombo->currentText();
    if (srcText == "RTSP") {
        opt.rtsp_url = ui->rtspEdit->text().toStdString();
    } else {
        opt.camera_id = ui->sourceCombo->currentData().toInt();
    }
    opt.engine_path = ui->engineEdit->text().toStdString();
    opt.conf_thres = static_cast<float>(ui->confSpin->value());

    auto app = std::make_unique<PersonDetectorApp>(opt);
    personWorker_ = new PersonWorker(std::move(app));
    personThread_ = new QThread(this);
    personWorker_->moveToThread(personThread_);

    connect(personThread_, &QThread::started, personWorker_, &PersonWorker::run);
    connect(personWorker_, &PersonWorker::frameReady, this, &MainWindow::onPersonFrameReady);
    connect(personWorker_, &PersonWorker::finished, this, &MainWindow::onPersonFinished);
    connect(personWorker_, &PersonWorker::finished, personThread_, &QThread::quit);
    connect(personThread_, &QThread::finished, personWorker_, &QObject::deleteLater);
    connect(personThread_, &QThread::finished, personThread_, &QObject::deleteLater);

    personThread_->start();
    ui->logText->append("行人检测已启动");
}

void MainWindow::onStopPersonClicked() {
    clearPersonThread();
    ui->logText->append("行人检测已停止");
}

void MainWindow::onPersonFrameReady(const QImage& img) {
    ui->personLabel->setPixmap(QPixmap::fromImage(img));
}

void MainWindow::onPersonFinished() {
    personThread_ = nullptr;
    personWorker_ = nullptr;
}

void MainWindow::clearPersonThread() {
    if (personWorker_) {
        QMetaObject::invokeMethod(personWorker_, "stop", Qt::QueuedConnection);
    }
    if (personThread_) {
        personThread_->quit();
        personThread_->wait();
        personThread_ = nullptr;
        personWorker_ = nullptr;
    }
}
