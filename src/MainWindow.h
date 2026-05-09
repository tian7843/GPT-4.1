#pragma once
#include <QMainWindow>
#include "ImageWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
private:
    ImageWidget *imgWidget;
    void setupMenu();
private slots:
    void openImage();
};
