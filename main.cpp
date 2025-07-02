/*Copyright (C) 2025  Teg Miles
 This file is part of Vnstat_gui.

Movar is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

Movar is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Vnstat_gui. If not, see <https://www.gnu.org/licenses/>.*/

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
