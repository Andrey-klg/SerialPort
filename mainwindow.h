#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QMessageBox"
#include "QDir"
#include "QDesktopWidget"

#include "tmi.h"
#include "tmithread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    QString monLocalPath;

private slots:
    void on_chooseFile_clicked();
    void setStatus(bool status);
    void finishParseSlot(QString folderPath);

    void on_startParse_clicked();
    void startParcing();

    void setDefault();
    void on_startWriteButton_clicked();

    void on_refreshButton_clicked();

    void globalErrorExecute();

    void activateGUI();

    void on_connectButton_clicked(bool checked);
    void connectionState(bool flag);

    QStringList sortCat(QStringList list,QString filter);


private:
    bool runStatus;
    bool delFile;

    Tmi *tmi;
    TmiThread *portThread;


    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
