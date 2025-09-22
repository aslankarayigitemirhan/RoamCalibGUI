#include "CaptureGallery.h"
#include <QHBoxLayout>
#include <QPixmap>
#include <QResizeEvent>

CaptureGallery::CaptureGallery(QWidget* parent): QWidget(parent) {
    list = new QListWidget(this);
    list->setViewMode(QListView::IconMode);
    list->setResizeMode(QListWidget::Adjust);
    list->setMovement(QListWidget::Static);
    list->setSpacing(8);
    list->setFlow(QListView::LeftToRight);
    list->setWrapping(true);
    list->setUniformItemSizes(false);
    list->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->addWidget(list);

    connect(list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* it){
        bool inc = (it->checkState() != Qt::Checked);
        it->setCheckState(inc ? Qt::Checked : Qt::Unchecked);
        emit toggled(list->row(it), inc);
    });

    setThumbnailSize(thumbSize);
}

void CaptureGallery::clear() { list->clear(); }

void CaptureGallery::addItem(const QImage& thumb, const QString& path) {
    auto pm = QPixmap::fromImage(thumb.scaled(thumbSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    auto *item = new QListWidgetItem(QIcon(pm), path.isEmpty() ? "frame" : path);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    list->addItem(item);
}

std::vector<FrameItem> CaptureGallery::items() const {
    std::vector<FrameItem> out; out.reserve(list->count());
    for (int i=0;i<list->count();++i) {
        auto *it = list->item(i);
        FrameItem f;
        f.thumb = it->icon().pixmap(thumbSize).toImage();
        f.included = (it->checkState()==Qt::Checked);
        f.path = it->text();
        out.push_back(f);
    }
    return out;
}

std::vector<int> CaptureGallery::includedIndices() const {
    std::vector<int> idx;
    for (int i=0;i<list->count();++i)
        if (list->item(i)->checkState()==Qt::Checked) idx.push_back(i);
    return idx;
}

void CaptureGallery::setThumbnailSize(const QSize& size) {
    thumbSize = size;
    list->setIconSize(size);
    // grid hücresi: ikona biraz padding
    list->setGridSize(QSize(thumbSize.width()+24, thumbSize.height()+36));
}

void CaptureGallery::setGridMode(bool grid) {
    gridMode = grid;
    list->setViewMode(grid ? QListView::IconMode : QListView::ListMode);
}

void CaptureGallery::resizeEvent(QResizeEvent* ev) {
    QWidget::resizeEvent(ev);
    // Qt kendisi IconMode + wrapping ile akış düzenini ayarlıyor.
    // Burada gridSize yeniden set edilerek daha iyi dolgu sağlanır.
    list->setGridSize(QSize(thumbSize.width()+24, thumbSize.height()+36));
}
void CaptureGallery::markExcluded(int idx) {
    if (idx >= 0 && idx < list->count()) {
        auto *item = list->item(idx);

        // Yazıyı kalın yap
        QFont f = item->font();
        f.setBold(true);
        item->setFont(f);

        // Arka planı gri yap
        item->setBackground(QBrush(Qt::lightGray));

        // İstersen kırmızı kenarlık da ekleyebilirsin:
        item->setData(Qt::UserRole, true); // "excluded" flag
    }
}
