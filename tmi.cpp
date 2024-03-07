#include "tmi.h"


Tmi::Tmi(QObject *parent) :
    QThread(parent)
{
    time_prev = 0;
    file_size = 0;

    cropFiles = true;

    filemap.insert(marker_A,new QFile(this));
    filemap.insert(marker_B,new QFile(this));
    filemap.insert(marker_C,new QFile(this));
    filemap.insert(marker_D,new QFile(this));

    filenamemap.insert(marker_A,"c1");
    filenamemap.insert(marker_B,"c2");
    filenamemap.insert(marker_C,"c1");
    filenamemap.insert(marker_D,"c2");

    counterTimeMap.insert(marker_A,1);
    counterTimeMap.insert(marker_B,1);
    counterTimeMap.insert(marker_C,1);
    counterTimeMap.insert(marker_D,1);

    inFile = qApp->applicationDirPath() + QDir::separator() + "f1.txt"; // Разбираемый файл
}

// ----------------------------------------------------

void Tmi::setFile(QString name)
{
    inFile = qApp->applicationDirPath() + QDir::separator() + name;\
    QFile::remove(inFile);
}
// ----------------------------------------------

void Tmi::setParceFile(QString path)
{
    inFile = path;
    QFile file(inFile);
    file_size = file.size();
}

// ----------------------------------------------

Tmi::~Tmi()
{
}

// ----------------------------------------------

int Tmi::getPacket()
{
    return packet;
}

// ----------------------------------------------

void Tmi::run()
{   
     time_prev = 0;

     QDir qd;
     QDir qd1;

     if(genName.isEmpty()) genName = "mon_" + QDateTime::currentDateTime().toString("ddMMyy_hhmmss");

     full_path = qApp->applicationDirPath() + QDir::separator() + genName;
     QString monFile = full_path + QDir::separator() + genName + ".mon";

     if (qd.exists(full_path)) qd.remove(full_path);
     qd.mkdir(full_path);

     QString src,dst;
     src =  inFile;
     dst =  full_path + QDir::separator() + "input_" + QFileInfo(inFile).baseName() + ".txt";

     QFile::copy(src,dst);

     QFile srcFile(src);


     QMap<uchar,QFile*>::iterator file_i;
     for (file_i = filemap.begin(); file_i != filemap.end(); ++file_i){
         file_i.value()->setFileName(full_path + QDir::separator() + genName  + "." + filenamemap[file_i.key()]);
     }


     if(srcFile.open(QIODevice::ReadOnly))
     {
         packet = 0;

         QMap<uchar,QFile*>::iterator file_i;
         for (file_i = filemap.begin(); file_i != filemap.end(); ++file_i){
             file_i.value()->open(QIODevice::Append);
         }

         while (!srcFile.atEnd()){
             QByteArray inbuffer = srcFile.read(4096);
             recordLpi(inbuffer,monFile,src);
             int prog = (srcFile.pos()*100/srcFile.size());
             emit progress(prog);
         }

              QTextStream stream;

              if(packet > 1) {
               if(filemap['A']->size() > 0) {
                stream.setDevice(filemap['A']);
                stream << QString("Разобрано пакетов: " + QString::number(packet));
                stream << "\n" << "\n"
                     << QString("Разбор завершен. Системное время: "
                      + QDateTime::currentDateTime().time().toString("hh:mm:ss"));
               }

               if(filemap['B']->size() > 0) {
                stream.setDevice(filemap['B']);
                stream << QString("Разобрано пакетов: " + QString::number(packet));
                stream << "\n" << "\n"
                     << QString("Разбор завершен. Системное время: "
                      + QDateTime::currentDateTime().time().toString("hh:mm:ss"));
               }
              } //end if(packet > 1)


         for (file_i = filemap.begin(); file_i != filemap.end(); ++file_i){
            if(file_i.value()) {
            if (file_i.value()->isOpen()) file_i.value()->close();
            if (!file_i.value()->size())
                file_i.value()->remove(); // Нулевые
            }
         }
    }

    srcFile.close();

    emit finishedRecord(full_path);
}

// ----------------------------------------------------

void Tmi::setGenName(QString name)
{
    genName = name;
}

// ----------------------------------------------------

void Tmi::checkFileEndBySize(uchar type)
{
     if((packet > 0) && !(packet % 10000))
     {
        counterTimeMap[type]++;
        filemap[type]->close();
        filemap[type]->setFileName(full_path + QDir::separator() + genName + "-" + QString::number(counterTimeMap[type]) + "." + filenamemap[type]);
        filemap[type]->remove();
        filemap[type]->open(QIODevice::Append);
        filemap[type]->seek(0);
    }
}

// ----------------------------------------------------------
 
QString Tmi::addSpace(QString str,short shift) //Добавляем пробелы Строку с файлом
{
     QString data = str;
     QString data_Buf = "";
     for(int i=0;i<data.length();i++)
     {
         data_Buf += data.at(i);
         if(i%shift == 0) data_Buf += "  ";
     }

     return data_Buf;
}

