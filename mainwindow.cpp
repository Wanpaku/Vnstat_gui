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
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    load_settings();
    create_language_menu();
    qInfo() << tr("Start program Vnstat_gui.");
    load_time_periods_combobox();
    load_interfaces_combobox();
    load_units_combobox();
    query_sql_database();
}

MainWindow::~MainWindow()
{
    settings->deleteLater();
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    // Exit from the app by the shortcut
    QApplication::quit();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // Closing app event
    qInfo() << tr("Exit from the app");
    event->accept();
}

void MainWindow::load_settings()
{
    // Loading settings to variable settings
    if (settings == nullptr) {
        settings = new QSettings("Vnstat_gui");
    }
    settings->setFallbacksEnabled(false);
}

void MainWindow::create_language_menu()
{
    // Dynamic creation of a language interface menu
    languages_group = new QActionGroup(ui->menu_Interface_language);
    languages_group->setExclusive(true);

    connect(languages_group, SIGNAL(triggered(QAction*)), this,
            SLOT(at_language_changed(QAction*)));

    const QString& default_language { "English" };
    const QString current_language
        = settings->value("interface_language", default_language).toString();
    QString default_locale;

    if (current_language == default_language) {
        default_locale = "en_US";
    } else {
        default_locale = current_language;
    }

    const QString language_path = QApplication::applicationDirPath();

    const QDir dir(language_path);
    QStringList filenames = dir.entryList(QStringList("Vnstat_gui_*.qm"));
    for (const auto& filename : std::as_const(filenames)) {
        QString locale;
        locale = filename;
        locale.truncate(locale.lastIndexOf('.'));
        locale.remove(0, locale.lastIndexOf("i") + 2);
        const QString language
            = QLocale::languageToString(QLocale(locale).language());
        QAction* action = new QAction(language, ui->menubar);
        action->setCheckable(true);
        action->setData(locale);
        ui->menu_Interface_language->addAction(action);
        languages_group->addAction(action);
        if (default_locale == locale) {
            action->setChecked(true);
        }
    }

    load_interface_language(current_language);
}

void MainWindow::at_language_changed(QAction* action)
{
    // Function for a slot that reacting on changes of interface language
    if (action != nullptr) {
        load_interface_language(action->data().toString());
        load_time_periods_combobox();
        load_units_combobox();
        qInfo() << tr("Interface language changed to: ")
                << action->data().toString();
    }
}

void MainWindow::load_interface_language(const QString& interface_language)
{
    // Downloading new interface language
    const QString& default_language { "English" };
    QString current_language
        = settings->value("interface_language", default_language).toString();

    current_language = interface_language;
    const QLocale locale = QLocale(current_language);
    QLocale::setDefault(locale);
    switch_translator(start_translator,
                      QString("Vnstat_gui_%1.qm").arg(interface_language));

    save_interface_language_config();
}

void MainWindow::switch_translator(QTranslator& translator,
                                   const QString& filename)
{
    // Deleting old QTranslator and downloading new QTranslator
    qApp->removeTranslator(&translator);

    const QString path = QApplication::applicationDirPath() + '/';

    if (start_translator.load(path + filename)) {
        qApp->installTranslator(&start_translator);
    }
}

