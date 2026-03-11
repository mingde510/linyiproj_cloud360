#pragma once
#include <QMainWindow>
#include <QThread>
#include <QImage>
#include <memory>
#include <pcl/visualization/pcl_visualizer.h>
#include <opencv2/opencv.hpp>
#include "../core/SessionManager.h"
#include "../core/VisualizerHelper.h"
#include "../person_tr/PersonDetectorApp.h"
#include <QVector>
#include <QPointF>
#include <QPolygonF>
#include <QRubberBand>
#include <QMouseEvent>
#include <QPainterPath>
#include <QRectF>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 通用 Lidar 任务 Worker
class Worker : public QObject {
    Q_OBJECT
public:
    enum Task { MakeBG, RecordFG, Subtract, ExportPCD };

    explicit Worker(SessionManager* session, Task task, QString exportName="", QObject* parent=nullptr);

signals:
    void progress(double elapsed, double total);
    void finished(bool ok, QString msg);

public slots:
    void run();

private:
    SessionManager* session_;
    Task task_;
    QString exportName_;
};

// 行人检测 Worker
class PersonWorker : public QObject {
    Q_OBJECT
public:
    explicit PersonWorker(std::unique_ptr<PersonDetectorApp> app, QObject* parent=nullptr);

public slots:
    void run();
    void stop();

signals:
    void frameReady(const QImage& img);
    void finished();

private:
    std::unique_ptr<PersonDetectorApp> app_;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent=nullptr);
    ~MainWindow();

private slots:
    void onMakeBgClicked();
    void onRecordFgClicked();
    void onSubtractClicked();
    void onExportClicked();

    void onWorkerProgress(double elapsed, double total);
    void onWorkerFinished(bool ok, QString msg);

    void onCloudDisplayChanged();

    void onStartPersonClicked();
    void onStopPersonClicked();
    void onPersonFrameReady(const QImage& img);
    void onPersonFinished();
    void onSourceChanged(int idx);

protected:
    void closeEvent(QCloseEvent *e) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::MainWindow *ui;

    std::unique_ptr<SessionManager> session_;

    QThread* workerThread_ = nullptr;
    Worker* worker_ = nullptr;
    Worker::Task lastTask_{Worker::MakeBG};

    QThread* personThread_ = nullptr;
    PersonWorker* personWorker_ = nullptr;

    pcl::visualization::PCLVisualizer::Ptr vis_;
    bool visReady_{false};

    void startTask(Worker::Task task, QString exportName="");
    void setUiRunning(bool running);
    void refreshStats();
    SessionManager::Options collectOptionsFromUi() const;

    void loadConfig();
    void saveConfig() const;
    void updateButtonsState();
    QString configDir() const;

    void initPointCloudView();
    void refreshPointCloudView();
    void clearPersonThread();
    void populateSources();

    // ===== 告警ROI交互 =====
    bool selectingAlarmRoi_ = false;   // 是否处于“点选ROI”模式
    bool alarmRoiReady_ = false;       // 是否已闭合完成
    bool freezeForRoi_ = false;        // 是否冻结视频
    QVector<QPointF> alarmPtsLabel_;   // ROI点（label坐标系）
    QPolygonF alarmPolyLabel_;         // ROI多边形（label坐标系）

    QImage frozenFrame_;              // 冻结帧（原图尺寸）
    QImage frozenFrameShown_;         // 冻结帧缩放后（label尺寸）
    QRect  shownRectInLabel_;         // 实际图像在label内的显示区域（含padding）
    float  shownScale_ = 1.f;          // 原图 -> label的缩放比例
    bool freezeForRoiPending_ = false; 

    float alarmIouThresh_ = 0.5f;      // UI阈值（alarmIouSpin）

    // 画ROI到当前显示帧（label坐标）
    void redrawFrozenWithRoi();

    // label坐标ROI -> 原图坐标ROI
    QPolygonF polyLabelToImage(const QPolygonF& polyLabel, int imgW, int imgH);

    // 告警判定：框与ROI的重叠占比(近似IoU)
    void checkAlarmWithPoly(const std::vector<QRectF>& personBoxesImg,
                            int imgW, int imgH);
};