// ----------------------------------------------------------

void Tmi::recordLpi(QByteArray inbuffer, QString savePath, QString baseFile)
{
    QTextStream stream;

    mainBuffer.append(inbuffer);

    QByteArray pars;
    QByteArray data_KC; // BA Командного слова с перестановкой
    QByteArray data_OC; // BA Ответного слова с перестановкой

    QString time_str=""; // Текущее;

    quint64 time=0;

    quint8 KC[2];
    quint8 OC[2];

    quint64 basetime = 0;

    int i = 0;

    while(mainBuffer.length() != 0)
    {
       if((mainBuffer[i]=='A') || (mainBuffer[i]=='B'))
       {
           if(mainBuffer.length() < 10) break; // Не считаем счетчики КС

           stream.setDevice(filemap[mainBuffer[i]]);
            if(filemap[mainBuffer[i]]->size() == 0)
            {
                stream << ("Начало разбора файла " + QDateTime::currentDateTime().time().toString("hh:mm:ss"));
                stream << "\n" << ("Дата создания файла: " + QFileInfo(baseFile).created().toString("dd-MM-yyyy hh:mm:ss")) << "\n" << "\n";
            }

           i++;

           uchar mas[4];
           for (int t=0; t< 5; t++)
               mas[t] = mainBuffer.at(t+1);
           time = (*(int*)mas * 8);


           quint64 tt = time/1000000;
           int msec  = (time - (tt*1000000)) / 1000;

           
           if(cropFiles) checkFileEndBySize(mainBuffer[0]); // Интервал время больше 1800 секунд

           basetime = time; // mks
           time_str = timeState(tt,msec);


           i+=5;
           quint8 chanel = mainBuffer[i]; // 7й байт -  (0 - осн, 1-рез)
           i++;
           quint8 error = mainBuffer[i];  // 8й байт - ошибка
           i++;
           quint8 ks_counter = mainBuffer[i]; //9й байс счетчик длины кс
           quint8 os_counter = mainBuffer[i+1]; //10й байт счетчик длины Ос
           i+=2;

           //Разбор КС // 11-12 байты перевернуто
           KC[0] = mainBuffer[i+1];
           KC[1] = mainBuffer[i];

           quint8 adress = KC[0] >> 3; // (Если 1f Групповая)
           quint8 regim = KC[0] & 0x04; // 0 приём, 1 - передача
           quint8 subadress = (KC[0] & 0x03) << 3;
           subadress |= ((KC[1] & 0xE0) >>5 ); // Подадрес
           quint8 words = (KC[1] & 0x1F);

           i+=2;
           // Данные после КС (если есть) 13й итд


           int fullsize = ks_counter + os_counter + 10;
           if(fullsize > mainBuffer.size()) break;

           if(ks_counter > 2) {
             int j=0;
             while(j<ks_counter-2) {
                data_KC[j] = mainBuffer[i+1];
                data_KC[j+1] = mainBuffer[i];

                j+=2;  i+=2;
             }
           }

           // ---------
           //Разбор ОС (перевернуто)
           OC[0] = mainBuffer[i+1];
           OC[1] = mainBuffer[i];

           i+=2;

           if(os_counter>2)
           {
               int j=0;
               while(j<os_counter-2)
               {
                   data_OC[j] = mainBuffer[i+1];
                   data_OC[j+1] = mainBuffer[i];

                   j+=2;  i+=2;
               }
           }


           mainBuffer.remove(0,fullsize);
           i = 0;
           // ------------------------------------------------------------

           // Запись (Парсинг)
           stream << "Пакет №" + QString::number(packet+1) + ":" << "\n";
           stream << "-------------------------------------------------------" << "\n";
           // ---------

           if(chanel) stream << QString("Канал: Резервный") << "\n";
           else stream << QString("Канал: Основной") << "\n";

           if(words == 0)  stream << QString("Число слов: 32") << "\n";
           else stream << "Число слов: " + QString::number(words) << "\n" ;
           // ---------


           QString title = "   АДРЕС: ";
           if(adress == 0x1f) title += "Групповая";
           else title += QString::number(adress);

           title += "   НАПРАВЛЕНИЕ: ";

           if(regim) title += "T";
           else      title += "R";


           title += "   ПОДАДРЕС: ";
           title += QString::number(subadress);

           stream << title << "\n";

           title = "Командное слово: " + QString::number(KC[0],16).rightJustified(2,'0').toUpper()
                                               + QString::number(KC[1],16).rightJustified(2,'0').toUpper() + "  ";
           stream << title << "\n";

           //-----
           QString ks_data=" ";
           if(data_KC.count() > 0) {
             for (int j1 = 0;j1< ks_counter-2;j1++)
             {
                 ks_data += QString::number((quint8)data_KC.at(j1),16).toUpper().rightJustified(2,'0');
                 if((j1+1)%2 == 0) ks_data += " ";
                 if((j1+1)%32 == 0 && (j1+1)%64 != 0) ks_data +="\n ";
             }
           //-----
             stream << QString(ks_data);
           }
           stream << "\n";

           data_KC = "";
           //-----
           stream << "Ответное слово: " + QString::number(OC[0],16).rightJustified(2,'0').toUpper()
                                        + QString::number(OC[1],16).rightJustified(2,'0').toUpper()
                                        << "\n";

           ks_data=" ";
           if(os_counter>2)
           {
               for (int j1 = 0;j1<os_counter-2;j1++)
               {
                     ks_data += QString::number((quint8)data_OC.at(j1),16).toUpper().rightJustified(2,'0');
                     if((j1+1)%2 == 0) ks_data += " ";
                     if((j1+1)%32 == 0 && (j1+1)%64 != 0) ks_data +="\n ";
               }
               stream << ks_data << "\n";
           }


           //-----
           if(error)
               stream << QString("Ошибка: " + QString::number(error)) << "\n";
           else
               stream << QString("Ошибка: Нет") << "\n";
           //-----

           stream << "Время: " + time_str << "\n";
           //-----


           double interval = basetime - time_prev;
           QString dimension;

         if(interval > 0) {
           if(interval > 1000000) // Больше секунды
           {
              dimension = " c";
              int s =  (basetime - time_prev)/1000000;
              int ms =  (basetime - time_prev)/1000 - s*1000;
              QString intervalSTR = QString::number(s) + "," + QString::number(ms).rightJustified(3,'0');
              if(packet>0) stream << "Интервал: " + intervalSTR + dimension << "\n";
           }
           else
           {

             if(interval<1000) { dimension = " мкс";
               if(packet>0) stream << "Интервал: " + QString::number(interval) + dimension << "\n";
             }
             else {
               QString dimension = " мс";
               int ms = (basetime - time_prev)/1000;
               int mks = (basetime - time_prev) - (ms*1000);
               QString intervalSTR = QString::number(ms) + "," + QString::number(mks).rightJustified(3,'0');
               if(packet>0) stream << "Интервал: " + intervalSTR + dimension << "\n";
             }
           }
         } //end interval > 0

           //-----
           time_prev = basetime;
           //-----
           stream << "\n" << "\n";
           //-------------------------------------------------------

           packet++;

       } // end A,B
       else {
       if((mainBuffer[i] == 'C') || (mainBuffer[i]=='D'))
           {
               if (mainBuffer.length() > 2) {
               pars.append(mainBuffer.mid(0,2)); // длинна пакета C
               mainBuffer.remove(0,2);
               QString data =  pars.at(1) ? "Включен основной канал" : "Включен резервный канал";

               stream.setDevice(filemap[mainBuffer[i]]);
               stream  << data << "\r\n";              
              }
           }
           else
           {
             if (mainBuffer.length() >0)
             {
               QString rem ="tmi REMOVE 0x" + QString::number(mainBuffer[0],16).rightJustified(2,'0') + "\n";
               QFile file(full_path + QDir::separator() + "err_ti.txt");
               file.open(QIODevice::Append);
               file.write(rem.toStdString().c_str(),rem.length());
               file.close();
               qDebug() << rem;
               mainBuffer.remove(0,1);
             }
           } // end default
       } //end C,D

       pars.clear();
     } // end while
}

