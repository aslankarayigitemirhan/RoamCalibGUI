#include "SettingsWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>

SettingsWindow::SettingsWindow(QWidget* parent): QDialog(parent) {
    setWindowTitle("Setting Calibration Environment & Parameters");
    resize(800, 600);

    patternW = new PatternSettingsWidget(this);
    cornerW  = new CornerSettingsWidget(this);

    modelCombo = new QComboBox(this);
    modelCombo->addItem("Pinhole", (int)CameraModel::Pinhole);
    modelCombo->addItem("Fisheye", (int)CameraModel::Fisheye);

    imgW = new QSpinBox(this); imgW->setRange(0, 8192); imgW->setValue(0);
    imgH = new QSpinBox(this); imgH->setRange(0, 8192); imgH->setValue(0);

    btnSave = new QPushButton("Save Parameters", this);
    btnExit = new QPushButton("Exit", this);
    btnBack = new QPushButton("Go to MainWindow", this);

    auto *topLay = new QHBoxLayout();
    auto *colL = new QVBoxLayout();
    auto *colR = new QVBoxLayout();

    colL->addWidget(new QLabel("Camera Model")); colL->addWidget(modelCombo);
    colL->addSpacing(8);
    colL->addWidget(new QLabel("Dataset Image Size (0=keep)"));
    auto *sizeLay = new QHBoxLayout();
    sizeLay->addWidget(new QLabel("W:")); sizeLay->addWidget(imgW);
    sizeLay->addWidget(new QLabel("H:")); sizeLay->addWidget(imgH);
    colL->addLayout(sizeLay);
    colL->addStretch();

    colR->addWidget(new QLabel("Calibration Pattern"));
    colR->addWidget(patternW);
    colR->addSpacing(8);
    colR->addWidget(new QLabel("Corner Detector Algorithm Selection"));
    colR->addWidget(cornerW);
    colR->addStretch();

    topLay->addLayout(colL,1);
    topLay->addLayout(colR,2);

    auto *btns = new QHBoxLayout();
    btns->addWidget(btnBack);
    btns->addStretch();
    btns->addWidget(btnExit);
    btns->addWidget(btnSave);

    auto *main = new QVBoxLayout(this);
    main->addLayout(topLay, 1);
    main->addLayout(btns);

    connect(btnSave, &QPushButton::clicked, this, &SettingsWindow::onSave);
    connect(btnExit, &QPushButton::clicked, this, &SettingsWindow::reject);
    connect(btnBack, &QPushButton::clicked, this, &SettingsWindow::accept);
}

void SettingsWindow::setInitial(const PatternConfig& p, const CornerSettings& c,
                                CameraModel m, const QSize& datasetSize) {
    p_ = p; c_ = c; m_ = m; dsSize_ = datasetSize;
    patternW->apply(p_);
    cornerW->apply(c_);
    int idx = modelCombo->findData((int)m_);
    if (idx>=0) modelCombo->setCurrentIndex(idx);
    imgW->setValue(dsSize_.width());
    imgH->setValue(dsSize_.height());
}

void SettingsWindow::onSave() {
    PatternConfig np = patternW->config();
    CornerSettings nc = cornerW->settings();
    CameraModel nm = (CameraModel)modelCombo->currentData().toInt();
    QSize ds(imgW->value(), imgH->value());
    emit parametersSaved(np, nc, nm, ds);
    accept();
}