void MainWindow::changeEvent(QEvent* event)
{
    // Updating of the interface language on the fly
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::save_interface_language_config()
{
    // Saving current interface language
    const QString& current_interface_language
        = languages_group->checkedAction()->data().toString();
    settings->setValue("interface_language", current_interface_language);
}

auto MainWindow::open_sql_database_file(const QString& path) -> bool
{
    // Opening of the vnstat database file
    QSqlDatabase vnstat_db = QSqlDatabase::addDatabase("QSQLITE");
    vnstat_db.setDatabaseName(path);

    if (!vnstat_db.open()) {
        qDebug() << tr("Error: Could not open vnstat database file")
                 << vnstat_db.lastError();
        QMessageBox::critical(nullptr, tr("Database Error"),
                              tr("Could not open database file: ")
                                  + vnstat_db.lastError().text());
        return false;
    }
    return true;
}

void MainWindow::query_sql_database()
{
    // Query sql database and send the data to the table widget
    if (open_sql_database_file(vnstat_db_path)) {
        QSqlQuery query;

        const QString interface_name = ui->interface_combo->currentText();
        const int time_period = ui->time_period_combo->currentIndex();
        const int unit_name = ui->units_combo->currentIndex();

        if (!interface_name.isEmpty() && time_period != -1
            && unit_name != -1) {
            const double units_denom { units_names[unit_name] };
            const QString sql_command("SELECT date, rx, tx FROM "
                                      + table_names[time_period]
                                      + " WHERE "
                                        "interface = "
                                      + interfaces[interface_name]);

            if (query.exec(sql_command)) {
                ui->data_table->clearContents();
                ui->data_table->setRowCount(0);
                while (query.next()) {
                    const int new_row = ui->data_table->rowCount();
                    ui->data_table->insertRow(new_row);

                    QString date = query.value(0).toString();
                    // Format month and year date
                    if (time_period == 3) {
                        const qsizetype index = date.lastIndexOf('-');
                        date.remove(index, 3);
                    } else if (time_period == 4) {
                        const qsizetype index = date.indexOf('-');
                        date.remove(index, 6);
                    }
                    const double received_data
                        = query.value(1).toDouble() / units_denom;
                    const double transmitted_data
                        = query.value(2).toDouble() / units_denom;
                    const double total_data = received_data + transmitted_data;

                    set_cell(new_row, static_cast<int>(Table_Column::Date),
                             date);

                    set_cell(new_row, static_cast<int>(Table_Column::Received),
                             received_data);
                    set_cell(new_row,
                             static_cast<int>(Table_Column::Transmitted),
                             transmitted_data);
                    set_cell(new_row, static_cast<int>(Table_Column::Total),
                             total_data);
                }
                load_table_settings();

            } else {
                qWarning() << "Database text: "
                           << query.lastError().databaseText() << "Driver text"
                           << query.lastError().driverText()
                           << "Last query: " << query.lastQuery();
            }
        }

        QSqlDatabase::database().close();
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }
}

template <typename T>
void MainWindow::set_cell(const int row, const int col, const T data_cell)
{
    // Insert data from database to the table widget
    QTableWidgetItem* total_data_item = new QTableWidgetItem();
    if constexpr (std::is_same_v<T, double>) {
        const double formatte_data = std::round(data_cell * 100. + 0.5) / 100.;
        total_data_item->setData(Qt::DisplayRole, formatte_data);
    } else {
        total_data_item->setData(Qt::DisplayRole, data_cell);
    }

    total_data_item->setTextAlignment(Qt::AlignCenter);
    ui->data_table->setItem(row, col, total_data_item);
}

void MainWindow::load_table_settings()
{
    // Loading table widget settings
    ui->data_table->horizontalHeader()->setResizeContentsPrecision(-1);
    ui->data_table->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    ui->data_table->verticalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    ui->data_table->setSortingEnabled(true);
}

void MainWindow::load_interfaces_combobox()
{
    // Loading interfaces combobox at the start of the app
    if (open_sql_database_file(vnstat_db_path)) {
        QSqlQuery query;
        query.exec("SELECT DISTINCT name, id FROM interface");
        while (query.next()) {
            interfaces[query.value(0).toString()] = query.value(1).toString();
            ui->interface_combo->addItem(query.value(0).toString());
        }

        const int no_interface { -1 };
        const int current_interface_choice
            = settings->value("interface_choice", no_interface).toInt();
        if (current_interface_choice != no_interface) {
            ui->interface_combo->setCurrentIndex(current_interface_choice);
        }

        QSqlDatabase::database().close();
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }
}

void MainWindow::load_time_periods_combobox()
{
    // Loading time periods combobox at the start of the app
    ui->time_period_combo->clear();
    const QStringList time_periods { tr("5th minutes"), tr("Hour"),
                                     tr("Day"),         tr("Month"),
                                     tr("Year"),        tr("Top days") };
    ui->time_period_combo->addItems(time_periods);
    const int no_time_period { -1 };
    const int current_time_period_choice
        = settings->value("time_period_choice", no_time_period).toInt();
    if (current_time_period_choice != no_time_period) {
        ui->time_period_combo->setCurrentIndex(current_time_period_choice);
    }
}

void MainWindow::load_units_combobox()
{
    // Loading units combobox at the start of the app
    ui->units_combo->clear();
    const QStringList units_list { tr("Bytes"), tr("Kilobytes"),
                                   tr("Megabytes"), tr("Gigabytes") };
    ui->units_combo->addItems(units_list);
    const int no_unit { -1 };
    const int current_unit_choice
        = settings->value("unit_choice", no_unit).toInt();

    if (current_unit_choice != no_unit) {
        ui->units_combo->setCurrentIndex(current_unit_choice);
    }
}

void MainWindow::save_inteface_choice()
{
    const int current_interface_choice = ui->interface_combo->currentIndex();
    settings->setValue("interface_choice", current_interface_choice);
}

void MainWindow::save_time_period_choice()
{
    const int current_time_period_choice
        = ui->time_period_combo->currentIndex();
    settings->setValue("time_period_choice", current_time_period_choice);
}

void MainWindow::save_unit_choice()
{
    const int current_unit_choice = ui->units_combo->currentIndex();
    settings->setValue("unit_choice", current_unit_choice);
}

void MainWindow::on_units_combo_activated(int index)
{
    save_unit_choice();
    query_sql_database();
}

void MainWindow::on_interface_combo_activated(int index)
{
    save_inteface_choice();
    query_sql_database();
}

void MainWindow::on_time_period_combo_activated(int index)
{
    save_time_period_choice();
    query_sql_database();
}

void MainWindow::on_actionAbout_triggered()
{
    // Show content of About message
    QMessageBox msgBox;
    msgBox.setText(tr(
        "<b><p><center>Vnstat_gui</b></p></center>"
        "<p><center>This is a simple program that shows network statistics. "
        "The data loading from vnstat database file. "
        "Therefore you need to have installed vnstat application on yours PC. "
        "</p></center>"
        "<p><center>The app icon created by "
        "<a href='https://www.flaticon.com/free-icon/setting_7415329'>Eucalyp "
        "- "
        "Flaticon</a></p></center>"
        "<p><center>License: <a "
        "href='https://www.gnu.org/licenses/gpl-3.0.html'>GNU Public "
        "License v3</a></p></center>"
        "<p><center>Copyright 2025 ©Teg Miles "
        "(movarocks2@gmail.com)</p></center>"));
    qInfo() << tr("Watched About info");
    msgBox.exec();
}
