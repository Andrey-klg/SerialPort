#ifndef TMITHREAD_H
#define TMITHREAD_H

#if QT_VERSION > 0x050000
   #ifdef Q_OS_WIN
      #include  "SerialPort/qserialport.h"
      #include  "SerialPort/qserialportinfo.h"
   #else
      #include "QSerialPort"
      #include "QSerialPortInfo"
   #endif
#else
   #include  "SerialPort/qserialport.h"
   #include  "SerialPort/qserialportinfo.h"
#endif

#include <QWaitCondition>
#include <QFile>
#include <QDebug>

#include <QSharedPointer>
#include <QBuffer>
#include "QTimer"
#include "QMessageBox"


class TmiThread :  public QObject
{
    Q_OBJECT

public:
    TmiThread(QObject *object = 0);
    ~TmiThread();

    bool init(const QString &portName,QString fileRecord, qint32 baudRate);
    bool isOpen() { return portOpen; }
    bool isRun() { return isRunning; }
    bool write(QByteArray ba);

    void start();
    void stop(bool send = true);
    void createConnections(bool flag);
    void setRecordFile(QString file) { fileRecord = file; }
    void closeConnection();

    QStringList getPortList();

signals:
    void stopSignal();
    void globalError();
    void changeIndicate(bool);


private slots:
    void readSlot();
    void errorSlot(QSerialPort::SerialPortError error);
    void slotDelayIndicate();


private:

    QSerialPort *serial;

    bool portOpen;
    bool isRunning;

    QTimer *timer;
    QString fileRecord;
    QString portName;

};


#endif // TMITHREAD_H
