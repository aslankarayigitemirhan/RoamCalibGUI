#include "MainWindow.h"
#include "ResultsWindow.h"
#include "SettingsWindow.h"
#include "Utils.h"
#include <opencv2/videoio/registry.hpp>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QApplication>
#include <QToolBar>
#include <QAction>
#include <QInputDialog>

static void applyLightTheme()
{
    qApp->setStyleSheet(R"(
        QWidget {
            background: #f5f7fa;
            color: #1f2937;
            font-family: "Segoe UI", Roboto, "Helvetica Neue", Arial;
            font-size: 14px;
        }
        QToolBar { background: #ffffff; border-bottom: 1px solid #e5e7eb; spacing: 6px; }
        QToolButton { border-radius: 8px; padding: 6px 10px; }
        QToolButton:hover { background: #eef2ff; }
        QPushButton {
            min-height: 34px; padding: 6px 14px;
            border-radius: 8px; background-color: #2563eb; color: white;
        }
        QPushButton:hover { background-color: #1d4ed8; }
        QPushButton:disabled { background-color: #9ca3af; color: #e5e7eb; }
        QComboBox, QSpinBox, QDoubleSpinBox {
            border: 1px solid #d1d5db; border-radius: 6px; padding: 4px 8px; background: white;
        }
        .Card { background: #ffffff; border: 1px solid #e5e7eb; border-radius: 12px; }
        QListWidget::item { border: 1px solid #d1d5db; border-radius: 8px; margin: 4px; padding: 4px; }
        QListWidget::item:hover { border: 2px solid #2563eb; background: #eef2ff; }
        QListWidget::item:selected { border: 2px solid #1d4ed8; background: #dbeafe; }
    )");
}
static void applyDarkTheme()
{
    qApp->setStyleSheet(R"(
        QWidget { background: #0b1220; color: #e5e7eb; font-family: "Segoe UI", Roboto, "Helvetica Neue", Arial; font-size: 14px; }
        QToolBar { background: #0f172a; border-bottom: 1px solid #1f2937; spacing: 6px; }
        QToolButton { border-radius: 8px; padding: 6px 10px; color: #e5e7eb; }
        QToolButton:hover { background: #1f2937; }
        QPushButton { min-height: 34px; padding: 6px 14px; border-radius: 8px; background-color: #3b82f6; color: white; }
        QPushButton:hover { background-color: #2563eb; }
        QPushButton:disabled { background-color: #6b7280; color: #cbd5e1; }
        QComboBox, QSpinBox, QDoubleSpinBox { border: 1px solid #334155; border-radius: 6px; padding: 4px 8px; background: #111827; color: #e5e7eb; }
        .Card { background: #0f172a; border: 1px solid #1f2937; border-radius: 12px; }
        QListWidget::item { border: 1px solid #334155; border-radius: 8px; margin: 4px; padding: 4px; }
        QListWidget::item:hover { border: 2px solid #3b82f6; background: #111827; }
        QListWidget::item:selected { border: 2px solid #60a5fa; background: #0b1220; }
    )");
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    resize(1200, 800);
    setMinimumSize(1024, 720);
    setWindowTitle("Camera Calibration GUI");

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    applyLightTheme();

    // Toolbar
    QToolBar *toolbar = addToolBar("Main");
    toolbar->setMovable(false);
    QAction *actTheme = toolbar->addAction("🌙");
    actTheme->setCheckable(true);
    connect(actTheme, &QAction::toggled, this, [](bool dark)
            { if (dark) applyDarkTheme(); else applyLightTheme(); });

    // Top bar
    deviceCombo = new QComboBox(this);

    btnBack = new QPushButton("⟵ Back", this);
    btnBack->setEnabled(false);
    btnOpen = new QPushButton("Open Device", this);
    btnCapture = new QPushButton("Start Capture", this);
    btnCapture->setEnabled(false);
    btnCalibrate = new QPushButton("Run Calibration", this);
    btnCalibrate->setEnabled(false);
    btnImport = new QPushButton("Import Dataset", this);
    btnExport = new QPushButton("Export Dataset", this);
    btnExport->setEnabled(false);
    btnOpenSettings = new QPushButton("Open Settings", this);
    btnOpenResults = new QPushButton("Show Results", this);
    statusLbl = new QLabel("OS: " + utils::osName(), this);

    auto *top = new QHBoxLayout();
    top->addWidget(new QLabel("Device:"));
    top->addWidget(deviceCombo, 1);
    top->addWidget(btnOpen);
    top->addSpacing(10);
    top->addWidget(btnBack);
    top->addStretch();
    top->addWidget(statusLbl);

    // Preview (Distorted & Undistorted)
    preview = new PreviewPane(this);
    preview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Gallery
    gallery = new CaptureGallery(this);
    gallery->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto *midBtns = new QHBoxLayout();
    midBtns->addWidget(btnOpenSettings);
    midBtns->addSpacing(12);
    midBtns->addWidget(btnCapture);
    midBtns->addWidget(btnCalibrate);
    midBtns->addSpacing(12);
    midBtns->addWidget(btnOpenResults);
    midBtns->addStretch();
    midBtns->addWidget(btnImport);
    midBtns->addWidget(btnExport);

    // Optimize Dataset
    btnOptimize = new QPushButton("Optimize Dataset", this);
    btnOptimize->setEnabled(false); 
    midBtns->addWidget(btnOptimize);

    auto *mainLay = new QVBoxLayout(central);
    mainLay->addLayout(top);
    mainLay->addWidget(preview, 4);
    mainLay->addWidget(gallery, 2);
    mainLay->addLayout(midBtns);

    connect(btnOptimize, &QPushButton::clicked, this, &MainWindow::optimizeDataset);

    // Connections
    connect(&timer, &QTimer::timeout, this, &MainWindow::grabAndTryAccept);
    connect(btnOpen, &QPushButton::clicked, this, &MainWindow::openSelectedDevice);
    connect(btnCapture, &QPushButton::clicked, this, &MainWindow::handleCaptureClicked);
    connect(btnCalibrate, &QPushButton::clicked, this, &MainWindow::runCalibration);
    connect(btnImport, &QPushButton::clicked, this, &MainWindow::importDataset);
    connect(btnExport, &QPushButton::clicked, this, &MainWindow::exportDataset);
    connect(btnBack, &QPushButton::clicked, this, [this]
            {
        if (opened) { cap.release(); opened=false; }
        btnBack->setEnabled(false);
        btnOpen->setEnabled(true);
        deviceCombo->setEnabled(true);
        btnCapture->setEnabled(false);
        statusLbl->setText("Closed. Select device again."); });
    connect(btnOpenSettings, &QPushButton::clicked, this, &MainWindow::openSettingsWindow);
    connect(btnOpenResults, &QPushButton::clicked, this, &MainWindow::openResultsWindow);

    refreshDevices();
}

MainWindow::~MainWindow()
{
    if (cap.isOpened())
        cap.release();
}

// -------- Capture --------
void MainWindow::handleCaptureClicked()
{
    if (!opened) return;

    if (!capturing)
    {
        capturing = true;
        timer.start(60);
        btnCapture->setText("Capture Frame");
        statusLbl->setText("Capturing... press again to save current frame");
    }
    else
    {
        if (!lastDetectedCorners.empty())
        {
            acceptedFrames.push_back(lastFrame.clone());
            QImage thumb = utils::matToQImage(lastFrame);
            gallery->addItem(thumb, QString("frame_%1").arg(acceptedFrames.size()));

            btnCalibrate->setEnabled(acceptedFrames.size() >= 5);
            btnExport->setEnabled(true);
            btnOptimize->setEnabled(acceptedFrames.size() >= 5);   

            statusLbl->setText("Frame captured.");
        }
        else
        {
            statusLbl->setText("No valid pattern detected. Try again.");
        }
    }
}

// -------- Import --------
void MainWindow::importDataset()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select dataset folder (images)");
    if (dir.isEmpty())
        return;
    QStringList filters = {"*.png", "*.jpg", "*.jpeg", "*.bmp"};
    QDir d(dir);
    auto files = d.entryList(filters, QDir::Files, QDir::Name);
    if (files.isEmpty())
    {
        QMessageBox::information(this, "Import", "No images found.");
        return;
    }

    int imported = 0;
    for (auto &f : files)
    {
        QString path = dir + "/" + f;
        QImage img(path);
        if (img.isNull()) continue;

        cv::Mat m = utils::qimageToMat(img);
        if (datasetTargetSize.isValid() && datasetTargetSize.width() > 0 && datasetTargetSize.height() > 0)
            cv::resize(m, m, cv::Size(datasetTargetSize.width(), datasetTargetSize.height()));

        if (calib.acceptFrame(m))
        {
            acceptedFrames.push_back(m);
            gallery->addItem(img, f);
            imported++;
        }
    }

    if (imported == 0)
        QMessageBox::warning(this, "Import", "No valid calibration patterns detected.");
    else
        statusLbl->setText(QString("Imported %1 valid frames.").arg(imported));

    btnCalibrate->setEnabled(acceptedFrames.size() >= 5);
    btnExport->setEnabled(!acceptedFrames.empty());
    btnOptimize->setEnabled(acceptedFrames.size() >= 5);   
}

// -------- Device --------
void MainWindow::refreshDevices()
{
    deviceCombo->clear();
    auto list = devMgr.listDevices();
    if (list.isEmpty())
    {
        deviceCombo->addItem("No cameras found", "");
        btnOpen->setEnabled(false);
    }
    else
    {
        for (auto &path : list)
        {
            deviceCombo->addItem(path, path); // label: /dev/video0, data: aynı
        }
        btnOpen->setEnabled(true);
    }
}
void MainWindow::optimizeDataset() {
    if (acceptedFrames.empty()) {
        QMessageBox::information(this, "Optimize", "No dataset loaded.");
        return;
    }

    // Yeni geçici kalibrasyon
    CalibrationController temp;
    temp.setPattern(calib.pattern());
    temp.setCorner(calib.corner());
    temp.setModel(calib.model());

    for (auto &m : acceptedFrames)
        if (!m.empty()) temp.acceptFrame(m);

    CalibrationResult res;
    if (!temp.calibrate(res)) {
        QMessageBox::warning(this, "Optimize", "Calibration failed, cannot optimize.");
        return;
    }

    // Ortalama ve standart sapma hesapla
    double meanErr = 0.0;
    for (double e : res.perViewError) meanErr += e;
    meanErr /= res.perViewError.size();

    double var = 0.0;
    for (double e : res.perViewError) var += (e - meanErr) * (e - meanErr);
    var /= res.perViewError.size();
    double sigma = std::sqrt(var);

    double threshold = meanErr + sigma;  

    QVector<int> badIdx;
    for (int i = 0; i < (int)res.perViewError.size(); ++i) {
        if (res.perViewError[i] > threshold)
            badIdx.push_back(i);
    }

    if (badIdx.isEmpty()) {
        QMessageBox::information(this, "Optimize",
                                 QString("All frames are within mean+sigma (%.3f px).").arg(threshold));
        return;
    }

    // Kullanıcıya sor: kaç tanesini çıkaralım?
    bool ok;
    int x = QInputDialog::getInt(this, "Exclude Frames",
                                 QString("Mean = %1, σ = %2\nThreshold = %3 px\n%4 frames above threshold.\nHow many to exclude?")
                                     .arg(meanErr, 0, 'f', 3)
                                     .arg(sigma, 0, 'f', 3)
                                     .arg(threshold, 0, 'f', 3)
                                     .arg(badIdx.size()),
                                 std::min<int>(badIdx.size(), 3), 1, badIdx.size(), 1, &ok);
    if (!ok) return;

    // En kötü x frame çıkar
    std::vector<std::pair<int,double>> ranked;
    for (int idx : badIdx)
        ranked.push_back({idx, res.perViewError[idx]});

    std::sort(ranked.begin(), ranked.end(),
              [](auto&a, auto&b){ return a.second > b.second; });

    for (int i=0; i<x; ++i) {
        int idx = ranked[i].first;
        acceptedFrames[idx] = cv::Mat();      
        gallery->markExcluded(idx);           
    }

    QMessageBox::information(this, "Optimize",
                             QString("Excluded %1 worst frames above mean+σ (%.3f px).").arg(x).arg(threshold));

    btnCalibrate->setEnabled(gallery->includedIndices().size() >= 5);
}



void MainWindow::openSelectedDevice()
{
    int idx = deviceCombo->currentIndex();
    if (idx < 0)
        return;

    QString devPath = deviceCombo->currentText(); // Örn: "/dev/video0"
    bool ok = false;

    auto available = cv::videoio_registry::getBackends();

    // --- V4L2 backend ---
    if (std::find(available.begin(), available.end(), cv::CAP_V4L2) != available.end())
    {
        qInfo() << "[Device] Trying (V4L2):" << devPath;
        ok = cap.open(devPath.toStdString(), cv::CAP_V4L2);
    }

    // --- GStreamer backend ---
    if (!ok && std::find(available.begin(), available.end(), cv::CAP_GSTREAMER) != available.end())
    {
        qInfo() << "[Device] Trying (GStreamer):" << devPath;
        ok = cap.open("v4l2://" + devPath.toStdString(), cv::CAP_GSTREAMER);
    }

    // --- Default backend ---
    if (!ok)
    {
        qWarning() << "[Device] Trying default backend for:" << devPath;
        ok = cap.open(devPath.toStdString());
    }

    // Kamera açılamadıysa
    if (!ok || !cap.isOpened())
    {
        QMessageBox::warning(this, "Open", "Failed to open camera: " + devPath);
        return;
    }

    // Kamera parametreleri
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    cap.set(cv::CAP_PROP_FPS, 30);

    // İlk frame’i test et
    cv::Mat test;
    cap >> test;
    if (test.empty())
    {
        QMessageBox::warning(this, "Open", "Camera opened but did not return frames!");
        qCritical() << "[Device] Opened but no frames captured from" << devPath;
        cap.release();
        return;
    }

    qInfo() << "[Device] Camera opened with backend:"
            << QString::fromStdString(cap.getBackendName())
            << " Size:" << test.cols << "x" << test.rows;

    opened = true;
    btnBack->setEnabled(true);
    deviceCombo->setEnabled(false);
    btnOpen->setEnabled(false);
    btnCapture->setEnabled(true);
    statusLbl->setText("Opened camera " + devPath + " via " + QString::fromStdString(cap.getBackendName()));
}

// -------- Capture & Live --------

void MainWindow::grabAndTryAccept()
{
    if (!cap.isOpened())
    {
        qWarning() << "[grabAndTryAccept] cap is not opened!";
        return;
    }

    cv::Mat frame;
    cap >> frame;

    if (frame.empty())
    {
        qWarning() << "[grabAndTryAccept] Empty frame from camera!";
        return;
    }

    qInfo() << "[grabAndTryAccept] Captured frame size:" << frame.cols << "x" << frame.rows;

    // --- Pattern detection ---
    cv::Mat gray;
    if (frame.channels() == 3)
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    else
        gray = frame;

    std::vector<cv::Point2f> corners;
    cv::Size patSize;
    bool found = calib.detectPattern(gray, corners, patSize);

    // Distorted panel için frame üzerinde çizim yapıyoruz
    cv::Mat vis = frame.clone();
    if (found)
    {
        cv::drawChessboardCorners(vis, patSize, corners, true);
        if (utils::isPatternWellFramed(corners, gray.size()))
        {
            lastDetectedCorners = corners;
            lastFrame = frame.clone();
        }
        else
        {
            cv::rectangle(vis, {5, 5, vis.cols - 10, vis.rows - 10}, {0, 0, 255}, 2);
            lastDetectedCorners.clear();
        }
    }
    else
    {
        lastDetectedCorners.clear();
    }

    // --- Undistortion (only if calibration done) ---
    cv::Mat und;
    if (calib.isCalibrated())
        und = calib.undistortFrame(frame);

    // --- UI Update ---
    // Sol panel: çizimli distorted
    // Sağ panel: undistorted (varsa)
    preview->updateFrames(vis, und);
}

// -------- Calibration --------
void MainWindow::runCalibration()
{
    auto idxs = gallery->includedIndices();
    if (idxs.size() < 5)
    {
        QMessageBox::information(this, "Calibrate", "Select at least 5 frames.");
        return;
    }

    CalibrationController temp;
    temp.setPattern(calib.pattern());
    temp.setCorner(calib.corner());
    temp.setModel(calib.model());

    for (int id : idxs)
    {
        cv::Mat m = acceptedFrames[id];
        if (datasetTargetSize.isValid() && datasetTargetSize.width() > 0 && datasetTargetSize.height() > 0)
        {
            cv::resize(m, m, cv::Size(datasetTargetSize.width(), datasetTargetSize.height()));
        }
        temp.acceptFrame(m);
    }

    CalibrationResult res;
    if (!temp.calibrate(res))
    {
        QMessageBox::warning(this, "Calibrate", "Calibration failed.");
        return;
    }

    
    calib.setPattern(temp.pattern());
    calib.setCorner(temp.corner());
    calib.setModel(temp.model());
    calib.applyResult(res);

    ResultsWindow dlg(res,
                      temp.imagePoints(), temp.objectPoints(),
                      temp.imageSize(),
                      temp.patternSize(),
                      this);
    dlg.exec();

    statusLbl->setText("Calibration successful. RMS = " + QString::number(res.rms, 'f', 3));
}


void MainWindow::exportDataset()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select folder to save accepted frames");
    if (dir.isEmpty())
        return;
    auto idxs = gallery->includedIndices();
    int count = 0;
    for (int id : idxs)
    {
        QString path = QString("%1/frame_%2.png").arg(dir).arg(id);
        QImage img = utils::matToQImage(acceptedFrames[id]);
        img.save(path);
        ++count;
    }
    QMessageBox::information(this, "Export", QString("Saved %1 images.").arg(count));
}

// -------- Windows --------
void MainWindow::openSettingsWindow()
{
    SettingsWindow dlg(this);
    dlg.setInitial(calib.pattern(), calib.corner(),
                   calib.model(), datasetTargetSize);

    connect(&dlg, &SettingsWindow::parametersSaved,
            this, &MainWindow::applySettingsFromDialog);
    dlg.exec();
}

void MainWindow::openResultsWindow()
{
    QMessageBox::information(this, "Results", "Run calibration first to see results.");
}
void MainWindow::applySettingsFromDialog(const PatternConfig &p, const CornerSettings &c,
                                         CameraModel m, const QSize &dsSize)
{
    calib.setPattern(p);
    calib.setCorner(c);
    calib.setModel(m);
    datasetTargetSize = dsSize;
    statusLbl->setText("Settings saved.");
}
