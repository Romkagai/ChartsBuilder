#ifndef DATAGETTER_H
#define DATAGETTER_H
#include <QString>
#include <QList>

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

class IDataGetter
{
public:
    virtual ~IDataGetter() = default;
    virtual bool CheckFile(const QString &filePath) = 0;
    virtual QList<QPair<QString, qreal>> getData(const QString &filePath) = 0;

};

class SQLiteDataGetter : public IDataGetter
{
    bool CheckFile(const QString &filePath)
    {
        closeDatabase();
        openDatabase(filePath);
        QStringList tables = database.tables();
        if (tables.isEmpty()) {
            qDebug() << "В базе данных отсутствуют таблицы";
            return 0;
        }
        return 1;
    }

    QList<QPair<QString, qreal>> getData(const QString &filePath)
    {
        QList<QPair<QString, qreal>> data;
        QStringList tables = database.tables();
        QString tableName = tables.first();
        QSqlQuery query;
        if (!query.exec    ("SELECT substr(Time, 7, 4) || '-' || substr(Time, 4, 2) AS month_year, AVG(Value) AS average "
                        "FROM " + tableName + " "
                                      "GROUP BY month_year"))
        {
            qDebug() << "Не удалось выполнить запрос";
        }
        while (query.next()) {
            QString monthYear = query.value(0).toString();
            qreal average = query.value(1).toReal();
            data.append(qMakePair(monthYear, average));
        }
        return data;
    }

    void closeDatabase()
    {
        if (database.isOpen())
        {
            database.close();
            database.removeDatabase(database.connectionName());
        }
    }

    bool openDatabase(const QString& filePath)
    {
        closeDatabase();

        database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName(filePath);

        if (!database.open())
        {
            qDebug() << "Ошибка открытия базы данных:" << database.lastError().text();
                return false;
        }

        return true;
    }

private:
    QSqlDatabase database;

};



//class IDataGetter
//{
//public:
//    virtual ~IDataGetter() = 0;
//    virtual QList<QPair<QString, qreal>> getData() const = 0;

//};

//class IDataGetter
//{
//public:
//    virtual ~IDataGetter() = 0;
//    virtual QList<QPair<QString, qreal>> getData() const = 0;

//};

#endif // DATAGETTER_H
