#include "mainwindow.h"
#include "logger.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <memory>

int main(int argc, char* argv[])
{
    Logger::init();
    std::unique_ptr<QApplication> app { std::make_unique<QApplication>(argc,
                                                                       argv) };
    QApplication::setWindowIcon(QIcon(":/icons/icons/setting.png"));

    std::unique_ptr<MainWindow> mainwindow { std::make_unique<MainWindow>() };
    mainwindow->show();
    return app->exec();
}
