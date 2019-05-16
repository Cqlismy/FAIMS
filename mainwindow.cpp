#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScrollBar>
#include <QMessageBox>
#include <iostream>
#include <QtGui>
#include <QtWidgets>
#include <QTime>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_status(new QLabel),
    m_settings(new SettingDialog),
    m_serial(new QSerialPort(this))
{
    m_ui->setupUi(this);
    setWindowTitle(QStringLiteral("FAIMS SerialPort"));
    showMaximized();
    setWindowIcon(QIcon(":/images/mainwindow.png"));

    //init chart begin
    m_chart = new QChart;
    m_chartview = new QChartView(m_chart);
    m_chartview->setRenderHints(QPainter::Antialiasing);
    m_series = new QLineSeries;
    m_chart->addSeries(m_series);

//    QPen pen(Qt::blue);
//    pen.setWidth(3);
//    m_series->setPen(pen);

    axisX = new QValueAxis;
    axisX->setRange(-13.5, 13.5);
    axisX->setLabelFormat("%g");
    axisX->setMinorTickCount(5);
    axisX->setTitleText(QStringLiteral("Voltage(V)"));

    axisY = new QValueAxis;
    axisY->setRange(-2, 10);
    axisY->setLabelFormat("%g");
    axisY->setMinorTickCount(5);
    axisY->setTitleText(QStringLiteral("Current(pA)"));

    m_ui->ymaxlineEdit->setText(QString::number(axisY->max()));
    m_ui->yminlineEdit->setText(QString::number(axisY->min()));
    m_ui->xmaxlineEdit->setText(QString::number(axisX->max()));
    m_ui->xminlineEdit->setText(QString::number(axisX->min()));

    m_chart->setAxisX(axisX, m_series);
    m_chart->setAxisY(axisY, m_series);
    m_chart->legend()->hide();
    m_chart->setTitle(QStringLiteral("FAIMS Spectrum"));
    m_ui->chartLayout->addWidget(m_chartview);

    connect(m_ui->ymaxlineEdit, &QLineEdit::editingFinished, this, &MainWindow::updateYmax);
    connect(m_ui->yminlineEdit, &QLineEdit::editingFinished, this, &MainWindow::updateYmin);
    connect(m_ui->xmaxlineEdit, &QLineEdit::editingFinished, this, &MainWindow::updateXmax);
    connect(m_ui->xminlineEdit, &QLineEdit::editingFinished, this, &MainWindow::updateXmin);
    //init chart end
    connect(m_ui->svaluelineEdit, &QLineEdit::editingFinished, this, &MainWindow::updateSvalue);

    initWidget();
    initActiosConnect();

    m_ui->startpushButton->setEnabled(false);

    //connect(m_serial, &QSerialPort::error, this, &MainWindow::handleError);
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(m_ui->startpushButton, &QPushButton::clicked, this, &MainWindow::startButton);
    connect(m_ui->clearpushButton, &QPushButton::clicked, this, &MainWindow::clearButton);

    currentymax = axisY->min();
    m_ui->datareceivetextEdit->setProperty("noinput", true);
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::initWidget(void)
{
    m_ui->actionConnect->setIcon(QIcon(":/images/connect.png"));
    m_ui->actionConnect->setStatusTip(QStringLiteral("Connect to the serial port of the Faims system MCU."));
    m_ui->actionDisconnect->setIcon(QIcon(":/images/disconnect.png"));
    m_ui->actionDisconnect->setStatusTip(QStringLiteral("Disconnect to the serial port of the Faims system MCU."));
    m_ui->actionConfigure->setIcon(QIcon(":/images/settings.png"));
    m_ui->actionConfigure->setStatusTip(QStringLiteral("Faims system serial port configuration."));
    m_ui->actionQuit->setIcon(QIcon(":/images/application-exit.png"));
    m_ui->actionQuit->setStatusTip(QStringLiteral("Exit the program."));
    m_ui->actionClear->setIcon(QIcon(":/images/clear.png"));
    m_ui->actionClear->setStatusTip(QStringLiteral("Clear datas and waveform"));
    m_ui->actionOpen_file->setIcon(QIcon(":/images/fileopen.png"));
    m_ui->actionOpen_file->setStatusTip(QStringLiteral("Open a data file."));
    m_ui->actionSave_file->setIcon(QIcon(":/images/filesave.png"));
    m_ui->actionSave_file->setStatusTip(QStringLiteral("Save the collected datas to a file."));
    m_ui->actionSave_chart->setIcon(QIcon(":/images/savechart.ico"));
    m_ui->actionSave_chart->setStatusTip(QStringLiteral("Save the image file of the displayed waveform."));

    m_ui->mainToolBar->addAction(m_ui->actionOpen_file);
    m_ui->mainToolBar->addAction(m_ui->actionSave_file);
    m_ui->mainToolBar->addAction(m_ui->actionSave_chart);
    m_ui->mainToolBar->addAction(m_ui->actionConnect);
    m_ui->mainToolBar->addAction(m_ui->actionDisconnect);
    m_ui->mainToolBar->addAction(m_ui->actionConfigure);
    m_ui->mainToolBar->addAction(m_ui->actionQuit);

    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionConfigure->setEnabled(true);
}

void MainWindow::initActiosConnect(void)
{
    connect(m_ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(m_ui->actionConfigure, &QAction::triggered, m_settings, &SettingDialog::show);
    connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(m_ui->actionAbout_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(m_ui->actionOpen_file, &QAction::triggered, this, &MainWindow::opendatafile);
    connect(m_ui->actionSave_file, &QAction::triggered, this, &MainWindow::savedatafile);
    connect(m_ui->actionSave_chart, &QAction::triggered, this, &MainWindow::savewaveform);
    connect(m_ui->actionClear, &QAction::triggered, this, &MainWindow::clearButton);
    connect(m_ui->actionQuit, &QAction::triggered, this, &MainWindow::close);

}

void MainWindow::showStatusMessage(const QString &message)
{
    m_status->setText(message);
}

void MainWindow::smartInserText(const QString &str)
{
    QTextCursor preCursor(m_ui->datareceivetextEdit->textCursor());

    bool scrollbar;
    QScrollBar *bar = m_ui->datareceivetextEdit->verticalScrollBar();
    int barValue = bar->value();
    if (barValue == bar->maximum()) {
        scrollbar = true;
    } else {
        scrollbar = false;
    }

    int size(m_ui->datareceivetextEdit->toPlainText().size() + str.size());
    if (size > 1000) {
        QTextCursor cursor(m_ui->datareceivetextEdit->textCursor());

        cursor.movePosition(QTextCursor::Start);
        cursor.setPosition(size - 1000, QTextCursor::KeepAnchor);
    }
    m_ui->datareceivetextEdit->moveCursor(QTextCursor::End);
    m_ui->datareceivetextEdit->insertPlainText(str);
    m_ui->datareceivetextEdit->setTextCursor(preCursor);

    if (scrollbar) {
        bar->setValue(bar->maximum());
    } else {
        bar->setValue(barValue);
    }
}

void MainWindow::strtovoltageandcurrent(const QString &str, QPointF &point)
{
    QString temp_str = str;
    quint8 str_size = temp_str.size();
    temp_str.remove((str_size - 2), 2);
    quint8 space_pos = temp_str.indexOf("  ");
    QString data_str;
    data_str = temp_str.left(space_pos);
    data_str.remove(0, 1);
    point.setX(data_str.toFloat());
    data_str.clear();
    data_str = temp_str.mid(space_pos + 2);
    data_str.remove(0, 1);
    point.setY(data_str.toFloat());
}

void MainWindow::Yvalueback(void)
{
    axisY->setRange(-2, 10);
    m_ui->ymaxlineEdit->setText(QString::number(axisY->max()));
    m_ui->yminlineEdit->setText(QString::number(axisY->min()));
    currentymax = axisY->min();
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
        m_ui->startpushButton->setEnabled(true);

        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    m_ui->startpushButton->setEnabled(true);
    m_ui->clearpushButton->setEnabled(true);
    } else {
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());
        showStatusMessage(QStringLiteral("Open error"));
    }

}

void MainWindow::closeSerialPort(void)
{
    if (m_serial->isOpen()) {
        m_serial->close();
    }
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionConfigure->setEnabled(true);
    m_ui->startpushButton->setEnabled(false);
    showStatusMessage(QStringLiteral("Disconnected"));
}

void MainWindow::about(void)
{
    QMessageBox::about(this, tr("About Famis SerialPort"),
                             tr("Faims SerialPort uses the "
                                "serial port to receive the "
                                "voltage and current data "
                                "collected by the MCU, and "
                                "uses the QChart class to "
                                "scan the voltage and current spectrum."));
}

void MainWindow::writeData(const QByteArray &data)
{
    m_serial->write(data);
}

void MainWindow::readData(void)
{
    static QString string;
    static QList<QPointF> points;
    string += m_serial->readAll();
    qDebug() << "string:" << string;
    while (string.count("\r\n") >=2) {
        int pos = string.indexOf("\r\n");
        QString temp_str = string.left(pos + 2);

        if (temp_str.contains("V") &&
            temp_str.contains("C") &&
            temp_str.contains("  ")) {

            QPointF point;
            strtovoltageandcurrent(temp_str, point);
            if (point.y() > currentymax) {
                currentymax = point.y();
                if ((axisY->max() - currentymax) < 5) {
                    axisY->setMax(currentymax + 20);
                    m_ui->ymaxlineEdit->setText(QString::number(currentymax + 20));
                }
            }
            points << point;
        }
        smartInserText(temp_str);
        string.remove(temp_str);
    }
    m_series->append(points);
    points.clear();
    if (string == "InputConfig:\r\n") {
        smartInserText(QStringLiteral("InputConfig:\r\n"));
    }
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::startButton(void)
{
    QString command;
    if (m_ui->mode1checkBox->isChecked()) {
        command.append("A");
    } else if (m_ui->mode2checkBox->isChecked()) {
        command.append("D");
    } else if (m_ui->mode3checkBox->isChecked()) {
        command.append("K");
    } else {
        command.append("S0000C0000T0000");
        writeData(&command.toStdString()[0]);
        return;
    }
    QString S,C,T;
    QString S_Value;
    S_Value = m_ui->svaluelineEdit->text();
    qreal x = S_Value.toDouble();
    axisX->setRange((x - 2*x) - 0.5, x + 0.5);
    m_ui->xminlineEdit->setText(QString::number(axisX->min()));
    m_ui->xmaxlineEdit->setText(QString::number(axisX->max()));
    qreal y = (-2046/13.0 *x) + 2046;
    quint16 y_ = quint16(y);
    S = QString::number(y_);
    while (S.size() < 4) {
        S.insert(0, "0");
    }
    C = m_ui->cvaluelineEdit->text();
    while (C.size() < 4) {
        C.insert(0, "0");
    }
    T= m_ui->tvaluelineEdit->text();
    while (T.size() < 4) {
        T.insert(0, "0");
    }
    command.append("S").append(S);
    command.append("C").append(C);
    command.append("T").append(T);
    writeData(&command.toStdString()[0]);

    m_ui->datareceivetextEdit->clear();
}

void MainWindow::clearButton(void)
{
    quint32 cnt = m_series->count();
    qDebug() << "Point:" << cnt;
    m_series->clear();
    m_ui->datareceivetextEdit->clear();
    Yvalueback();
    if (m_serial->isOpen()) {
        smartInserText(QStringLiteral("InputConfig:\r\n"));
    }
}

void MainWindow::updateYmax(void)
{
    bool ok;
    qreal value;
    value = m_ui->ymaxlineEdit->text().toDouble(&ok);
    if (ok) {
        static_cast<QValueAxis*>(m_chart->axisY())->setMax(value);
    } else {
        m_ui->ymaxlineEdit->blockSignals(true);
        QMessageBox::critical(this, tr("Critical Error"), tr("Invalid Value!"));
        m_ui->ymaxlineEdit->setText(QString::number(axisY->max()));
        m_ui->ymaxlineEdit->blockSignals(false);
    }
}

void MainWindow::updateYmin(void)
{
    bool ok;
    qreal value;
    value = m_ui->yminlineEdit->text().toDouble(&ok);
    if (ok) {
        static_cast<QValueAxis*>(m_chart->axisY())->setMin(value);
    } else {
        m_ui->yminlineEdit->blockSignals(true);
        QMessageBox::critical(this, tr("Critical Error"), tr("Invalid Value!"));
        m_ui->yminlineEdit->setText(QString::number(axisY->min()));
        m_ui->yminlineEdit->blockSignals(false);
    }

}

void MainWindow::updateXmax(void)
{
    bool ok;
    qreal value;
    value = m_ui->xmaxlineEdit->text().toDouble(&ok);
    if (ok) {
        static_cast<QValueAxis*>(m_chart->axisX())->setMax(value);
    } else {
        m_ui->xmaxlineEdit->blockSignals(true);
        QMessageBox::critical(this, tr("Critical Error"), tr("Invalid Value!"));
        m_ui->xmaxlineEdit->setText(QString::number(axisX->max()));
        m_ui->xmaxlineEdit->blockSignals(false);
    }
}

void MainWindow::updateXmin(void)
{
    bool ok;
    qreal value;
    value = m_ui->xminlineEdit->text().toDouble(&ok);
    if (ok) {
        static_cast<QValueAxis*>(m_chart->axisX())->setMin(value);
    } else {
        m_ui->xminlineEdit->blockSignals(true);
        QMessageBox::critical(this, tr("Critical Error"), tr("Invalid Value!"));
        m_ui->xminlineEdit->setText(QString::number(axisX->min()));
        m_ui->xminlineEdit->blockSignals(false);
    }
}

void MainWindow::updateSvalue(void)
{
    bool ok;
    qreal value;
    value = m_ui->svaluelineEdit->text().toDouble(&ok);
    if (ok) {
        static_cast<QValueAxis*>(m_chart->axisX())->setMin((value - 2*value) - 0.5);
        static_cast<QValueAxis*>(m_chart->axisX())->setMax(value + 0.5);
    } else {
        m_ui->xminlineEdit->blockSignals(true);
        m_ui->xmaxlineEdit->blockSignals(true);
        QMessageBox::critical(this, tr("Critical Error"), tr("Invalid Value!"));
        m_ui->xminlineEdit->setText(QString::number(axisX->min()));
        m_ui->xmaxlineEdit->setText(QString::number(axisX->max()));
        m_ui->xminlineEdit->blockSignals(false);
        m_ui->xmaxlineEdit->blockSignals(false);
    }
}


//void MainWindow::opendatafile(void)
//{
//    QFileDialog *d = new QFileDialog(this);
//    d->setWindowTitle("Open Data File");
//    d->setDirectory(".");
//    d->setNameFilter("Text Files(*.txt)");
//    connect(d, SIGNAL(fileSelected(QString)), this, SLOT(openfileselect(QString)));
//    d->show();
//}

void MainWindow::opendatafile(void)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Data File"), ".", tr("Text Files(*.txt)"));
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, tr("Open Datas File"), tr("Can not open file:\n%1").arg(fileName));
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
                        m_ui->ymaxlineEdit->setText(QString::number(axisY->max()));
                    }
                }
                m_series->append(point);
            }
            str.clear();
        }
        file.close();
        qreal x = m_series->at(0).x();
        axisX->setRange(x - 1, (x - 2*x) + 1);
        m_ui->xminlineEdit->setText(QString::number(axisX->min()));
        m_ui->xmaxlineEdit->setText(QString::number(axisX->max()));
        QMessageBox::information(this, tr("Open Datas File"), tr("The data file was opened successfully."));
    }
}

