#include "ImageWidget.h"
#include <QPainter>
#include <QFileInfo>
#include <QImageReader>
#include <QMouseEvent>
#include <QInputDialog>
#include <QContextMenuEvent>
#include <QMenu>
#include <QMessageBox>
#include "DcmLoader.h"

// Helper: 归一化并转换至QImage以显示
template<typename T>
QImage toDisplayImage(const std::vector<T>& buffer, int w, int h, bool is16bit, T valmin=0, T valmax=std::numeric_limits<T>::max()) {
    QImage out(w, h, QImage::Format_Grayscale8);
    double scale = 255.0/(valmax-valmin);
    for(int y=0; y<h; ++y)
    for(int x=0; x<w; ++x) {
        int idx = y*w+x;
        auto v = buffer[idx];
        int g = int((v-valmin)*scale);
        if(g<0) g=0; if(g>255) g=255;
        out.setPixel(x,y, g);
    }
    return out;
}

ImageWidget::ImageWidget(QWidget *parent)
    : QWidget(parent), width(0), height(0), is16Bit(false), drawingRect(false)
{}

bool ImageWidget::loadImage(const QString& path) {
    QFileInfo fi(path);
    auto ext = fi.suffix().toLower();

    origData16.clear(); origData8.clear();

    if(ext=="dcm") {
        return loadDcm(path);
    } else if(ext=="raw") {
        return loadRaw(path);
    } else if(ext=="tif" || ext=="tiff") {
        QImageReader reader(path);
        reader.setAutoDetectImageFormat(true);
        QImage img = reader.read();
        if(img.isNull()) return false;
        width = img.width(); height = img.height();
        QImage gray = img.convertToFormat(QImage::Format_Grayscale16);
        origData16.resize(width*height);
        for(int y=0; y<height; ++y)
            for(int x=0; x<width; ++x)
                origData16[y*width+x] = gray.pixel(x,y) & 0xffff;
        is16Bit = true;
        displayImage = toDisplayImage(origData16, width, height, true);
        update();
        return true;
    } else if(ext=="jpg" || ext=="png" || ext=="bmp") {
        QImage img(path);
        if(img.isNull()) return false;
        width = img.width(); height = img.height();
        QImage gray = img.convertToFormat(QImage::Format_Grayscale8);
        origData8.resize(width*height);
        for(int y=0; y<height; ++y)
            for(int x=0; x<width; ++x)
                origData8[y*width+x] = gray.pixel(x,y) & 0xff;
        is16Bit = false;
        displayImage = gray;
        update();
        return true;
    } else {
        return false;
    }
}

bool ImageWidget::loadDcm(const QString& path) {
    int w,h;
    std::vector<uint16_t> data;
    if(DcmLoader::loadDcm(path, w, h, data)) {
        width = w; height = h;
        origData16 = data;
        is16Bit = true;
        displayImage = toDisplayImage(origData16, width, height, true);
        update();
        return true;
    }
    return false;
}

// RAW: 用户输入宽高
bool ImageWidget::loadRaw(const QString& fname) {
    QFile file(fname);
    if(!file.open(QIODevice::ReadOnly)) return false;
    bool ok;
    int w = QInputDialog::getInt(this, "输入RAW宽度", "宽度:", 512, 1, 4096, 1, &ok);
    if(!ok) return false;
    int h = QInputDialog::getInt(this, "输入RAW高度", "高度:", 512, 1, 4096, 1, &ok);
    if(!ok) return false;

    QByteArray arr = file.readAll();
    int n = arr.size();
    if(n == w*h*2) {
        origData16.resize(w*h);
        const uint16_t *data = reinterpret_cast<const uint16_t*>(arr.constData());
        for(int i=0;i<w*h;++i) origData16[i] = data[i];
        width=w; height=h; is16Bit=true;
        displayImage = toDisplayImage(origData16,w,h,true);
        update();
        return true;
    } else if(n == w*h) { // 8位
        origData8.resize(w*h);
        const uint8_t *data = reinterpret_cast<const uint8_t*>(arr.constData());
        for(int i=0;i<w*h;++i) origData8[i] = data[i];
        width=w; height=h; is16Bit=false;
        displayImage = toDisplayImage(origData8,w,h,false);
        update();
        return true;
    } else {
        QMessageBox::warning(this,"RAW尺寸不符","文件尺寸与输入宽高不匹配！");
        return false;
    }
}

