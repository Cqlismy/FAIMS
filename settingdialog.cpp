#include "settingdialog.h"
#include "ui_settingdialog.h"

#include <QLineEdit>
#include <QIntValidator>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

SettingDialog::SettingDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::SettingDialog),
    m_intValidator(new QIntValidator(0, 4000000, this))
{
    m_ui->setupUi(this);

    setWindowIcon(QIcon(":/images/settings"));
    setWindowTitle(tr("Configure"));

    m_ui->baudrateBox->setInsertPolicy(QComboBox::NoInsert);

    connect(m_ui->applyButton, &QPushButton::clicked,
                     this, &SettingDialog::apply);

    connect(m_ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                     this, &SettingDialog::showPortInfo);

    connect(m_ui->baudrateBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                     this, &SettingDialog::checkCustomBaudRatePolicy);

    connect(m_ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                     this, &SettingDialog::checkCustomDevicePathPolicy);

    fillPortsParameters();
    fillPortsInfo();

    updateSettings();

}

SettingDialog::~SettingDialog()
{
    delete m_ui;
}

SettingDialog::Settings SettingDialog::settings() const
{
    return m_currentSettings;
}

void SettingDialog::showPortInfo(int idx)
{
    if (idx == -1) {
        return;
    }
    const QStringList list = m_ui->serialPortInfoListBox->itemData(idx).toStringList();
    m_ui->descrptionlabel->setText(tr("Description: %1").
                                   arg(list.count() > 1 ? list.at(1) : tr(blankString)));
    m_ui->manufacturerlabel->setText(tr("Manufacturer: %1").
                                   arg(list.count() > 2 ? list.at(2) : tr(blankString)));
    m_ui->serialnumberlabel->setText(tr("Serial number: %1").
                                   arg(list.count() > 3 ? list.at(3) : tr(blankString)));
    m_ui->locationlabel->setText(tr("Location: %1").
                                   arg(list.count() > 4 ? list.at(4) : tr(blankString)));
    m_ui->vidlabel->setText(tr("Vendor Identifier: %1").
                                   arg(list.count() > 5 ? list.at(5) : tr(blankString)));
    m_ui->pidlabel->setText(tr("Product Identifier: %1").
                                   arg(list.count() > 6 ? list.at(6) : tr(blankString)));
}

void SettingDialog::apply(void)
{
    updateSettings();
    QWidget::hide();
}

void SettingDialog::checkCustomBaudRatePolicy(int idx)
{
    const bool isCustomBaudRate = !m_ui->baudrateBox->itemData(idx).isValid();
    m_ui->baudrateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        m_ui->baudrateBox->clearEditText();
        QLineEdit *edit = m_ui->baudrateBox->lineEdit();
        edit->setValidator(m_intValidator);
    }
}


void SettingDialog::checkCustomDevicePathPolicy(int idx)
{
    const bool isCustomPath = !m_ui->serialPortInfoListBox->itemData(idx).isValid();
    m_ui->serialPortInfoListBox->setEditable(isCustomPath);
    if (isCustomPath) {
        m_ui->serialPortInfoListBox->clearEditText();
    }
}

void SettingDialog::fillPortsInfo()
{
    m_ui->serialPortInfoListBox->clear();
    QString description;
    QString manufacturer;
    QString serialnumber;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialnumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialnumber.isEmpty() ? serialnumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);
        m_ui->serialPortInfoListBox->addItem(list.first(), list);
    }
    m_ui->serialPortInfoListBox->addItem(tr("Custom"));

}

void SettingDialog::fillPortsParameters()
{
    m_ui->baudrateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    m_ui->baudrateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    m_ui->baudrateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    m_ui->baudrateBox->addItem(QStringLiteral("57600"), QSerialPort::Baud57600);
    m_ui->baudrateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    m_ui->baudrateBox->addItem(tr("Custom"));

    m_ui->databitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    m_ui->databitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    m_ui->databitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    m_ui->databitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
    m_ui->databitsBox->setCurrentIndex(3);

    m_ui->parityBox->addItem(tr("None"), QSerialPort::NoParity);
    m_ui->parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    m_ui->parityBox->addItem(tr("Odd"), QSerialPort::EvenParity);
    m_ui->parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    m_ui->parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);

    m_ui->stopbitBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    m_ui->stopbitBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    m_ui->stopbitBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    m_ui->flowcontrolBox->addItem(tr("None"), QSerialPort::NoFlowControl);
    m_ui->flowcontrolBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    m_ui->flowcontrolBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
}


void SettingDialog::updateSettings()
{
    m_currentSettings.name = m_ui->serialPortInfoListBox->currentText();

    if (m_ui->baudrateBox->currentIndex() == 4) {
        m_currentSettings.baudRate = m_ui->baudrateBox->currentText().toInt();
    } else {
        m_currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
                    m_ui->baudrateBox->itemData(m_ui->baudrateBox->currentIndex()).toInt());
    }
    m_currentSettings.stringBaudRate = QString::number(m_currentSettings.baudRate);

    m_currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
                    m_ui->databitsBox->itemData(m_ui->databitsBox->currentIndex()).toInt());
    m_currentSettings.stringDataBits = m_ui->databitsBox->currentText();

    m_currentSettings.parity = static_cast<QSerialPort::Parity>(
                    m_ui->parityBox->itemData(m_ui->parityBox->currentIndex()).toInt());
    m_currentSettings.stringParity = m_ui->parityBox->currentText();

    m_currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
                    m_ui->stopbitBox->itemData(m_ui->stopbitBox->currentIndex()).toInt());
    m_currentSettings.stringStopBits = m_ui->stopbitBox->currentText();

    m_currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
                    m_ui->flowcontrolBox->itemData(m_ui->flowcontrolBox->currentIndex()).toInt());
    m_currentSettings.stringFlowControl = m_ui->flowcontrolBox->currentText();

}

void SettingDialog::on_searchButton_clicked()
{
    fillPortsInfo();
}
