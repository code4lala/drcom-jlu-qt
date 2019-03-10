// 设定single application的继承类
#define QAPPLICATION_CLASS QApplication
#include "mainwindow.h"
#include <singleapplication.h>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(DrCOM_JLU_Qt);
    SingleApplication a(argc,argv);
    SingleApplication::setQuitOnLastWindowClosed(false);
    MainWindow w;
    QObject::connect(&a,&SingleApplication::instanceStarted,[&w](){
        w.ShowLoginWindow();
    });
    w.show();
    return a.exec();
}
