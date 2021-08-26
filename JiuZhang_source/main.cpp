#include "GUI.h"
#include <QFile>
#include <QApplication>

void setStyleSheet()
{//设置样式表
    QFile file("Resources\\white.qss");
    file.open(QIODevice::ReadOnly);
    QString styleSheet = file.readAll();
    qApp->setStyleSheet(styleSheet);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    setStyleSheet();

    myjz_519 w;
    w.show();
    return a.exec();
}
