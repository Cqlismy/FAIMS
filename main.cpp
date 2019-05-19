#include "mainwindow.h"
#include <QApplication>
#include "frminput.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //frmInput::Instance()->Init("control", "silvery", 10, 10);


    MainWindow w;
    w.show();

    return a.exec();
}
