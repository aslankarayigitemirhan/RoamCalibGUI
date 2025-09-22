#pragma once
#include <QWidget>
#include <QListWidget>
#include <QListView>
#include <vector>

struct FrameItem
{
    QImage thumb;
    bool included;
    QString path;
};

class CaptureGallery : public QWidget
{
    Q_OBJECT
public:
    explicit CaptureGallery(QWidget *parent = nullptr);

    void clear();
    void addItem(const QImage &thumb, const QString &path);
    std::vector<FrameItem> items() const;
    std::vector<int> includedIndices() const;

    void setThumbnailSize(const QSize &size);
    void setGridMode(bool grid);
    void markExcluded(int idx); // 🔹 yeni fonksiyon

protected:
    void resizeEvent(QResizeEvent *ev) override;

signals:
    void toggled(int index, bool included);

private:
    QListWidget *list;
    QSize thumbSize = QSize(160, 120);
    bool gridMode = true;
};
