#include "PreviewPane.h"
#include "Utils.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>

PreviewPane::PreviewPane(QWidget* parent): QWidget(parent) {
    distorted = new QLabel(this);
    undistorted = new QLabel(this);

    distorted->setMinimumSize(320,240);
    undistorted->setMinimumSize(320,240);

    distorted->setAlignment(Qt::AlignCenter);
    undistorted->setAlignment(Qt::AlignCenter);

    distorted->setText("Distorted (live)");
    undistorted->setText("Undistorted (live)");

    auto *left = new QVBoxLayout();
    left->addWidget(new QLabel("Original (Distorted)"));
    left->addWidget(distorted,1);

    auto *right = new QVBoxLayout();
    right->addWidget(new QLabel("Corrected (Undistorted)"));
    right->addWidget(undistorted,1);

    auto *lay = new QHBoxLayout(this);
    lay->addLayout(left,1);
    lay->addLayout(right,1);
}

void PreviewPane::updateFrames(const cv::Mat& d, const cv::Mat& u) {
    // Distorted her durumda güncellenir
    if (!d.empty()) {
        QImage qd = utils::matToQImage(d);
        distorted->setPixmap(
            QPixmap::fromImage(qd).scaled(
                distorted->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            )
        );
    } else {
        distorted->setText("No distorted frame");
    }

    // Undistorted yoksa distorted göster
    if (!u.empty()) {
        QImage qu = utils::matToQImage(u);
        undistorted->setPixmap(
            QPixmap::fromImage(qu).scaled(
                undistorted->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            )
        );
    } else if (!d.empty()) {
        QImage qd = utils::matToQImage(d);
        undistorted->setPixmap(
            QPixmap::fromImage(qd).scaled(
                undistorted->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            )
        );
    } else {
        undistorted->setText("No undistorted frame");
    }
}
