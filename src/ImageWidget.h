#pragma once
#include <QWidget>
#include <QImage>
#include <QRect>
#include <memory>

enum RawType { RAW_16BIT, RAW_8BIT };

class ImageWidget : public QWidget {
    Q_OBJECT
public:
    explicit ImageWidget(QWidget *parent = nullptr);
    bool loadImage(const QString &path);
protected:
    void paintEvent(QPaintEvent *evt) override;
    void mousePressEvent(QMouseEvent *evt) override;
    void mouseMoveEvent(QMouseEvent *evt) override;
    void mouseReleaseEvent(QMouseEvent *evt) override;
    void contextMenuEvent(QContextMenuEvent *evt) override;
private:
    QImage displayImage;
    std::vector<uint16_t> origData16;
    std::vector<uint8_t> origData8;
    int width, height;
    bool is16Bit;
    QRect selectionRect;
    bool drawingRect;
    QPoint rectStart;
    // For DICOM
    bool loadDcm(const QString &path);
    // For RAW
    bool loadRaw(const QString &path);
    // For other formats
    bool loadCommon(const QString &path);
    void adjustImageGray(const QRect &roi);
    void resetDisplay();
};