void ImageWidget::paintEvent(QPaintEvent*) {
    if(displayImage.isNull()) return;
    QPainter p(this);
    double scale = std::min(
        (double)width()/displayImage.width(),
        (double)height()/displayImage.height());
    int sw = int(displayImage.width()*scale);
    int sh = int(displayImage.height()*scale);
    p.drawImage(QRect(0,0, sw, sh), displayImage);
    if(selectionRect.isValid()) {
        p.setPen(QPen(Qt::red, 2));
        QRect rect = selectionRect;
        rect = QRect(int(rect.left()*scale), int(rect.top()*scale),
                     int(rect.width()*scale), int(rect.height()*scale));
        p.drawRect(rect);
    }
}

void ImageWidget::mousePressEvent(QMouseEvent *evt) {
    if(evt->button()==Qt::RightButton) {
        rectStart = evt->pos();
        selectionRect = QRect(rectStart, QSize());
        drawingRect = true;
    }
}

void ImageWidget::mouseMoveEvent(QMouseEvent *evt) {
    if(drawingRect) {
        QPoint pt = evt->pos();
        int x0 = std::min(rectStart.x(), pt.x());
        int y0 = std::min(rectStart.y(), pt.y());
        int x1 = std::max(rectStart.x(), pt.x());
        int y1 = std::max(rectStart.y(), pt.y());
        // map画布-->原图（等比缩放）
        double scale = std::min((double)width()/displayImage.width(), (double)height()/displayImage.height());
        x0 = int(x0/scale); y0 = int(y0/scale); x1 = int(x1/scale); y1 = int(y1/scale);
        selectionRect = QRect(QPoint(x0,y0), QPoint(x1,y1));
        update();
    }
}

void ImageWidget::mouseReleaseEvent(QMouseEvent *evt) {
    if(drawingRect && evt->button()==Qt::RightButton) {
        drawingRect=false;
        // 保证rect不超界
        selectionRect = selectionRect.intersected(QRect(0,0,width,height));
        adjustImageGray(selectionRect);
    }
}

void ImageWidget::contextMenuEvent(QContextMenuEvent *evt) {
    QMenu menu(this);
    menu.addAction("重置灰度", [this](){ resetDisplay(); });
    menu.exec(evt->globalPos());
}

// 灰度调整核心：框内统计后全图线性拉伸
void ImageWidget::adjustImageGray(const QRect& roi) {
    if(width==0 || height==0) return;
    if(!roi.isValid()) return;
    // 统计ROI灰度最大/最小/平均
    uint16_t roi_min=65535, roi_max=0;
    uint32_t roi_sum=0; int roi_cnt=0;
    if(is16Bit) {
        for(int y=roi.top(); y<roi.bottom(); ++y)
            for(int x=roi.left(); x<roi.right(); ++x) {
                int idx=y*width+x;
                uint16_t v=origData16[idx];
                if(v<roi_min) roi_min=v; if(v>roi_max) roi_max=v;
                roi_sum += v; ++roi_cnt;
            }
        // 拉伸到ROI范围
        uint16_t minv=roi_min, maxv=roi_max;
        displayImage = toDisplayImage(origData16, width, height, true, minv, maxv);
    } else {
        uint8_t roi_min=255, roi_max=0;
        for(int y=roi.top(); y<roi.bottom(); ++y)
            for(int x=roi.left(); x<roi.right(); ++x) {
                int idx=y*width+x;
                uint8_t v=origData8[idx];
                if(v<roi_min) roi_min=v; if(v>roi_max) roi_max=v;
                roi_sum += v; ++roi_cnt;
            }
        // 拉伸到ROI范围
        uint8_t minv=roi_min, maxv=roi_max;
        displayImage = toDisplayImage(origData8, width, height, false, minv, maxv);
    }
    update();
}

void ImageWidget::resetDisplay() {
    if(is16Bit)
        displayImage = toDisplayImage(origData16, width, height, true);
    else
        displayImage = toDisplayImage(origData8, width, height, false);
    selectionRect = QRect();
    update();
}
