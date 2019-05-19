#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingdialog.h"
#include <QMessageBox>
#include <QDebug>
#include <QScrollBar>
#include <QtGui>
#include <QtWidgets>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_status(new QLabel),
    m_settings(new SettingDialog),
    m_serial(new QSerialPort(this))
{
    m_ui->setupUi(this);


    setWindowTitle(tr("FAIMS SerialPort"));
    setWindowIcon(QIcon(":/images/mainwindow.png"));


    showMaximized();
//    setWindowFlags(Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);

    //chart_begin
    m_chart = new QChart;
    m_chartview = new QChartView(m_chart);
    m_chartview->setRenderHint(QPainter::Antialiasing);
    m_series = new QLineSeries;
    m_chart->addSeries(m_series);

    QPen pen(Qt::blue);
    pen.setWidth(3);
    m_series->setPen(pen);

    axisX = new QValueAxis;
    axisX->setRange(-13.5, 13.5);
    axisX->setLabelFormat("%g");
    axisX->setMinorTickCount(6);
    axisX->setTitleText(QStringLiteral("Voltage(V)"));

    axisY = new QValueAxis;
    axisY->setRange(-2, 2);
    axisY->setLabelFormat("%g");
    axisY->setMinorTickCount(6);
    axisY->setTitleText(QStringLiteral("Current(pA)"));

    m_chart->setAxisX(axisX, m_series);
    m_chart->setAxisY(axisY, m_series);
    m_chart->legend()->hide();
    m_chart->setTitle(QStringLiteral("FAIMS Spectrum"));
    m_ui->chartLayout->addWidget(m_chartview);
    //chart_end

    connect(m_ui->svaluelineEdit, &QLineEdit::editingFinished,this, &MainWindow::updateSvalue);

    initWidget();
    initActionsConnect();

    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);

    currentymax = axisY->max();   //获取当前y轴的初始最大值
    currentymin = axisY->min();   //获取当前y轴的初始最小值
    m_ui->receivetextEdit->setProperty("noinput", true);

    //检测输入的数值是否有效
    connect(m_ui->zeroerrorlineEdit, &QLineEdit::editingFinished, this, &MainWindow::zeroerrorvalue);
    connect(m_ui->zerodriftlineEdit, &QLineEdit::editingFinished, this, &MainWindow::zerodriftvalue);
    connect(m_ui->svaluelineEdit, &QLineEdit::editingFinished, this, &MainWindow::svalue);
    connect(m_ui->cvaluelineEdit, &QLineEdit::editingFinished, this, &MainWindow::cvalue);
    connect(m_ui->tvaluelineEdit, &QLineEdit::editingFinished, this, &MainWindow::tvalue);
}

MainWindow::~MainWindow()
{
    delete m_ui;
    delete m_settings;
}

void MainWindow::initWidget(void)
{
    m_ui->actionConnect->setIcon(QIcon(":/images/connect.png"));
    m_ui->actionConnect->setStatusTip(tr("Connect to the serial port of the Faims system MCU."));
    m_ui->actionDisconnect->setIcon(QIcon(":/images/disconnect.png"));
    m_ui->actionDisconnect->setStatusTip(tr("Disconnect to the serial port of the Faims system MCU."));
    m_ui->actionConfigure->setIcon(QIcon(":/images/settings.png"));
    m_ui->actionConfigure->setStatusTip(tr("Faims system serial port configuration."));
    m_ui->actionQuit_2->setIcon(QIcon(":/images/application-exit.png"));
    m_ui->actionQuit_2->setStatusTip(tr("Exit the program."));
    m_ui->actionClear->setIcon(QIcon(":/images/clear.png"));
    m_ui->actionClear->setStatusTip(tr("Clear data and waveform."));
    m_ui->actionOpen_file->setIcon(QIcon(":/images/fileopen.png"));
    m_ui->actionOpen_file->setStatusTip(tr("Open a data file."));
    m_ui->actionSave_Data->setIcon(QIcon(":/images/filesave.png"));
    m_ui->actionSave_Data->setStatusTip(tr("Save the collected data to a file."));
    m_ui->actionSave_Chart->setIcon(QIcon(":/images/savechart.ico"));
    m_ui->actionSave_Chart->setStatusTip(tr("Save the image file of the displayed waveform."));

    m_ui->mainToolBar->addAction(m_ui->actionOpen_file);
    m_ui->mainToolBar->addAction(m_ui->actionSave_Data);
    m_ui->mainToolBar->addAction(m_ui->actionSave_Chart);
    m_ui->mainToolBar->addAction(m_ui->actionConnect);
    m_ui->mainToolBar->addAction(m_ui->actionDisconnect);
    m_ui->mainToolBar->addAction(m_ui->actionConfigure);
    m_ui->mainToolBar->addAction(m_ui->actionQuit_2);


    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionQuit->setEnabled(true);
    m_ui->actionConfigure->setEnabled(true);

    m_ui->statusBar->addWidget(m_status);
}

