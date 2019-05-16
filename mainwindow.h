#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QLabel>
#include "settingdialog.h"
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QPointF>

QT_BEGIN_NAMESPACE
class QLabel;

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

using namespace QtCharts;

class SettingDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void openSerialPort(void);
    void closeSerialPort(void);
    void about(void);
    void writeData(const QByteArray &data);
    void readData(void);
    void handleError(QSerialPort::SerialPortError error);
    void startButton(void);
    void clearButton(void);
    void updateYmax(void);
    void updateYmin(void);
    void updateXmax(void);
    void updateXmin(void);
    void updateSvalue(void);
    void opendatafile(void);
    void openfileselect(QString fileName);
    void savedatafile(void);
    void datafileselect(QString fileName);
    void savewaveform(void);
    void waveformfileselect(QString fileName);

private:
    void initWidget(void);
    void initActiosConnect(void);
    void showStatusMessage(const QString &message);
    void smartInserText(const QString &str);
    void strtovoltageandcurrent(const QString &str, QPointF &point);
    void Yvalueback(void);
    void mysleep(quint32 msec);
private:
    Ui::MainWindow *m_ui;

    QLabel *m_status;
    SettingDialog *m_settings;
    QSerialPort *m_serial;

    //charts
    QLineSeries *m_series;
    QChart *m_chart;
    QChartView *m_chartview;
    QValueAxis *axisX;
    QValueAxis *axisY;
    float currentymax;


};

#endif // MAINWINDOW_H
