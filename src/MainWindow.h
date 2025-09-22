#pragma once
#include <QMainWindow>
#include <QTimer>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <opencv2/opencv.hpp>
#include "CalibrationController.h"
#include "CaptureGallery.h"
#include "PreviewPane.h"
#include "DeviceManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent=nullptr);
    ~MainWindow();

private slots:
    void openSelectedDevice();
    void refreshDevices();
    void handleCaptureClicked();
    void grabAndTryAccept();
    void runCalibration();
    void importDataset();
    void exportDataset();
    void openSettingsWindow();
    void openResultsWindow();
    void applySettingsFromDialog(const PatternConfig& p,
                                 const CornerSettings& c,
                                 CameraModel m,
                                 const QSize& dsSize);

private:
    // Video & zamanlayıcı
    cv::VideoCapture cap;
    QTimer timer;
QPushButton *btnOptimize = nullptr;
void optimizeDataset();

    // UI bileşenleri
    QComboBox *deviceCombo;
    QPushButton *btnOpen;
    QPushButton *btnBack;
    QPushButton *btnCapture;
    QPushButton *btnCalibrate;
    QPushButton *btnImport;
    QPushButton *btnExport;
    QPushButton *btnOpenSettings;
    QPushButton *btnOpenResults;
    QLabel *statusLbl;

    // Özel bileşenler
    CaptureGallery *gallery;
    PreviewPane *preview;
    DeviceManager devMgr;
    CalibrationController calib;

    // durum
    bool opened = false;
    bool capturing = false;

    // frame state
    std::vector<cv::Point2f> lastDetectedCorners;
    cv::Mat lastFrame;
    std::vector<cv::Mat> acceptedFrames;

    // dataset hedef boyutu (SettingsWindow’dan gelir)
    QSize datasetTargetSize;
};