void MainWindow::openSerialPort(void)
{
    const SettingDialog::Settings p = m_settings->settings();
    m_serial->setPortName(p.name);
    m_serial->setBaudRate(p.baudRate);
    m_serial->setDataBits(p.dataBits);
    m_serial->setParity(p.parity);
    m_serial->setStopBits(p.stopBits);
    m_serial->setFlowControl(p.flowControl);

    if (m_serial->open(QIODevice::ReadWrite)) {
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->actionConfigure->setEnabled(false);

        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));

    } else {
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());
        showStatusMessage(tr("Open error."));
    }

}

void MainWindow::closeSerialPort()
{
    if (m_serial->isOpen()) {
        m_serial->close();
    }
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionConfigure->setEnabled(true);
    showStatusMessage(tr("Disconnected"));
}

void MainWindow::about(void)
{
    QMessageBox::about(this, tr("About FAIMS SerialPort"),
                             tr("FAIMS SerialPort is a QT application "
                                "that communicates with the FAIMS lower "
                                "computer control center through the serial port."
                                "After the serial port is successfully connected,"
                                "Click the Show button to display the current parameters."
                                "Click the Set button to change the parameters."
                                "Auto Zero selects the auto zero function after selection."
                                "Negative Current supports negative current acquisition after selection."
                                "Zero Error is the range of auto-zero error.It can be set to 0.01~2pA."
                                "Zero Drift is the range of zero drift.It can be set to 0~50pA."
                                "FAIMS SerialPort can be configured with FAIMS lower position machine."
                                "Three working modes,in which CV Span is the compensation "
                                "voltage range of FAIMS system,the value can be in "
                                "the range of 0~13.5V,Density is the sampling point "
                                "of FAIMS system,the value can be in the range of 1~4096,"
                                "Step Size is the step size of sampling,the value is The range "
                                "is from 1 to 1000,corresponding to 10~1010ms."));
}

void MainWindow::writeData(const QByteArray &data)
{
    m_serial->write(data);
}

void MainWindow::readData()
{
    static QString string;
    static QList<QPointF> points;
    string += m_serial->readAll();

    while (string.endsWith("\r\n")) {
        int pos = string.indexOf("\r\n");   //找到"\r\n"的位置
        QString temp_string = string.left(pos + 2);   //根据得到的位置取出首个数据包

        //判断是否是补偿电压与电流的数据包，是的话，提取有效值并且将点存入points容器中
        if (temp_string.contains("V") &&
            temp_string.contains("C") &&
            temp_string.contains("  ")) {
            QPointF point;
            strtovoltageandcurrent(temp_string, point); //将数据点进行输出
            if (point.y() > currentymax) {
                currentymax = point.y();
                if ((axisY->max() - currentymax) < 5) {
                    axisY->setMax(currentymax + 20);
                }
            }
            if (point.y() < currentymin) {
                currentymin = point.y();
                if ((currentymin - axisY->min()) < 5) {
                    axisY->setMin(currentymin - 20);
                }
            }
            points << point;
        } else if (temp_string.contains("Input_Sys_config")) {
            //设置调零参数
            QString command;
            if (m_ui->autozerocheckBox->isChecked()) {
                command.append("A0001");   //自动调零开
            } else {
                command.append("A0000");   //自动调零关
            }
            if (m_ui->negativecurrentcheckBox->isChecked()) {
                command.append("N0001");    //负电流采集开
            } else {
                command.append("N0000");    //负电流采集关
            }
            qreal z_e = m_ui->zeroerrorlineEdit->text().toDouble();  //获取调零误差
            z_e = z_e * 100;  //放大100倍，单位为pA
            QString str_ZE = QString::number(z_e);
            while (str_ZE.size() < 4) {
                str_ZE.insert(0, "0");
            }
            command.append("O").append(str_ZE);
            QString str_ZD = m_ui->zerodriftlineEdit->text();   //获取零点漂移
            while (str_ZD.size() < 4) {
                str_ZD.insert(0, "0");
            }
            command.append("Z").append(str_ZD);
            writeData(&command.toStdString()[0]);  //将设置命令发送到串口
        }

        smartInserText(temp_string);
        string.remove(temp_string);   //继续处理下一个"\r\n"结尾的字符串
    }
    m_series->append(points);     //画图
    points.clear();                //清除容器内已存在的点
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
        closeSerialPort();
    }
}


