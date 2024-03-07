#include "mainwindow.h"
#include "QTextCodec"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec *codec=QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);

    #if QT_VERSION < QT_VERSION_CHECK(5,0,0)
      QTextCodec::setCodecForCStrings(codec);
      QTextCodec::setCodecForTr(codec);
    #endif

    MainWindow w;
    w.show();
    return a.exec();
}
