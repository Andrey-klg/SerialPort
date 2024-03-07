#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    runStatus = false;

    monLocalPath = "";
    delFile = true;

    ui->textEdit->setEnabled(false);
    ui->startParse->setEnabled(false);

    ui->startWriteButton->setEnabled(false);
    ui->connectButton->setEnabled(false);
    ui->comboBox->setEnabled(false);

    QStringList bauds;
    bauds << "110" << "150" << "300" << "1200" << "2400" << "4800" << "9600" << "19200" << "38400" << "57600"
          << "115200" << "230400" << "460800" << "921600";

    ui->comboBox->addItems(bauds);
    ui->comboBox->setCurrentIndex(12);

    this->move((QApplication::desktop()->screenGeometry().width()/2)-this->width()/2,(QApplication::desktop()->screenGeometry().height()/2)-this->height()/2-15);
    ui->progressBar->setMaximum(100);


    portThread = new TmiThread(this);
    connect(portThread,SIGNAL(globalError()),this,SLOT(globalErrorExecute()));

    tmi = new Tmi(this);
    connect(tmi,SIGNAL(finishedRecord(QString)),this,SLOT(finishParseSlot(QString)));
    connect(tmi,SIGNAL(progress(int)),ui->progressBar,SLOT(setValue(int)));
    this->setWindowTitle("Монитор порта");

    QTimer::singleShot(100,this,SLOT(on_refreshButton_clicked()));
}

// ----------------------------------------------

MainWindow::~MainWindow()
{
    tmi->deleteLater();

    delete ui;
}

// ----------------------------------------------

void MainWindow::on_chooseFile_clicked()
{
    // Выбор лок. файла для разбора
    QFileDialog *dialog = new QFileDialog(this);
    dialog->setOption(QFileDialog::ReadOnly);

    QPalette palette;
    palette.setColor(QPalette::Window,QColor(200,200,209));
    dialog->setPalette(palette);

    QString filePath = dialog->getOpenFileName(this, "Выбрать файл монитора для разбора",qApp->applicationDirPath());

    if(!filePath.isEmpty())
    {
       ui->startParse->setEnabled(true);
       ui->monfile->setText(QFileInfo(filePath).fileName());
       tmi->setParceFile(filePath);
       setDefault();
    }
    else
    {
        ui->monfile->setText("Файл монитора");
        ui->startParse->setEnabled(false);
    }
}

// ----------------------------------------------

void MainWindow::setStatus(bool status)
{
   if(status) ui->statusLabel->setPixmap(QPixmap(":/res/good.png"));
   else       ui->statusLabel->setPixmap(QPixmap(":/res/error24.png"));
}

// ----------------------------------------------

void MainWindow::finishParseSlot(QString folderPath)
{
    QMessageBox *box = new QMessageBox(this);
    box->setWindowTitle("Разбор завершен");


    QDir dir;
    dir.setPath(folderPath);
    qDebug() << QFileInfo(folderPath).fileName();
    dir.setSorting(QDir::Time);

    QStringList files = dir.entryList(QDir::NoDotAndDotDot | QDir::Files);
    QString filter = QFileInfo(folderPath).fileName();


    files = sortCat(files,filter);

    QString text = "";
    for(int i=0; i<files.count();i++)
    {
        if(QFileInfo(files.at(i)).fileName().contains(filter))
           text+= QFileInfo(files.at(i)).fileName() + "<br>";
    }

    if(!text.isEmpty())
    {
        if(files.count()>1) box->setText("Сформированы файлы <br>" + text);
        else box->setText("Сформирован файл <br>" + text);

        for(int i=0; i<files.count();i++)
        {
            QFile file(folderPath + QDir::separator() + files.at(i));

            if(file.open(QFile::ReadOnly))
            {
                if(QFileInfo(files.at(i)).fileName().contains(filter))
                {
                    ui->textEdit->setEnabled(true);
                    ui->textEdit->appendHtml("<center>Разбор файла <b>" + QFileInfo(files.at(i)).fileName() + "</b><br></center>");

                    #if QT_VERSION < QT_VERSION_CHECK(5,0,0)
                       ui->textEdit->appendPlainText(file.readAll());
                    #else
                       ui->textEdit->appendPlainText(file.readAll().toStdString().c_str());
                    #endif
                }
            }
        }
    }
    else
       box->setText("Файлы монитора не сформированы!");

    this->setCursor(Qt::ArrowCursor);
    ui->connectFrame->setEnabled(true);
    ui->workFrame->setEnabled(true);
    ui->frame_3->setEnabled(true);


    if(delFile) QFile::remove(tmi->getFile());

    box->exec();
    delete box;
}

// ----------------------------------------------

void MainWindow::on_startParse_clicked() // Разбор файла монитора
{
    delFile = false;
    startParcing();
}

// ----------------------------------------------