void MainWindow::initActionsConnect()
{
    connect(m_ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(m_ui->actionQuit_2, &QAction::triggered, this, &MainWindow::close);
    connect(m_ui->actionConfigure, &QAction::triggered, m_settings, &SettingDialog::show);
    connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(m_ui->actionAbout_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(m_ui->actionOpen_file, &QAction::triggered, this, &MainWindow::opendatafile);
    connect(m_ui->actionSave_Data, &QAction::triggered, this, &MainWindow::savedatas);
    connect(m_ui->actionSave_Chart, &QAction::triggered, this, &MainWindow::savewaveform);
    connect(m_ui->actionClear, &QAction::triggered, this, &MainWindow::on_clearButton_clicked);

}

void MainWindow::showStatusMessage(const QString &message)
{
    m_status->setText(message);
}

void MainWindow::smartInserText(const QString &str)
{
    QTextCursor preCursor(m_ui->receivetextEdit->textCursor());

    bool scrollbar;
    QScrollBar *bar = m_ui->receivetextEdit->verticalScrollBar();
    int barValue = bar->value();
    if (barValue == bar->maximum()) {
        scrollbar = true;
    } else {
        scrollbar = false;
    }

    int size(m_ui->receivetextEdit->toPlainText().size() + str.size());
    if (size > 1000) {
        QTextCursor cursor(m_ui->receivetextEdit->textCursor());

        cursor.movePosition(QTextCursor::Start);
        cursor.setPosition(size - 1000, QTextCursor::KeepAnchor);
        //cursor.removeSelectedText();
    }
    m_ui->receivetextEdit->moveCursor(QTextCursor::End);
    m_ui->receivetextEdit->insertPlainText(str);
    m_ui->receivetextEdit->setTextCursor(preCursor);

    if (scrollbar) {
        bar->setValue(bar->maximum());
    } else {
        bar->setValue(barValue);
    }
}

void MainWindow::on_startButton_clicked()
{
   QString command;
   if (m_ui->Mode1checkBox->isChecked()) {
       command.append("A");
   } else if (m_ui->Mode2checkBox->isChecked()) {
       command.append("D");
   } else if (m_ui->Mode3checkBox->isChecked()) {
       command.append("K");
   } else {
       command.append("S0000C0000T0000");
       writeData(&command.toStdString()[0]);
       return;
   }
   QString S,S_Value,C,T;
   S_Value = m_ui->svaluelineEdit->text();
   double x = S_Value.toDouble();
   axisX->setRange((x - 2*x) - 0.5, x + 0.5);

   double y = (-2046/13.0*x) + 2046;
   quint16 y_ = quint16(y);
   S = QString::number(y_);
   while (S.size() < 4) {
       S.insert(0, "0");
   }
   C = m_ui->cvaluelineEdit->text();
   while (C.size() < 4) {
       C.insert(0, "0");
   }
   T = m_ui->tvaluelineEdit->text();
   while (T.size() < 4) {
       T.insert(0, "0");
   }
   command.append("S").append(S);
   command.append("C").append(C);
   command.append("T").append(T);
   writeData(&command.toStdString()[0]);

   m_ui->receivetextEdit->clear();

}

void MainWindow::strtovoltageandcurrent(const QString &str, QPointF &point)
{
    QString temp_str = str;
    quint8 str_size = temp_str.size();
    temp_str.remove((str_size - 2), 2); //将数据包的"\r\n"移除
    quint8 space_pos = temp_str.indexOf("  "); //获取"  "的位置
    QString data_str;
    data_str = temp_str.left(space_pos);
    data_str.remove(0, 1);  //移除"V"
    point.setX(data_str.toFloat()); //获得补偿电压的数值
    data_str.clear();
    data_str = temp_str.mid(space_pos + 2);
    data_str.remove(0, 1);  //移除"C"
    point.setY(data_str.toFloat());  //获得微弱电流的数据
}


void MainWindow::updateSvalue(void)
{
    bool ok;
    qreal value;
    value = m_ui->svaluelineEdit->text().toDouble(&ok);
    if (ok) {
        static_cast<QValueAxis*>(m_chart->axisX())->setMax(value + 0.5);
        static_cast<QValueAxis*>(m_chart->axisX())->setMin((value - 2*value) - 0.5);
    } else {
        QMessageBox::critical(this, tr("Critical Error"), tr("Invalid Value!"));
    }
}

void MainWindow::on_clearButton_clicked()
{
    quint32 cnt = m_series->count();

    if (cnt != 0) {
       qDebug() << "Point:" << cnt;
       m_series->clear();   //清除已存在的点
    }
    m_ui->receivetextEdit->clear();
    yvalueback();
    if (m_serial->isOpen()) {
        smartInserText("InputConfig:\r\n");
    }
}

void MainWindow::opendatafile(void)
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open Datas File"), ".", tr("Text Files(*.txt)"));
    if (!path.isEmpty()) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, tr("Open Datas File"), tr("Can not open file:\n%1").arg(path));
            return;
        }
        QTextStream in(&file);
        QPointF point;
        QString str;

        while (!in.atEnd()) {
            str = in.readLine();
            if (!str.endsWith("\r\n")) {
                str.append(QStringLiteral("\r\n"));
            }
            smartInserText(str);
            if ((str.contains("V")) && (str.contains("C"))) {
                strtovoltageandcurrent(str, point);
                if (point.y() > currentymax) {
                    currentymax = point.y();
                    if ((axisY->max() - currentymax) < 5) {
                        axisY->setMax(currentymax + 20);
                    }
                }
                if (point.y() < currentymin) {
                    currentymin = point.y();
                    if ((currentymin - axisY->min()) < 5) {
                        axisY->setMin(currentymin - 20);
                    }
                }
                m_series->append(point);
            }
            str.clear();
        }
        file.close();
        double x = m_series->at(1).x();
        axisX->setRange(x - 1, (x - 2*x) + 1);
        QMessageBox::information(this, tr("Open Datas File"), tr("The data file was opened successfully."));
    }
}

