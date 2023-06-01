#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QFileSystemModel>
#include <QTreeView>
#include <QTableView>
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

#include "ioc_container.h"
#include "chartdrawer.h"


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

    void setChartStrategy(std::shared_ptr<ChartStrategy> strategy)
    {
        chartStrategy = strategy;
    }

    void drawChart()
    {
        chartStrategy->draw(chartView, fileData);
    }




private slots:
    void on_selectionTreeChangedSlot(const QItemSelection &selected, const QItemSelection &deselected);
    void on_selectionListChangedSlot(const QItemSelection &selected, const QItemSelection &deselected);
    void comboBoxItemSelected(int index);
    void onCheckBoxStateChanged(int state);
    void onButtonOpenTreeView();



private:
    Ui::MainWindow *ui;

    // Файловая система
    QFileSystemModel *rightPartModel;
    QFileSystemModel *leftPartModel;
    QTreeView *treeView;
    QListView *listView;

    // Все для графика
    QChartView *chartView;
//    QChart *chart;
//    QBarCategoryAxis *axisX;
//    QValueAxis *axisY;

    // Все для комбобокса
    QComboBox *comboBox;

    // Путь до файла
    QString filePath;

    // Данные
    QList<QPair<QString, qreal>> fileData;

    // Выбор цвета
    QCheckBox *checkBox;

    // Кнопка открытия дерева файлов
    QPushButton* openTreeView;

    // Текст "Выбрать тип диаграммы"
    QLabel* diagrammType;

    //IoC_контейнер
    IOCContainer DataGetterContainer;

    // Стратегия для рисования графика
    std::shared_ptr<ChartStrategy> chartStrategy;

};

#endif // MAINWINDOW_H
