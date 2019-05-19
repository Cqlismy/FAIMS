#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QLabel>
#include "settingdialog.h"
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>


QT_BEGIN_NAMESPACE

class QLabel;


namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

using namespace QtCharts;

class SettingsDialog;

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
    void on_startButton_clicked();
    void updateSvalue(void);
    void on_clearButton_clicked();
    void opendatafile(void);
    void savedatas(void);
    void savewaveform(void);
    void on_showsystemButton_clicked();
    void on_setsystemButton_clicked();
    void zeroerrorvalue(void);
    void zerodriftvalue(void);
    void svalue(void);
    void cvalue(void);
    void tvalue(void);

private:
    void initWidget(void);
    void initActionsConnect(void);
    void showStatusMessage(const QString &message);
    void smartInserText(const QString &str);
    void strtovoltageandcurrent(const QString &str, QPointF &point);
    void yvalueback(void);

private:
    Ui::MainWindow *m_ui = nullptr;

    QLabel *m_status = nullptr;
    SettingDialog *m_settings = nullptr;
    QSerialPort *m_serial = nullptr;

    //chart
    QLineSeries *m_series;
    QChart *m_chart;
    QChartView *m_chartview;
    QValueAxis *axisX;
    QValueAxis *axisY;

    float currentymax;
    float currentymin;

};

#endif // MAINWINDOW_H