void MainWindow::savedatas(void)
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save Datas File"), ".", tr("Text Files(*.txt)"));
    if (!path.isEmpty()) {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::warning(this, tr("Save Datas File"), tr("Can not open file:\n%1").arg(path));
            return;
        }
        QTextStream out(&file);
        out << m_ui->receivetextEdit->toPlainText();
        file.close();
        QMessageBox::information(this, tr("Save Datas File"), tr("The data file was saved successfully."));
    }
}

void MainWindow::savewaveform(void)
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save Chart"), "." ,
                                                tr("PNG Files(*.png)"));
    if (!path.isEmpty()) {
        QScreen *screen = QGuiApplication::primaryScreen();
        if (screen) {
            screen->grabWindow(m_ui->centralWidget->winId()).save(path);
        }
        QMessageBox::information(this, tr("Save Chart"), tr("The image file was saved successfully."));
    }
}

void MainWindow::yvalueback(void)
{
    axisY->setRange(-2, 2);
    currentymax = axisY->max();
    currentymin = axisY->min();
}

void MainWindow::on_showsystemButton_clicked()
{
    QByteArray str = "P";
    writeData(str);
}

void MainWindow::on_setsystemButton_clicked()
{
    QByteArray str = "F";
    writeData(str);
}

void MainWindow::zeroerrorvalue(void)
{
   bool ok;
   qreal value = m_ui->zeroerrorlineEdit->text().toDouble(&ok);
   if (ok) {
        if ((value < 0.01) || (value > 2.0)) {
            QMessageBox::warning(this, tr("Critical Error"), tr("Invalid Value!\n"
                                                            "The range you enter should be 0.01~2pA,"
                                                            "please re-enter."));
       }
   }
}

void MainWindow::zerodriftvalue(void)
{
    bool ok;
    qreal value = m_ui->zerodriftlineEdit->text().toDouble(&ok);
    if (ok) {
        if ((value < 0) || (value > 50)) {
            QMessageBox::warning(this, tr("Critical Error"), tr("Invalid Value!\n"
                                                                "The range you enter should be 0~50pA,"
                                                                "please re-enter."));
        }
    }
}

void MainWindow::svalue(void)
{
    bool ok;
    qreal value = m_ui->svaluelineEdit->text().toDouble(&ok);
    if (ok) {
        if ((value < 0) || (value) > 13.5) {
            QMessageBox::warning(this, tr("Critical Error"), tr("Invalid Value!\n"
                                                                "The range you enter should be 0~13.5V,"
                                                                "please re-enter."));
        }
    }
}

void MainWindow::cvalue(void)
{
    bool ok;
    int value = m_ui->cvaluelineEdit->text().toInt(&ok);
    if (ok) {
        if ((value < 1) || (value) > 4096) {
            QMessageBox::warning(this, tr("Critical Error"), tr("Invalid Value!\n"
                                                                "The range you enter should be 1~4096 point,"
                                                                "please re-enter."));
        }
    }
}

void MainWindow::tvalue(void)
{
    bool ok;
    int value = m_ui->tvaluelineEdit->text().toInt(&ok);
    if (ok) {
        if ((value < 1) || (value > 1000)) {
            QMessageBox::warning(this, tr("Critical Error"), tr("Invalid Value!\n"
                                                                "The range you enter should be 1~1000,"
                                                                "please re-enter."));
        }
    }
}