// ----------------------------------------------

QString Tmi::timeState(uint sec) //Вывод 3 знака после запятой
{
    uint htime = sec/3600;
    uint mtime = (sec-htime*3600)/60 ;
    uint stime = sec-htime*3600-mtime*60 ;

    QString h = QString::number(htime).rightJustified(2,'0');
    QString m = QString::number(mtime).rightJustified(2,'0');
    QString s = QString::number(stime).rightJustified(2,'0');


    QString  str_time = h + ":" +
            m +":" +
            s;

    return str_time;
}

// ----------------------------------------------

QString Tmi::timeState(uint sec, uint msec) //Вы версия сссс.мс
{
    QString s = QString::number(sec).rightJustified(4,'0');
    QString ms = QString::number(msec).rightJustified(3,'0',true);


    QString str_time = s +":" +
            ms;

    return  str_time;
}

// ----------------------------------------------

void Tmi::resetFileMaps()
{
    filemap.clear();
    filenamemap.clear();
    counterTimeMap.clear();
    genName.clear();

    filemap.insert(marker_A,new QFile(this));
    filemap.insert(marker_B,new QFile(this));
    filemap.insert(marker_C,new QFile(this));
    filemap.insert(marker_D,new QFile(this));

    filenamemap.insert(marker_A,"c1");
    filenamemap.insert(marker_B,"c2");
    filenamemap.insert(marker_C,"c1");
    filenamemap.insert(marker_D,"c2");

    counterTimeMap.insert(marker_A,1);
    counterTimeMap.insert(marker_B,1);
    counterTimeMap.insert(marker_C,1);
    counterTimeMap.insert(marker_D,1);
}
