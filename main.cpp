#include "mainwindow.h"
#include <singleapplication.h>
#include <QTranslator>

int main(int argc, char *argv[])
{
    SingleApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    Q_INIT_RESOURCE(DrCOM_JLU_Qt);
    SingleApplication a(argc,argv);
    SingleApplication::setQuitOnLastWindowClosed(false);

    QFont font=a.font();
    font.setPointSize(12);
    a.setFont(font);

    QTranslator translator;
    translator.load(":/ts/DrCOM_zh_CN.qm");
    a.installTranslator(&translator);

    MainWindow w;
    QObject::connect(&a,&SingleApplication::instanceStarted,[&w](){
        w.ShowLoginWindow();
    });
    w.show();
    return a.exec();
}
