#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 设置全局字体，防止不同平台显示差异
    QFont font("Microsoft YaHei", 10);
    a.setFont(font);

    MainWindow w;
    w.show();
    return a.exec();
}
