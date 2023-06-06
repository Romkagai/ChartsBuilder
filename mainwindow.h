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
#include "datagetter.h"

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

    void setDataGetterStrategy(std::shared_ptr<IDataGetterStrategy> strategy)
    {
        DataGetterStrategy = strategy;
    }

    bool CheckFile()
    {
        return DataGetterStrategy->CheckFile(filePath);
    }

    QList<QPair<QString, qreal>> GetData()
    {
        return DataGetterStrategy->GetData(filePath);
    }




private slots:
    void on_selectionTreeChangedSlot(const QItemSelection &selected, const QItemSelection &deselected);
    void on_selectionListChangedSlot(const QItemSelection &selected, const QItemSelection &deselected);
    void comboBoxItemSelected();
    void onCheckBoxStateChanged(int state);
    void onButtonOpenTreeView();



private:
    Ui::MainWindow *ui;

    // Файловая система
    QFileSystemModel *rightPartModel;
    QFileSystemModel *leftPartModel;
    QTreeView *treeView;
    QListView *listView;

    // Окно вывода графика
    QChartView *chartView;

    // Выбор типа графика
    QComboBox *comboBox;

    // Путь до файла
    QString filePath;

    // Данные
    QList<QPair<QString, qreal>> fileData;

    // Чек-бокс выбора цвета графика
    QCheckBox *checkBox;

    // Кнопка открытия дерева файлов
    QPushButton* openTreeView;

    // Текст "Выбрать тип диаграммы"
    QLabel* diagrammType;

    //IoC_контейнер
    IOCContainer Container;

    //Стратегия для взятия данных
    std::shared_ptr<IDataGetterStrategy> DataGetterStrategy;

};

#endif // MAINWINDOW_H