void MainWindow::startParcing()
{
    this->setCursor(Qt::WaitCursor);
    ui->connectFrame->setEnabled(false);
    ui->workFrame->setEnabled(false);
    ui->frame_3->setEnabled(false);

    setDefault();
    tmi->setCropFlag(ui->crorBox->isChecked());

    if(ui->fileNameEdit->text() != "mon_ддммгг_ччммсс") tmi->setGenName(ui->fileNameEdit->text());

    tmi->start(QThread::NormalPriority);
}

// ----------------------------------------------

void MainWindow::setDefault()
{
    ui->textEdit->clear();
    ui->progressBar->setValue(0);
    tmi->resetFileMaps();
}

// ----------------------------------------------

void MainWindow::on_startWriteButton_clicked()
{
    if(!runStatus) // Включаем
    {
        setDefault();

        ui->connectFrame->setEnabled(false);
        runStatus = true;
        tmi->setFile(); // f1.txt
        portThread->setRecordFile(tmi->getFile());
        portThread->start();

        connect(portThread,SIGNAL(changeIndicate(bool)),this,SLOT(setStatus(bool)));

        ui->startWriteButton->setText("СТОП");
        ui->startWriteButton->setStyleSheet("color:red");
    }
    else
    {
        setStatus(false);
        runStatus = false;

        portThread->stop(true);
        disconnect(portThread,SIGNAL(changeIndicate(bool)),this,SLOT(setStatus(bool)));

        ui->startWriteButton->setText("Старт");
        ui->startWriteButton->setStyleSheet("color:black");


        QFile file(tmi->getFile());
        if(file.size())
        {
           delFile = true;
           startParcing();
        } else
        {
            QMessageBox *msg = new QMessageBox;
            msg->setText("Ошибка! Входные данные отсутствуют");
            msg->setWindowTitle("Ошибка!");
            msg->addButton(QMessageBox::Cancel);
            msg->setButtonText(QMessageBox::Cancel,"Закрыть");
            msg->exec();

            ui->connectFrame->setEnabled(false);
        }

    }
}

// ----------------------------------------------

void MainWindow::on_refreshButton_clicked()
{
   ui->portBox->clear();
   ui->refreshButton->setEnabled(false);

   QStringList portList = portThread->getPortList();
   ui->portBox->addItems(portList);

   if(portList.count())
   {
       ui->comboBox->setEnabled(true);
       ui->connectButton->setEnabled(true);
   }

   QTimer::singleShot(500,this,SLOT(activateGUI()));
}

// ----------------------------------------------

void MainWindow::globalErrorExecute()  // Произошел отвал порта
{
    if(runStatus)
    {
        on_startWriteButton_clicked();
    }

    portThread->createConnections(false);
    connectionState(false);
}

// ----------------------------------------------

void MainWindow::activateGUI()
{
    ui->refreshButton->setEnabled(true);
}

// ----------------------------------------------

void MainWindow::on_connectButton_clicked(bool checked)
{
    bool ok = false;

    if(checked) // Подключаемся
    {
        if(!ui->portBox->currentText().isEmpty())
        {
            ok = portThread->init(ui->portBox->currentText(),tmi->getFile(),ui->comboBox->currentText().toUInt());
        }

        portThread->createConnections(true);
        connectionState(ok & checked);
    }
    else // Отключаемся
    {
        portThread->closeConnection();
        connectionState(false);
    }
}

// ----------------------------------------------

void MainWindow::connectionState(bool flag) // Состояния ГУИ при коннекте
{
    ui->connectButton->setChecked(flag);

    ui->comboBox->setEnabled(!flag);
    ui->refreshButton->setEnabled(!flag);
    ui->portBox->setEnabled(!flag);
    ui->startWriteButton->setEnabled(flag);

    if(flag) {
       ui->connectButton->setText("Подключено");
       ui->connectButton->setStyleSheet("color:green");
    } else {
        ui->connectButton->setText("Подключить");
        ui->connectButton->setStyleSheet("color:black");
    }

   // on_refreshButton_clicked();
}

// ----------------------------------------------

QStringList MainWindow::sortCat(QStringList list,QString filter)
{
    QStringList sortList;
    int i=0;

    for(int i=0;i<list.count();i++) sortList << "";

    for(int i=0;i<list.count();i++) {
        if(QFileInfo(list.at(i)).fileName().contains(filter))
        {
            if(ui->crorBox->isChecked())
            {
                if(QFileInfo(list.at(i)).fileName().contains(filter + "-"))
                {
                    QString name = QFileInfo(list.at(i)).baseName();
                    name = name.mid(name.indexOf("-") + 1);

                    int idx = name.toInt();
                    sortList[idx-1] = QFileInfo(list.at(i)).fileName();
                }
                else
                    sortList[0] = QFileInfo(list.at(i)).fileName();
            }
            else
               sortList[i] = QFileInfo(list.at(i)).fileName();;
        }
    }

    return sortList;
}

// ----------------------------------------------
