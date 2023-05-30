#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QFileSystemModel>
#include <QTreeView>
#include <QTableView>
#include "ui_mainwindow.h"
#include <QSplitter>
#include <QListView>
#include <QTreeView>
#include <QFileSystemModel>
#include <QItemSelectionModel>
#include <QTableView>
#include <QHeaderView>
#include <QStatusBar>

// Подключаем библиотеки для работы с БД
#include <QtSql>
#include <QSqlQueryModel>

// Подключаем библиотеки для работы с графиками
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts>
#include <QPainter>
#include <QPdfWriter>


QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow

{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void DrawBar();
    void DrawPie();




private slots:
    void on_selectionTreeChangedSlot(const QItemSelection &selected, const QItemSelection &deselected);
    void on_selectionTableChangedSlot(const QItemSelection &selected, const QItemSelection &deselected);
    void comboBoxItemSelected(int index);
    void onCheckBoxStateChanged(int state);



private:
    Ui::MainWindow *ui;

    // Файловая система
    QFileSystemModel *rightPartModel;
    QFileSystemModel *leftPartModel;
    QTreeView *treeView;
    QTableView *tableView;

    // Все для графика
    QChartView *chartView;
    QChart *chart;
    QBarCategoryAxis *axisX;
    QValueAxis *axisY;

    // Все для комбобокса
    QComboBox *comboBox;

    // Путь до файла
    QString filePath;

    // Данные
    QList<QPair<QString, qreal>> fileData;

    // Выбор цвета
    QCheckBox *checkBox;

};

#endif // MAINWINDOW_H
