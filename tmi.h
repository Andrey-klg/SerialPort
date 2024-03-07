#ifndef TMI_H
#define TMI_H

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QApplication>
#include <QThread>
#include "QDir"


//const static QString serial_global = "00000000";

class Tmi : public QThread
{
    Q_OBJECT
public:
    explicit Tmi(QObject *parent = 0);

    int getPacket();
    quint64 selectedFileSize() { return file_size; }
    void resetFileMaps();


    virtual void run();

    void setCropFlag(bool flag) { cropFiles = flag; }
    void setGenName(QString name);
    void setFile(QString name = "f1.txt");
    void setParceFile(QString path); // Полный путь
    QString getFile() { return inFile; }

    ~Tmi();

signals:
    void finishedRecord(QString path);
    void progress(int pr);

private slots:
    QString timeState(uint sec);
    QString timeState(uint sec, uint msec);

private:
    enum marker {
        marker_A = 'A',   // M2-1 TI
        marker_B = 'B',   // M2-2 TI
        marker_C = 'C',   // M2-1 OK
        marker_D = 'D'    // M2-2 RK
    }; // байт - маркер пакета

    QString inFile;
    QString full_path;
    QString genName;

    QMap<uchar,QFile*> filemap;
    QMap<uchar,QString> filenamemap;
    QMap<uchar,int> counterTimeMap;


    int packet;

    bool cropFiles;

    QByteArray mainBuffer;
    quint64 time_prev;
    quint64 file_size;

    void recordLpi(QByteArray inbuffer, QString savePath,QString baseFile);
    bool watchStatusParcel(QByteArray inbuffer);

    QString addSpace(QString str,short shift);
    void checkFileEndBySize(uchar type);

};

#endif // TMI_H
