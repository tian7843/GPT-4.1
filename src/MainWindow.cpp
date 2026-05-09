#include "MainWindow.h"
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), imgWidget(new ImageWidget(this))
{
    setCentralWidget(imgWidget);
    setupMenu();
    setWindowTitle("灰度图像查看器");
    resize(1024, 768);
}

void MainWindow::setupMenu() {
    QMenu *fileMenu = menuBar()->addMenu("&文件");
    QAction *openAct = fileMenu->addAction("打开图像", this, SLOT(openImage()));
    fileMenu->addSeparator();
    QAction *exitAct = fileMenu->addAction("退出", qApp, SLOT(quit()));
}

void MainWindow::openImage() {
    QString fname = QFileDialog::getOpenFileName(this, "选择图像",
        "", "Images (*.jpg *.png *.bmp *.tif *.raw *.dcm)");
    if (!fname.isEmpty()) {
        if (!imgWidget->loadImage(fname)) {
            QMessageBox::warning(this, "错误", "无法打开所选图像文件。");
        }
    }
}
