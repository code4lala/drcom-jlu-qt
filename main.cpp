#include "mainwindow.h"
#include <singleapplication.h>
#include <QTranslator>
#include <QDebug>

int main(int argc, char *argv[])
{
    SingleApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    Q_INIT_RESOURCE(DrCOM_JLU_Qt);
    SingleApplication a(argc,argv);
    SingleApplication::setQuitOnLastWindowClosed(false);

    QFont font=a.font();
    font.setPointSize(10);
    font.setFamily("Microsoft YaHei");
    a.setFont(font);

    QTranslator translator;
    translator.load(":/ts/DrCOM_zh_CN.qm");
    a.installTranslator(&translator);

    MainWindow w;
    QObject::connect(&a,&SingleApplication::instanceStarted,[&w](){
        qDebug()<<"One instance had started. Its window will be shown by the next line of the source code.";
        w.ShowLoginWindow();
    });
    w.show();
    return a.exec();
}
