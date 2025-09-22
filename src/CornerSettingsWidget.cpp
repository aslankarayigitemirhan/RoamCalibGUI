#include "CornerSettingsWidget.h"
#include <QFormLayout>

CornerSettingsWidget::CornerSettingsWidget(QWidget* parent): QWidget(parent) {
    methodCombo = new QComboBox(this);
    methodCombo->addItem("Shi-Tomasi (goodFeaturesToTrack)", (int)CornerMethod::ShiTomasi);
    methodCombo->addItem("Harris", (int)CornerMethod::Harris);
    methodCombo->addItem("FAST", (int)CornerMethod::FAST);
    methodCombo->addItem("ORB (for viz)", (int)CornerMethod::ORB_);

    auto *lay = new QFormLayout(this);

    maxCorners = new QSpinBox(this); maxCorners->setRange(10, 5000); maxCorners->setValue(500);
    quality = new QDoubleSpinBox(this); quality->setRange(0.0001, 0.5); quality->setDecimals(4); quality->setValue(0.01);
    minDist = new QDoubleSpinBox(this); minDist->setRange(0, 50); minDist->setDecimals(2); minDist->setValue(5.0);
    blockSize = new QSpinBox(this); blockSize->setRange(3, 15); blockSize->setSingleStep(2); blockSize->setValue(3);
    harrisK = new QDoubleSpinBox(this); harrisK->setRange(0.01, 0.2); harrisK->setDecimals(3); harrisK->setValue(0.04);

    fastThresh = new QSpinBox(this); fastThresh->setRange(1, 100); fastThresh->setValue(20);
    fastNonmax = new QComboBox(this); fastNonmax->addItems({"true","false"}); fastNonmax->setCurrentIndex(0);

    termIters = new QSpinBox(this); termIters->setRange(1, 1000); termIters->setValue(30);
    termEps = new QDoubleSpinBox(this); termEps->setRange(1e-6, 1.0); termEps->setDecimals(6); termEps->setValue(0.01);
    winSize = new QSpinBox(this); winSize->setRange(1, 21); winSize->setValue(5);

    lay->addRow("Method:", methodCombo);
    lay->addRow("Max corners:", maxCorners);
    lay->addRow("Quality (Shi):", quality);
    lay->addRow("Min distance:", minDist);
    lay->addRow("Block size:", blockSize);
    lay->addRow("Harris k:", harrisK);
    lay->addRow("FAST threshold:", fastThresh);
    lay->addRow("FAST nonmax:", fastNonmax);
    lay->addRow("cornerSubPix win:", winSize);
    lay->addRow("Term max iters:", termIters);
    lay->addRow("Term eps:", termEps);

    connect(methodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int){emitChange();});
    auto onNum = [this](auto){ emitChange(); };
    connect(maxCorners, QOverload<int>::of(&QSpinBox::valueChanged), this, onNum);
    connect(blockSize, QOverload<int>::of(&QSpinBox::valueChanged), this, onNum);
    connect(fastThresh, QOverload<int>::of(&QSpinBox::valueChanged), this, onNum);
    connect(winSize, QOverload<int>::of(&QSpinBox::valueChanged), this, onNum);
    connect(termIters, QOverload<int>::of(&QSpinBox::valueChanged), this, onNum);
    connect(quality, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, onNum);
    connect(minDist, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, onNum);
    connect(harrisK, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, onNum);
    connect(termEps, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, onNum);
}

CornerSettings CornerSettingsWidget::settings() const {
    CornerSettings s;
    s.method = (CornerMethod)methodCombo->currentData().toInt();
    s.maxCorners = maxCorners->value();
    s.quality = quality->value();
    s.minDistance = minDist->value();
    s.blockSize = blockSize->value();
    s.harrisK = harrisK->value();
    s.fastThreshold = fastThresh->value();
    s.fastNonmax = fastNonmax->currentIndex()==0;
    s.termMaxIters = termIters->value();
    s.termEps = termEps->value();
    s.winSize = winSize->value();
    s.useHarris = (s.method == CornerMethod::Harris);
    return s;
}

void CornerSettingsWidget::apply(const CornerSettings& s) {
    methodCombo->setCurrentIndex((int)s.method);
    maxCorners->setValue(s.maxCorners);
    quality->setValue(s.quality);
    minDist->setValue(s.minDistance);
    blockSize->setValue(s.blockSize);
    harrisK->setValue(s.harrisK);
    fastThresh->setValue(s.fastThreshold);
    fastNonmax->setCurrentIndex(s.fastNonmax ? 0 : 1);
    termIters->setValue(s.termMaxIters);
    termEps->setValue(s.termEps);
    winSize->setValue(s.winSize);
    emitChange();
}

void CornerSettingsWidget::emitChange() { emit settingsChanged(settings()); }
