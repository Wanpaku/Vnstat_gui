#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QStyleHints>
#include <QSettings>
#include <QTranslator>
#include <QDir>
#include <QActionGroup>
#include <QCloseEvent>
#include <cmath>

QT_BEGIN_NAMESPACE

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
    MainWindow(const MainWindow& src) = delete;
    auto operator=(const MainWindow& rhs) -> MainWindow& = delete;
    MainWindow(const MainWindow&& src) = delete;
    auto operator=(const MainWindow&& rhs) -> MainWindow& = delete;

private slots:
    static void on_actionQuit_triggered();
    void at_language_changed(QAction* action);
    void on_units_combo_activated(int index);
    void on_interface_combo_activated(int index);
    void on_time_period_combo_activated(int index);
    static void on_actionAbout_triggered();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    Ui::MainWindow* ui;
    QMap<QString, QString> interfaces;

    const QMap<int, QString> table_names {
        { 0, "fiveminute" },
        { 1, "hour"       },
        { 2, "day"        },
        { 3, "month"      },
        { 4, "year"       },
        { 5, "top"        },
    };
    const QString vnstat_db_path {
        "/var/lib/vnstat/vnstat.db"
    };
    QSettings* settings { nullptr };
    QActionGroup* languages_group { nullptr };
    QTranslator start_translator;
    const QMap<int, double> units_names {
        { 0, 1.                    },
        { 1, 1024.                 },
        { 2, 1024. * 1024.         },
        { 3, 1024. * 1024. * 1024. }
    };
    enum class Table_Column : std::uint8_t {
        Date,
        Received,
        Transmitted,
        Total,
    };

    void load_settings();
    void create_language_menu();
    void load_interface_language(const QString& interface_language);
    void switch_translator(QTranslator& translator, const QString& filename);
    void changeEvent(QEvent* event) override;
    void save_interface_language_config();
    static auto open_sql_database_file(const QString& path) -> bool;
    void query_sql_database();
    template <typename T>
    void set_cell(const int row, const int col, const T data_cell);
    void load_table_settings();
    void load_interfaces_combobox();
    void load_time_periods_combobox();
    void load_units_combobox();
    void save_inteface_choice();
    void save_time_period_choice();
    void save_unit_choice();
};
#endif // MAINWINDOW_H
