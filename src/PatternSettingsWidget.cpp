#include "PatternSettingsWidget.h"
#include <QFormLayout>
#include <QVBoxLayout>

PatternSettingsWidget::PatternSettingsWidget(QWidget* parent): QWidget(parent) {
    patternCombo = new QComboBox(this);
    patternCombo->addItem("Checkerboard", (int)PatternType::Checkerboard);
    patternCombo->addItem("Circles Grid (symmetric)", (int)PatternType::Circles);
    patternCombo->addItem("Circles Grid (asymmetric)", (int)PatternType::AsymmetricCircles);
    patternCombo->addItem("AprilTag", (int)PatternType::AprilTag);
    stack = new QStackedWidget(this);
    QWidget* cb = makeCheckerPage();
    QWidget* cg = makeCirclesPage(false);
    QWidget* ac = makeCirclesPage(true);
    QWidget* at = makeAprilTagPage();
    stack->addWidget(cb);
    stack->addWidget(cg);
    stack->addWidget(ac);
    stack->addWidget(at);

    auto *lay = new QVBoxLayout(this);
    lay->addWidget(patternCombo);
    lay->addWidget(stack);

    connect(patternCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx){
        stack->setCurrentIndex(idx);
        emitChange();
    });
}

QWidget* PatternSettingsWidget::makeCheckerPage() {
    QWidget* w = new QWidget(this);
    auto *f = new QFormLayout(w);
    cbCols = new QSpinBox(w); cbCols->setRange(2, 50); cbCols->setValue(9);
    cbRows = new QSpinBox(w); cbRows->setRange(2, 50); cbRows->setValue(6);
    cbSize = new QDoubleSpinBox(w); cbSize->setRange(0.001, 1.0); cbSize->setDecimals(4); cbSize->setValue(0.025);
    f->addRow("Inner columns:", cbCols);
    f->addRow("Inner rows:", cbRows);
    f->addRow("Square size (m):", cbSize);
    connect(cbCols, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int){emitChange();});
    connect(cbRows, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int){emitChange();});
    connect(cbSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double){emitChange();});
    return w;
}

QWidget* PatternSettingsWidget::makeCirclesPage(bool asymmetric) {
    QWidget* w = new QWidget(this);
    auto *f = new QFormLayout(w);
    QSpinBox *cols = new QSpinBox(w); cols->setRange(2, 50); cols->setValue(asymmetric?4:7);
    QSpinBox *rows = new QSpinBox(w); rows->setRange(2, 50); rows->setValue(asymmetric?11:7);
    QDoubleSpinBox* spacing = new QDoubleSpinBox(w); spacing->setRange(0.001, 1.0); spacing->setDecimals(4); spacing->setValue(0.01);
    if (asymmetric) { acCols=cols; acRows=rows; acSpacing=spacing; }
    else { cgCols=cols; cgRows=rows; cgSpacing=spacing; }
    f->addRow("Columns:", cols);
    f->addRow("Rows:", rows);
    f->addRow("Spacing (m):", spacing);
    connect(cols, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int){emitChange();});
    connect(rows, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int){emitChange();});
    connect(spacing, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double){emitChange();});
    return w;
}
QWidget* PatternSettingsWidget::makeAprilTagPage() {
    QWidget* w = new QWidget(this);
    auto *f = new QFormLayout(w);

    tagFamilyCombo = new QComboBox(w);
    tagFamilyCombo->addItem("TAG_36h11", 0);
    tagFamilyCombo->addItem("TAG_25h9", 1);
    tagFamilyCombo->addItem("TAG_16h5", 2);

    tagCols = new QSpinBox(w); tagCols->setRange(1, 50); tagCols->setValue(6);
    tagRows = new QSpinBox(w); tagRows->setRange(1, 50); tagRows->setValue(8);
    tagSizeSpin = new QDoubleSpinBox(w); tagSizeSpin->setRange(0.001, 1.0); tagSizeSpin->setDecimals(4); tagSizeSpin->setValue(0.04);

    f->addRow("Tag family:", tagFamilyCombo);
    f->addRow("Columns:", tagCols);
    f->addRow("Rows:", tagRows);
    f->addRow("Tag size (m):", tagSizeSpin);

    connect(tagFamilyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int){emitChange();});
    connect(tagCols, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int){emitChange();});
    connect(tagRows, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int){emitChange();});
    connect(tagSizeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double){emitChange();});

    return w;
}

PatternConfig PatternSettingsWidget::config() const {
    PatternConfig cfg;
    cfg.type = (PatternType)patternCombo->currentData().toInt();
    if (cfg.type == PatternType::Checkerboard) {
        cfg.innerSize = cv::Size(cbCols->value(), cbRows->value());
        cfg.squareSize = cbSize->value();
    } else if (cfg.type == PatternType::Circles) {
        cfg.circleGridSize = cv::Size(cgCols->value(), cgRows->value());
        cfg.symmetric = true; cfg.circleSpacing = cgSpacing->value();
    } else if(cfg.type == PatternType::AsymmetricCircles) {
        cfg.circleGridSize = cv::Size(acCols->value(), acRows->value());
        cfg.symmetric = false; cfg.circleSpacing = acSpacing->value();
    } else  {
        // AprilTag için özel ayarlar
        cfg.tagFamily = tagFamilyCombo->currentData().toInt();
    cfg.tagGridSize = cv::Size(tagCols->value(), tagRows->value());
    cfg.tagSize = tagSizeSpin->value();

        
    }
    return cfg;
}

void PatternSettingsWidget::apply(const PatternConfig& cfg) {
    patternCombo->setCurrentIndex((int)cfg.type);
    if (cfg.type == PatternType::Checkerboard) {
        cbCols->setValue(cfg.innerSize.width);
        cbRows->setValue(cfg.innerSize.height);
        cbSize->setValue(cfg.squareSize);
    } else if (cfg.type == PatternType::Circles) {
        cgCols->setValue(cfg.circleGridSize.width);
        cgRows->setValue(cfg.circleGridSize.height);
        cgSpacing->setValue(cfg.circleSpacing);
    } else if (cfg.type == PatternType::AsymmetricCircles) {
        acCols->setValue(cfg.circleGridSize.width);
        acRows->setValue(cfg.circleGridSize.height);
        acSpacing->setValue(cfg.circleSpacing);
    } else {
        tagFamilyCombo->setCurrentIndex(cfg.tagFamily);
        tagCols->setValue(cfg.tagGridSize.width);
        tagRows->setValue(cfg.tagGridSize.height);
        tagSizeSpin->setValue(cfg.tagSize);
    }
    emitChange();
}

void PatternSettingsWidget::emitChange() { emit patternChanged(config()); }
