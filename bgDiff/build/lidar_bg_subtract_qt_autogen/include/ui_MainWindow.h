/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "QVTKWidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *centralVerticalLayout;
    QTabWidget *tabWidget;
    QWidget *tab_param;
    QVBoxLayout *paramVerticalLayout;
    QHBoxLayout *paramMainHBox;
    QVBoxLayout *leftLayout;
    QHBoxLayout *horizontalLayoutResult;
    QLabel *lengthLabel;
    QLabel *widthLabel;
    QLabel *yawLabel;
    QSpacerItem *spacerResult;
    QGroupBox *groupBoxParams;
    QGridLayout *gridLayoutParams;
    QLabel *labelPort;
    QLineEdit *portEdit;
    QLabel *labelBgSec;
    QDoubleSpinBox *bgSecondsSpin;
    QLabel *labelFgSec;
    QDoubleSpinBox *fgSecondsSpin;
    QLabel *labelVoxel;
    QDoubleSpinBox *voxelSpin;
    QLabel *labelDiff;
    QDoubleSpinBox *diffSpin;
    QGroupBox *groupBoxROI;
    QGridLayout *gridLayoutROI;
    QLabel *labelXmin;
    QDoubleSpinBox *xminSpin;
    QLabel *labelXmax;
    QDoubleSpinBox *xmaxSpin;
    QLabel *labelYmin;
    QDoubleSpinBox *yminSpin;
    QLabel *labelYmax;
    QDoubleSpinBox *ymaxSpin;
    QLabel *labelZmin;
    QDoubleSpinBox *zminSpin;
    QLabel *labelZmax;
    QDoubleSpinBox *zmaxSpin;
    QGroupBox *groupBoxCloudDisplay;
    QVBoxLayout *verticalLayoutCloudDisplay;
    QCheckBox *cbBackground;
    QCheckBox *cbForeground;
    QCheckBox *cbDiff;
    QCheckBox *cbShowPlane;
    QHBoxLayout *horizontalLayoutButtons;
    QPushButton *makeBgButton;
    QPushButton *recordFgButton;
    QPushButton *subtractButton;
    QPushButton *exportButton;
    QHBoxLayout *horizontalLayoutStatus;
    QProgressBar *progressBar;
    QLabel *statusLabel;
    QTextEdit *logText;
    QSpacerItem *leftSpacer;
    QGroupBox *groupBoxCloudView;
    QVBoxLayout *layoutCloudView;
    QVTKWidget *vtkWidget;
    QWidget *tab_view;
    QHBoxLayout *viewHorizontalLayout;
    QGroupBox *groupBoxPerson;
    QGridLayout *gridLayoutPerson;
    QLabel *labelSource;
    QComboBox *sourceCombo;
    QLabel *rtspLabel;
    QLineEdit *rtspEdit;
    QLabel *engineLabel;
    QLineEdit *engineEdit;
    QLabel *confLabel;
    QDoubleSpinBox *confSpin;
    QHBoxLayout *layoutPersonButtons;
    QPushButton *startPersonBtn;
    QPushButton *stopPersonBtn;
    QLabel *labelAlarmIou;
    QDoubleSpinBox *alarmIouSpin;
    QWidget *alarmBtnWidget;
    QHBoxLayout *alarmBtnLayout;
    QPushButton *btnSetAlarmRoi;
    QPushButton *btnClearAlarmRoi;
    QLabel *alarmStatusLabel;
    QGroupBox *groupBoxPersonView;
    QVBoxLayout *layoutPersonView;
    QLabel *personLabel;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1200, 800);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        centralVerticalLayout = new QVBoxLayout(centralwidget);
        centralVerticalLayout->setSpacing(8);
        centralVerticalLayout->setContentsMargins(8, 8, 8, 8);
        centralVerticalLayout->setObjectName(QString::fromUtf8("centralVerticalLayout"));
        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tab_param = new QWidget();
        tab_param->setObjectName(QString::fromUtf8("tab_param"));
        paramVerticalLayout = new QVBoxLayout(tab_param);
        paramVerticalLayout->setObjectName(QString::fromUtf8("paramVerticalLayout"));
        paramMainHBox = new QHBoxLayout();
        paramMainHBox->setObjectName(QString::fromUtf8("paramMainHBox"));
        leftLayout = new QVBoxLayout();
        leftLayout->setObjectName(QString::fromUtf8("leftLayout"));
        horizontalLayoutResult = new QHBoxLayout();
        horizontalLayoutResult->setObjectName(QString::fromUtf8("horizontalLayoutResult"));
        lengthLabel = new QLabel(tab_param);
        lengthLabel->setObjectName(QString::fromUtf8("lengthLabel"));
        lengthLabel->setMinimumWidth(160);

        horizontalLayoutResult->addWidget(lengthLabel);

        widthLabel = new QLabel(tab_param);
        widthLabel->setObjectName(QString::fromUtf8("widthLabel"));
        widthLabel->setMinimumWidth(160);

        horizontalLayoutResult->addWidget(widthLabel);

        yawLabel = new QLabel(tab_param);
        yawLabel->setObjectName(QString::fromUtf8("yawLabel"));
        yawLabel->setMinimumWidth(160);

        horizontalLayoutResult->addWidget(yawLabel);

        spacerResult = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayoutResult->addItem(spacerResult);


        leftLayout->addLayout(horizontalLayoutResult);

        groupBoxParams = new QGroupBox(tab_param);
        groupBoxParams->setObjectName(QString::fromUtf8("groupBoxParams"));
        gridLayoutParams = new QGridLayout(groupBoxParams);
        gridLayoutParams->setObjectName(QString::fromUtf8("gridLayoutParams"));
        labelPort = new QLabel(groupBoxParams);
        labelPort->setObjectName(QString::fromUtf8("labelPort"));

        gridLayoutParams->addWidget(labelPort, 0, 0, 1, 1);

        portEdit = new QLineEdit(groupBoxParams);
        portEdit->setObjectName(QString::fromUtf8("portEdit"));

        gridLayoutParams->addWidget(portEdit, 0, 1, 1, 1);

        labelBgSec = new QLabel(groupBoxParams);
        labelBgSec->setObjectName(QString::fromUtf8("labelBgSec"));

        gridLayoutParams->addWidget(labelBgSec, 1, 0, 1, 1);

        bgSecondsSpin = new QDoubleSpinBox(groupBoxParams);
        bgSecondsSpin->setObjectName(QString::fromUtf8("bgSecondsSpin"));
        bgSecondsSpin->setDecimals(1);
        bgSecondsSpin->setMinimum(0.500000000000000);
        bgSecondsSpin->setMaximum(60.000000000000000);
        bgSecondsSpin->setSingleStep(0.500000000000000);
        bgSecondsSpin->setValue(5.000000000000000);

        gridLayoutParams->addWidget(bgSecondsSpin, 1, 1, 1, 1);

        labelFgSec = new QLabel(groupBoxParams);
        labelFgSec->setObjectName(QString::fromUtf8("labelFgSec"));

        gridLayoutParams->addWidget(labelFgSec, 2, 0, 1, 1);

        fgSecondsSpin = new QDoubleSpinBox(groupBoxParams);
        fgSecondsSpin->setObjectName(QString::fromUtf8("fgSecondsSpin"));
        fgSecondsSpin->setDecimals(1);
        fgSecondsSpin->setMinimum(0.500000000000000);
        fgSecondsSpin->setMaximum(60.000000000000000);
        fgSecondsSpin->setSingleStep(0.500000000000000);
        fgSecondsSpin->setValue(5.000000000000000);

        gridLayoutParams->addWidget(fgSecondsSpin, 2, 1, 1, 1);

        labelVoxel = new QLabel(groupBoxParams);
        labelVoxel->setObjectName(QString::fromUtf8("labelVoxel"));

        gridLayoutParams->addWidget(labelVoxel, 3, 0, 1, 1);

        voxelSpin = new QDoubleSpinBox(groupBoxParams);
        voxelSpin->setObjectName(QString::fromUtf8("voxelSpin"));
        voxelSpin->setDecimals(3);
        voxelSpin->setMinimum(0.001000000000000);
        voxelSpin->setMaximum(1.000000000000000);
        voxelSpin->setSingleStep(0.010000000000000);
        voxelSpin->setValue(0.050000000000000);

        gridLayoutParams->addWidget(voxelSpin, 3, 1, 1, 1);

        labelDiff = new QLabel(groupBoxParams);
        labelDiff->setObjectName(QString::fromUtf8("labelDiff"));

        gridLayoutParams->addWidget(labelDiff, 4, 0, 1, 1);

        diffSpin = new QDoubleSpinBox(groupBoxParams);
        diffSpin->setObjectName(QString::fromUtf8("diffSpin"));
        diffSpin->setDecimals(3);
        diffSpin->setMinimum(0.010000000000000);
        diffSpin->setMaximum(2.000000000000000);
        diffSpin->setSingleStep(0.010000000000000);
        diffSpin->setValue(0.150000000000000);

        gridLayoutParams->addWidget(diffSpin, 4, 1, 1, 1);


        leftLayout->addWidget(groupBoxParams);

        groupBoxROI = new QGroupBox(tab_param);
        groupBoxROI->setObjectName(QString::fromUtf8("groupBoxROI"));
        gridLayoutROI = new QGridLayout(groupBoxROI);
        gridLayoutROI->setObjectName(QString::fromUtf8("gridLayoutROI"));
        labelXmin = new QLabel(groupBoxROI);
        labelXmin->setObjectName(QString::fromUtf8("labelXmin"));

        gridLayoutROI->addWidget(labelXmin, 0, 0, 1, 1);

        xminSpin = new QDoubleSpinBox(groupBoxROI);
        xminSpin->setObjectName(QString::fromUtf8("xminSpin"));
        xminSpin->setDecimals(3);
        xminSpin->setMinimum(-100.000000000000000);
        xminSpin->setMaximum(100.000000000000000);
        xminSpin->setSingleStep(0.050000000000000);
        xminSpin->setValue(-2.000000000000000);

        gridLayoutROI->addWidget(xminSpin, 0, 1, 1, 1);

        labelXmax = new QLabel(groupBoxROI);
        labelXmax->setObjectName(QString::fromUtf8("labelXmax"));

        gridLayoutROI->addWidget(labelXmax, 0, 2, 1, 1);

        xmaxSpin = new QDoubleSpinBox(groupBoxROI);
        xmaxSpin->setObjectName(QString::fromUtf8("xmaxSpin"));
        xmaxSpin->setDecimals(3);
        xmaxSpin->setMinimum(-100.000000000000000);
        xmaxSpin->setMaximum(100.000000000000000);
        xmaxSpin->setSingleStep(0.050000000000000);
        xmaxSpin->setValue(2.000000000000000);

        gridLayoutROI->addWidget(xmaxSpin, 0, 3, 1, 1);

        labelYmin = new QLabel(groupBoxROI);
        labelYmin->setObjectName(QString::fromUtf8("labelYmin"));

        gridLayoutROI->addWidget(labelYmin, 1, 0, 1, 1);

        yminSpin = new QDoubleSpinBox(groupBoxROI);
        yminSpin->setObjectName(QString::fromUtf8("yminSpin"));
        yminSpin->setDecimals(3);
        yminSpin->setMinimum(-100.000000000000000);
        yminSpin->setMaximum(100.000000000000000);
        yminSpin->setSingleStep(0.050000000000000);
        yminSpin->setValue(-2.000000000000000);

        gridLayoutROI->addWidget(yminSpin, 1, 1, 1, 1);

        labelYmax = new QLabel(groupBoxROI);
        labelYmax->setObjectName(QString::fromUtf8("labelYmax"));

        gridLayoutROI->addWidget(labelYmax, 1, 2, 1, 1);

        ymaxSpin = new QDoubleSpinBox(groupBoxROI);
        ymaxSpin->setObjectName(QString::fromUtf8("ymaxSpin"));
        ymaxSpin->setDecimals(3);
        ymaxSpin->setMinimum(-100.000000000000000);
        ymaxSpin->setMaximum(100.000000000000000);
        ymaxSpin->setSingleStep(0.050000000000000);
        ymaxSpin->setValue(2.000000000000000);

        gridLayoutROI->addWidget(ymaxSpin, 1, 3, 1, 1);

        labelZmin = new QLabel(groupBoxROI);
        labelZmin->setObjectName(QString::fromUtf8("labelZmin"));

        gridLayoutROI->addWidget(labelZmin, 2, 0, 1, 1);

        zminSpin = new QDoubleSpinBox(groupBoxROI);
        zminSpin->setObjectName(QString::fromUtf8("zminSpin"));
        zminSpin->setDecimals(3);
        zminSpin->setMinimum(-100.000000000000000);
        zminSpin->setMaximum(100.000000000000000);
        zminSpin->setSingleStep(0.050000000000000);
        zminSpin->setValue(-1.000000000000000);

        gridLayoutROI->addWidget(zminSpin, 2, 1, 1, 1);

        labelZmax = new QLabel(groupBoxROI);
        labelZmax->setObjectName(QString::fromUtf8("labelZmax"));

        gridLayoutROI->addWidget(labelZmax, 2, 2, 1, 1);

        zmaxSpin = new QDoubleSpinBox(groupBoxROI);
        zmaxSpin->setObjectName(QString::fromUtf8("zmaxSpin"));
        zmaxSpin->setDecimals(3);
        zmaxSpin->setMinimum(-100.000000000000000);
        zmaxSpin->setMaximum(100.000000000000000);
        zmaxSpin->setSingleStep(0.050000000000000);
        zmaxSpin->setValue(1.000000000000000);

        gridLayoutROI->addWidget(zmaxSpin, 2, 3, 1, 1);


        leftLayout->addWidget(groupBoxROI);

        groupBoxCloudDisplay = new QGroupBox(tab_param);
        groupBoxCloudDisplay->setObjectName(QString::fromUtf8("groupBoxCloudDisplay"));
        verticalLayoutCloudDisplay = new QVBoxLayout(groupBoxCloudDisplay);
        verticalLayoutCloudDisplay->setObjectName(QString::fromUtf8("verticalLayoutCloudDisplay"));
        cbBackground = new QCheckBox(groupBoxCloudDisplay);
        cbBackground->setObjectName(QString::fromUtf8("cbBackground"));
        cbBackground->setChecked(true);

        verticalLayoutCloudDisplay->addWidget(cbBackground);

        cbForeground = new QCheckBox(groupBoxCloudDisplay);
        cbForeground->setObjectName(QString::fromUtf8("cbForeground"));

        verticalLayoutCloudDisplay->addWidget(cbForeground);

        cbDiff = new QCheckBox(groupBoxCloudDisplay);
        cbDiff->setObjectName(QString::fromUtf8("cbDiff"));
        cbDiff->setChecked(true);

        verticalLayoutCloudDisplay->addWidget(cbDiff);

        cbShowPlane = new QCheckBox(groupBoxCloudDisplay);
        cbShowPlane->setObjectName(QString::fromUtf8("cbShowPlane"));

        verticalLayoutCloudDisplay->addWidget(cbShowPlane);


        leftLayout->addWidget(groupBoxCloudDisplay);

        horizontalLayoutButtons = new QHBoxLayout();
        horizontalLayoutButtons->setObjectName(QString::fromUtf8("horizontalLayoutButtons"));
        makeBgButton = new QPushButton(tab_param);
        makeBgButton->setObjectName(QString::fromUtf8("makeBgButton"));

        horizontalLayoutButtons->addWidget(makeBgButton);

        recordFgButton = new QPushButton(tab_param);
        recordFgButton->setObjectName(QString::fromUtf8("recordFgButton"));

        horizontalLayoutButtons->addWidget(recordFgButton);

        subtractButton = new QPushButton(tab_param);
        subtractButton->setObjectName(QString::fromUtf8("subtractButton"));

        horizontalLayoutButtons->addWidget(subtractButton);

        exportButton = new QPushButton(tab_param);
        exportButton->setObjectName(QString::fromUtf8("exportButton"));

        horizontalLayoutButtons->addWidget(exportButton);


        leftLayout->addLayout(horizontalLayoutButtons);

        horizontalLayoutStatus = new QHBoxLayout();
        horizontalLayoutStatus->setObjectName(QString::fromUtf8("horizontalLayoutStatus"));
        progressBar = new QProgressBar(tab_param);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setValue(0);
        progressBar->setTextVisible(true);

        horizontalLayoutStatus->addWidget(progressBar);

        statusLabel = new QLabel(tab_param);
        statusLabel->setObjectName(QString::fromUtf8("statusLabel"));
        statusLabel->setMinimumWidth(120);

        horizontalLayoutStatus->addWidget(statusLabel);


        leftLayout->addLayout(horizontalLayoutStatus);

        logText = new QTextEdit(tab_param);
        logText->setObjectName(QString::fromUtf8("logText"));
        logText->setReadOnly(true);
        logText->setMinimumHeight(120);

        leftLayout->addWidget(logText);

        leftSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

        leftLayout->addItem(leftSpacer);


        paramMainHBox->addLayout(leftLayout);

        groupBoxCloudView = new QGroupBox(tab_param);
        groupBoxCloudView->setObjectName(QString::fromUtf8("groupBoxCloudView"));
        layoutCloudView = new QVBoxLayout(groupBoxCloudView);
        layoutCloudView->setObjectName(QString::fromUtf8("layoutCloudView"));
        vtkWidget = new QVTKWidget(groupBoxCloudView);
        vtkWidget->setObjectName(QString::fromUtf8("vtkWidget"));
        vtkWidget->setMinimumHeight(360);

        layoutCloudView->addWidget(vtkWidget);


        paramMainHBox->addWidget(groupBoxCloudView);


        paramVerticalLayout->addLayout(paramMainHBox);

        tabWidget->addTab(tab_param, QString());
        tab_view = new QWidget();
        tab_view->setObjectName(QString::fromUtf8("tab_view"));
        viewHorizontalLayout = new QHBoxLayout(tab_view);
        viewHorizontalLayout->setObjectName(QString::fromUtf8("viewHorizontalLayout"));
        groupBoxPerson = new QGroupBox(tab_view);
        groupBoxPerson->setObjectName(QString::fromUtf8("groupBoxPerson"));
        gridLayoutPerson = new QGridLayout(groupBoxPerson);
        gridLayoutPerson->setObjectName(QString::fromUtf8("gridLayoutPerson"));
        labelSource = new QLabel(groupBoxPerson);
        labelSource->setObjectName(QString::fromUtf8("labelSource"));

        gridLayoutPerson->addWidget(labelSource, 0, 0, 1, 1);

        sourceCombo = new QComboBox(groupBoxPerson);
        sourceCombo->setObjectName(QString::fromUtf8("sourceCombo"));

        gridLayoutPerson->addWidget(sourceCombo, 0, 1, 1, 1);

        rtspLabel = new QLabel(groupBoxPerson);
        rtspLabel->setObjectName(QString::fromUtf8("rtspLabel"));

        gridLayoutPerson->addWidget(rtspLabel, 1, 0, 1, 1);

        rtspEdit = new QLineEdit(groupBoxPerson);
        rtspEdit->setObjectName(QString::fromUtf8("rtspEdit"));

        gridLayoutPerson->addWidget(rtspEdit, 1, 1, 1, 1);

        engineLabel = new QLabel(groupBoxPerson);
        engineLabel->setObjectName(QString::fromUtf8("engineLabel"));

        gridLayoutPerson->addWidget(engineLabel, 2, 0, 1, 1);

        engineEdit = new QLineEdit(groupBoxPerson);
        engineEdit->setObjectName(QString::fromUtf8("engineEdit"));

        gridLayoutPerson->addWidget(engineEdit, 2, 1, 1, 1);

        confLabel = new QLabel(groupBoxPerson);
        confLabel->setObjectName(QString::fromUtf8("confLabel"));

        gridLayoutPerson->addWidget(confLabel, 3, 0, 1, 1);

        confSpin = new QDoubleSpinBox(groupBoxPerson);
        confSpin->setObjectName(QString::fromUtf8("confSpin"));
        confSpin->setDecimals(2);
        confSpin->setMinimum(0.100000000000000);
        confSpin->setMaximum(0.990000000000000);
        confSpin->setSingleStep(0.050000000000000);
        confSpin->setValue(0.500000000000000);

        gridLayoutPerson->addWidget(confSpin, 3, 1, 1, 1);

        layoutPersonButtons = new QHBoxLayout();
        layoutPersonButtons->setObjectName(QString::fromUtf8("layoutPersonButtons"));
        startPersonBtn = new QPushButton(groupBoxPerson);
        startPersonBtn->setObjectName(QString::fromUtf8("startPersonBtn"));

        layoutPersonButtons->addWidget(startPersonBtn);

        stopPersonBtn = new QPushButton(groupBoxPerson);
        stopPersonBtn->setObjectName(QString::fromUtf8("stopPersonBtn"));

        layoutPersonButtons->addWidget(stopPersonBtn);


        gridLayoutPerson->addLayout(layoutPersonButtons, 4, 0, 1, 2);

        labelAlarmIou = new QLabel(groupBoxPerson);
        labelAlarmIou->setObjectName(QString::fromUtf8("labelAlarmIou"));

        gridLayoutPerson->addWidget(labelAlarmIou, 5, 0, 1, 1);

        alarmIouSpin = new QDoubleSpinBox(groupBoxPerson);
        alarmIouSpin->setObjectName(QString::fromUtf8("alarmIouSpin"));
        alarmIouSpin->setMinimum(0.000000000000000);
        alarmIouSpin->setMaximum(1.000000000000000);
        alarmIouSpin->setSingleStep(0.050000000000000);
        alarmIouSpin->setValue(0.500000000000000);

        gridLayoutPerson->addWidget(alarmIouSpin, 5, 1, 1, 2);

        alarmBtnWidget = new QWidget(groupBoxPerson);
        alarmBtnWidget->setObjectName(QString::fromUtf8("alarmBtnWidget"));
        alarmBtnLayout = new QHBoxLayout(alarmBtnWidget);
        alarmBtnLayout->setObjectName(QString::fromUtf8("alarmBtnLayout"));
        alarmBtnLayout->setContentsMargins(0, 0, 0, 0);
        btnSetAlarmRoi = new QPushButton(alarmBtnWidget);
        btnSetAlarmRoi->setObjectName(QString::fromUtf8("btnSetAlarmRoi"));

        alarmBtnLayout->addWidget(btnSetAlarmRoi);

        btnClearAlarmRoi = new QPushButton(alarmBtnWidget);
        btnClearAlarmRoi->setObjectName(QString::fromUtf8("btnClearAlarmRoi"));

        alarmBtnLayout->addWidget(btnClearAlarmRoi);


        gridLayoutPerson->addWidget(alarmBtnWidget, 6, 0, 1, 3);

        alarmStatusLabel = new QLabel(groupBoxPerson);
        alarmStatusLabel->setObjectName(QString::fromUtf8("alarmStatusLabel"));

        gridLayoutPerson->addWidget(alarmStatusLabel, 7, 0, 1, 3);


        viewHorizontalLayout->addWidget(groupBoxPerson);

        groupBoxPersonView = new QGroupBox(tab_view);
        groupBoxPersonView->setObjectName(QString::fromUtf8("groupBoxPersonView"));
        layoutPersonView = new QVBoxLayout(groupBoxPersonView);
        layoutPersonView->setObjectName(QString::fromUtf8("layoutPersonView"));
        personLabel = new QLabel(groupBoxPersonView);
        personLabel->setObjectName(QString::fromUtf8("personLabel"));
        personLabel->setMinimumHeight(240);
        personLabel->setFrameShape(QFrame::Box);
        personLabel->setAlignment(Qt::AlignCenter);
        personLabel->setScaledContents(false);

        layoutPersonView->addWidget(personLabel);


        viewHorizontalLayout->addWidget(groupBoxPersonView);

        tabWidget->addTab(tab_view, QString());

        centralVerticalLayout->addWidget(tabWidget);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Lidar Background Subtract", nullptr));
        lengthLabel->setText(QApplication::translate("MainWindow", "Length: -- m", nullptr));
        lengthLabel->setStyleSheet(QApplication::translate("MainWindow", "font: 14pt; color: white;", nullptr));
        widthLabel->setText(QApplication::translate("MainWindow", "Width: -- m", nullptr));
        widthLabel->setStyleSheet(QApplication::translate("MainWindow", "font: 14pt; color: white;", nullptr));
        yawLabel->setText(QApplication::translate("MainWindow", "Yaw: -- deg", nullptr));
        yawLabel->setStyleSheet(QApplication::translate("MainWindow", "font: 14pt; color: white;", nullptr));
        groupBoxParams->setTitle(QApplication::translate("MainWindow", "\345\217\202\346\225\260\350\256\276\347\275\256", nullptr));
        labelPort->setText(QApplication::translate("MainWindow", "\344\270\262\345\217\243\347\253\257\345\217\243:", nullptr));
        portEdit->setText(QApplication::translate("MainWindow", "/dev/ttyACM0", nullptr));
        labelBgSec->setText(QApplication::translate("MainWindow", "\350\203\214\346\231\257\351\207\207\351\233\206\347\247\222\346\225\260:", nullptr));
        labelFgSec->setText(QApplication::translate("MainWindow", "\345\211\215\346\231\257\351\207\207\351\233\206\347\247\222\346\225\260:", nullptr));
        labelVoxel->setText(QApplication::translate("MainWindow", "\344\275\223\347\264\240\344\270\213\351\207\207\346\240\267(m):", nullptr));
        labelDiff->setText(QApplication::translate("MainWindow", "\345\267\256\345\210\206\345\237\272\345\207\206\351\230\210\345\200\274(m):", nullptr));
        groupBoxROI->setTitle(QApplication::translate("MainWindow", "ROI\350\243\201\345\211\252\350\214\203\345\233\264 (m)", nullptr));
        labelXmin->setText(QApplication::translate("MainWindow", "X min:", nullptr));
        labelXmax->setText(QApplication::translate("MainWindow", "X max:", nullptr));
        labelYmin->setText(QApplication::translate("MainWindow", "Y min:", nullptr));
        labelYmax->setText(QApplication::translate("MainWindow", "Y max:", nullptr));
        labelZmin->setText(QApplication::translate("MainWindow", "Z min:", nullptr));
        labelZmax->setText(QApplication::translate("MainWindow", "Z max:", nullptr));
        groupBoxCloudDisplay->setTitle(QApplication::translate("MainWindow", "\347\202\271\344\272\221\346\230\276\347\244\272\351\200\211\346\213\251", nullptr));
        cbBackground->setText(QApplication::translate("MainWindow", "\350\203\214\346\231\257\347\202\271\344\272\221", nullptr));
        cbForeground->setText(QApplication::translate("MainWindow", "\345\211\215\346\231\257\347\202\271\344\272\221", nullptr));
        cbDiff->setText(QApplication::translate("MainWindow", "\345\267\256\345\210\206\347\273\223\346\236\234\347\202\271\344\272\221", nullptr));
        cbShowPlane->setText(QApplication::translate("MainWindow", "\345\217\240\345\212\240\346\230\276\347\244\272\345\271\263\351\235\242", nullptr));
        makeBgButton->setText(QApplication::translate("MainWindow", "\345\210\266\344\275\234\350\203\214\346\231\257", nullptr));
        recordFgButton->setText(QApplication::translate("MainWindow", "\345\275\225\345\205\245\345\211\215\346\231\257", nullptr));
        subtractButton->setText(QApplication::translate("MainWindow", "\345\201\232\345\267\256\345\210\206", nullptr));
        exportButton->setText(QApplication::translate("MainWindow", "\345\257\274\345\207\272PCD", nullptr));
        statusLabel->setText(QApplication::translate("MainWindow", "\345\260\261\347\273\252", nullptr));
        logText->setPlaceholderText(QApplication::translate("MainWindow", "\346\227\245\345\277\227\350\276\223\345\207\272...", nullptr));
        groupBoxCloudView->setTitle(QApplication::translate("MainWindow", "\347\202\271\344\272\221\346\230\276\347\244\272", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_param), QApplication::translate("MainWindow", "\345\260\272\345\272\246\346\265\213\351\207\217", nullptr));
        groupBoxPerson->setTitle(QApplication::translate("MainWindow", "\350\241\214\344\272\272\346\243\200\346\265\213", nullptr));
        labelSource->setText(QApplication::translate("MainWindow", "\350\276\223\345\205\245\346\272\220:", nullptr));
        rtspLabel->setText(QApplication::translate("MainWindow", "RTSP:", nullptr));
        rtspEdit->setText(QApplication::translate("MainWindow", "rtsp://", nullptr));
        engineLabel->setText(QApplication::translate("MainWindow", "Engine:", nullptr));
        engineEdit->setText(QApplication::translate("MainWindow", "person_tr/best_person_fp16.engine", nullptr));
        confLabel->setText(QApplication::translate("MainWindow", "\347\275\256\344\277\241\351\230\210\345\200\274:", nullptr));
        startPersonBtn->setText(QApplication::translate("MainWindow", "\345\220\257\345\212\250\346\243\200\346\265\213", nullptr));
        stopPersonBtn->setText(QApplication::translate("MainWindow", "\345\201\234\346\255\242\346\243\200\346\265\213", nullptr));
        labelAlarmIou->setText(QApplication::translate("MainWindow", "\345\221\212\350\255\246IoU\351\230\210\345\200\274:", nullptr));
        btnSetAlarmRoi->setText(QApplication::translate("MainWindow", "\350\256\276\347\275\256\345\221\212\350\255\246\345\214\272\345\237\237", nullptr));
        btnClearAlarmRoi->setText(QApplication::translate("MainWindow", "\346\270\205\351\231\244\345\221\212\350\255\246\345\214\272\345\237\237", nullptr));
        alarmStatusLabel->setText(QApplication::translate("MainWindow", "\345\221\212\350\255\246\347\212\266\346\200\201: \346\234\252\345\221\212\350\255\246", nullptr));
        groupBoxPersonView->setTitle(QApplication::translate("MainWindow", "\350\241\214\344\272\272\346\243\200\346\265\213\347\224\273\351\235\242", nullptr));
        personLabel->setText(QApplication::translate("MainWindow", "\350\241\214\344\272\272\346\243\200\346\265\213\347\224\273\351\235\242", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_view), QApplication::translate("MainWindow", "\350\241\214\344\272\272\346\243\200\346\265\213", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
