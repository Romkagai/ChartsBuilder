#ifndef DATAGETTER_H
#define DATAGETTER_H
#include <QString>
#include <QList>

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

// Подключаем библиотеки для работы с SQLITE
#include <QtSql>
#include <QSqlQueryModel>

// Подключаем библиотеки для работы с JSON



class IDataGetterStrategy
{
public:
    virtual ~IDataGetterStrategy() = default;
    virtual bool CheckFile(const QString &filePath) = 0;
    virtual QList<QPair<QString, qreal>> GetData(const QString &filePath) = 0;

};

class SQLiteDataGetterStrategy : public IDataGetterStrategy
{
    bool CheckFile(const QString &filePath)
    {
        openDatabase(filePath);
        QStringList tables = database.tables();
        if (tables.isEmpty()) {
            qDebug() << "В базе данных отсутствуют таблицы";
            closeDatabase();
            return false;
        }
        closeDatabase();
        return true;
    }

    // Пока что за процесс компоновки данных отвечает функция получения данных
    // Так мы обрабатываем данные при их получении и занимаем меньше пространства в памяти при хранении этих данных
    // Но стоит ли компоновку данных выносить в отдельную функцию?
    QList<QPair<QString, qreal>> GetData(const QString &filePath)
    {
        closeDatabase();
        openDatabase(filePath);
        QList<QPair<QString, qreal>> data;
        QStringList tables = database.tables();
        QString tableName = tables.first();
        QSqlQuery query;
        // пока что будем считать, что запрос выполняется в любом случае
        query.exec("SELECT * FROM " + tableName + " "); // предусмотреть sql инъекцию?
        while (query.next()) {
            QString Time = query.value(0).toString();
            qreal Value = query.value(1).toReal();
            data.append(qMakePair(Time, Value));
        }
        closeDatabase();
        return data;
    }

private:
    QSqlDatabase database;

    void openDatabase(const QString& filePath)
    {
        closeDatabase();
        database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName(filePath);

        if (!database.open())
        {
                qDebug() << "Ошибка открытия базы данных:" << database.lastError().text();
        }
    }

    void closeDatabase()
    {
        if (database.isOpen())
        {
                database.close();
        }
    }

};



class JSONDataGetterStrategy : public IDataGetterStrategy
{
public:
    bool CheckFile(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly))
        {
                qDebug() << "Не удалось открыть файл:" << filePath;
                return false;
        }

        QByteArray jsonData = file.readAll();
        file.close();

        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
        if (jsonDoc.isNull() || !jsonDoc.isArray())
        {
                qDebug() << "Данный файл JSON некорректен";
                return false;
        }

        QJsonArray jsonArray = jsonDoc.array();
        if (jsonArray.isEmpty())
        {
                qDebug() << "Данный JSON файл пуст!";
                return false;
        }

        return true;
    }

    QList<QPair<QString, qreal>> GetData(const QString& filePath)
    {
        QFile file(filePath);
        file.open(QIODevice::ReadOnly);
        QByteArray jsonData = file.readAll();
        file.close();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
        QJsonArray jsonArray = jsonDoc.array();
        QList<QPair<QString, qreal>> data;
        for (const QJsonValue& value : jsonArray)
        {
                if (value.isObject())
                {
                    QJsonObject obj = value.toObject();
                    QString Time = obj["Time"].toString();
                    qreal Value = obj["Value"].toDouble();
                    data.append(QPair<QString, qreal>(Time, Value));
                }
        }
        return data;
    }
};

//class IDataGetter
//{
//public:
//    virtual ~IDataGetter() = 0;
//    virtual QList<QPair<QString, qreal>> getData() const = 0;

//};

#endif // DATAGETTER_H
