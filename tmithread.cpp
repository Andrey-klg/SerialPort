#include "tmithread.h"

TmiThread::TmiThread(QObject *object):
    QObject(object)
{
    serial = new QSerialPort(this);

    portOpen = false;
    isRunning = false;

    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),SLOT(readSlot()));
}

// ----------------------------------------

bool TmiThread::write(QByteArray ba)
{
    if (!serial->isOpen()) return false;
    int cnt = serial->write(ba);
    return true;
}

// ----------------------------------------

void TmiThread::createConnections(bool flag)
{
    if(flag)
    {
        connect(serial,SIGNAL(error(QSerialPort::SerialPortError)),this, SLOT(errorSlot(QSerialPort::SerialPortError)));
    }
    else
        disconnect(serial,SIGNAL(error(QSerialPort::SerialPortError)),this, SLOT(errorSlot(QSerialPort::SerialPortError)));
}

// ----------------------------------------

void TmiThread::closeConnection()
{
    serial->clear();
    if(serial->isOpen()) serial->close();
    createConnections(false);

    portOpen = false;
    isRunning = false;

}

// ----------------------------------------

QStringList TmiThread::getPortList()
{
    QStringList portNameList;

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){
        if (info.portName().contains("USB")){
            portNameList.append(info.portName());
        }
    }

    return portNameList;
}

// ----------------------------------------

void TmiThread::readSlot()
{
    QByteArray responseData =  serial->readAll();

    if (responseData.length() <= 0) return;

     emit changeIndicate(true);

     QFile file (fileRecord);
     file.open(QIODevice::Append);
     file.write(responseData);
     file.close();

     QTimer::singleShot(5,this,SLOT(slotDelayIndicate()));
}

// ----------------------------------------

void TmiThread::errorSlot(QSerialPort::SerialPortError error)
{
    if(timer->isActive()) timer->stop();

    QMessageBox *msg = new QMessageBox;
    if(serial->errorString() == "No error") {// Порт отключен - всё ок
        msg->setText("Порт " + portName + " отсоединён!");
        msg->setWindowTitle("");
    }
    else {
       msg->setText("Ошибка порта " + portName + ": " + serial->errorString());
       msg->setWindowTitle("Ошибка!");
    }
    msg->addButton(QMessageBox::Cancel);
    msg->setButtonText(QMessageBox::Cancel,"Закрыть");
    msg->exec();

    emit globalError();

    portOpen = false;
    isRunning = false;
    delete msg;
}

// ----------------------------------------

void TmiThread::slotDelayIndicate()
{
    emit changeIndicate(false);
}

// ----------------------------------------

TmiThread::~TmiThread()
{
    if(isRunning) stop(true);
    if(serial->isOpen()) serial->close();


    qDebug() << "destruct TmiThread" << serial->portName();
}

// ----------------------------------------

bool TmiThread::init(const QString &portName, QString fileRecord,qint32 baudRate)
{
    if(serial->isOpen()) serial->close();

    portOpen = false;

    this->fileRecord = fileRecord;
    this->portName = portName;

    serial->setPortName(portName);
    serial->setBaudRate(baudRate);

    int openCounter = 0;
    while (openCounter != 15){
        if (serial->open(QIODevice::ReadWrite)) {
            openCounter = 0;
            portOpen = true;
            break;
        }
        openCounter++;
    }

    if (openCounter) {

        QMessageBox *msg = new QMessageBox;
        msg->setText("Ошибка " + serial->errorString() + ". Не могу открыть выбранный порт!");
        msg->setWindowTitle("Ошибка!");
        msg->addButton(QMessageBox::Cancel);
        msg->setButtonText(QMessageBox::Cancel,"Закрыть");
        msg->exec();

        delete msg;

        return false;
    }

    return true;
}

// ----------------------------------------

void TmiThread::start()
{
    if(serial->bytesAvailable())
    {
        qDebug() << "serial->bytesAvailable()" << serial->bytesAvailable();
        serial->flush();
        serial->clear();
    }

    timer->start(32);

    char data[6] = {'U','p','n','U','s','t'};
    serial->write(data,6);

    isRunning = true;
}

// ----------------------------------------

void TmiThread::stop(bool send)
{
    createConnections(false);

    while(serial->bytesAvailable())
    {
        qDebug() << "stop serial->bytesAvailable()" << serial->bytesAvailable();
        serial->flush();
        serial->clear();
    }

    timer->stop();

    if (send) {
        char buf[3] = {0x55,0x73,0x70};
        int a = serial->write(buf,3);
        serial->waitForBytesWritten(100);
        serial->waitForBytesWritten(100);
        serial->waitForBytesWritten(100);
    }

    isRunning = false;
}

// ----------------------------------------