void MainWindow::openfileselect(QString fileName)
{

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, tr("Open Datas File"), tr("Can not open file:\n%1").arg(fileName));
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
                        m_ui->ymaxlineEdit->setText(QString::number(axisY->max()));
                    }
                }
                m_series->append(point);
            }
            str.clear();
        }
        file.close();
        qreal x = m_series->at(0).x();
        axisX->setRange(x - 1, (x - 2*x) + 1);
        m_ui->xminlineEdit->setText(QString::number(axisX->min()));
        m_ui->xmaxlineEdit->setText(QString::number(axisX->max()));
        //QMessageBox::information(this, tr("Open Datas File"), tr("The data file was opened successfully."));
    }
}

//void MainWindow::savedatafile(void)
//{
//    QFileDialog *d = new QFileDialog(this);
//    d->setWindowTitle("Save Data File");
//    d->setAcceptMode(QFileDialog::AcceptSave);
//    d->setDirectory(".");
//    d->setNameFilter("Text File(*.txt)");
//    connect(d, SIGNAL(fileSelected(QString)), this, SLOT(datafileselect(QString)));
//    d->show();
//}

void MainWindow::savedatafile(void)
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Data File"), ".", tr("Text File(*.txt)"));
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::warning(this, tr("Save Datas File"), tr("Can not open file:\n%1.").arg(fileName));
            return;
        }
        QTextStream out(&file);
        out << m_ui->datareceivetextEdit->toPlainText();
        file.close();
        QMessageBox::information(this, tr("Save Datas File"), tr("The data file was saved successfully."));
    }
}

