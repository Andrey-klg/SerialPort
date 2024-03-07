

#ifndef QSERIALPORT_P_H
#define QSERIALPORT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qserialport.h"

#include <private/qringbuffer_p.h>

QT_BEGIN_NAMESPACE

class QSerialPortErrorInfo
{
public:
    explicit QSerialPortErrorInfo(QSerialPort::SerialPortError newErrorCode = QSerialPort::UnknownError,
                                  const QString &newErrorString = QString());
    QSerialPort::SerialPortError errorCode;
    QString errorString;
};

class QSerialPortPrivateData
{
public:
    enum IoConstants {
        ReadChunkSize = 512,
        InitialBufferSize = 16384
    };

    QSerialPortPrivateData(QSerialPort *q);
    static int timeoutValue(int msecs, int elapsed);

    qint64 readBufferMaxSize;
    QRingBuffer readBuffer;
    QRingBuffer writeBuffer;
    QSerialPort::SerialPortError error;
    QString systemLocation;
    qint32 inputBaudRate;
    qint32 outputBaudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;
    QSerialPort::FlowControl flowControl;
    bool settingsRestoredOnClose;
    bool isBreakEnabled;
    QSerialPort * const q_ptr;
};

QT_END_NAMESPACE

#endif // QSERIALPORT_P_H