void MainWindow::datafileselect(QString fileName)
{
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::warning(this, tr("Save Datas File"), tr("Can not open file:\n%1.").arg(fileName));
            return;
        }
        QTextStream out(&file);
        out << m_ui->datareceivetextEdit->toPlainText();
        file.close();
        //QMessageBox::information(this, tr("Save Datas File"), tr("The data file was saved successfully."));
    }
}

//void MainWindow::savewaveform(void)
//{
//    QFileDialog *d = new QFileDialog(this);
//    d->setWindowTitle("Save Image File");
//    d->setAcceptMode(QFileDialog::AcceptSave);
//    d->setDirectory(".");
//    d->setNameFilter("Image Files(*.png)");
//    connect(d, SIGNAL(fileSelected(QString)), this, SLOT(waveformfileselect(QString)));
//    d->show();
//}

void MainWindow::savewaveform(void)
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image File"), ".", tr("Image File(*.png)"));
    if (!fileName.isEmpty()) {
        qDebug() << "fileName" << fileName;
        QScreen *screen = QGuiApplication::primaryScreen();
        mysleep(2000);
        if (screen) {
            screen->grabWindow(m_ui->centralWidget->winId()).save(fileName);
        }
        QMessageBox::information(this, tr("Save Waveform"), tr("The image file was saved successfully."));
    }
}

void MainWindow::waveformfileselect(QString fileName)
{
    if (!fileName.isEmpty()) {
        qDebug() << "fileName" << fileName;
        QScreen *screen = QGuiApplication::primaryScreen();
        mysleep(2000);
        if (screen) {
            screen->grabWindow(m_ui->centralWidget->winId()).save(fileName);
        }
        //QMessageBox::information(this, tr("Save Waveform"), tr("The image file was saved successfully."));
    }
}

void MainWindow::mysleep(quint32 msec)
{
    QTime reachtime = QTime::currentTime().addMSecs(msec);

    while (QTime::currentTime() < reachtime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}
