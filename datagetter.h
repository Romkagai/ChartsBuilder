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

// Стратегия получения данных
// Включает в себя две функции: проверка файла, получение данных.

class IDataGetterStrategy
{
public:
    virtual ~IDataGetterStrategy() = default;
    // Проверка файла
    virtual bool CheckFile(const QString &filePath) = 0;
    // Получение данных (храним данные в листе содержащем пары формата "строка-значение")
    virtual QList<QPair<QString, qreal>> GetData(const QString &filePath) = 0;

};

class SQLiteDataGetterStrategy : public IDataGetterStrategy
{
    bool CheckFile(const QString &filePath)
    {
        QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName(filePath);

        // Проверка открытия базы данных
        if (!database.open())
        {
            return false;
        }

        // Проверка содержания таблиц в базе данных
        QStringList tables = database.tables();
        if (tables.isEmpty()) {
            qDebug() << "В базе данных отсутствуют таблицы";
            database.close();
            return false;
        }

        // Проверка входных данных - условимся тем, что в таблице должны быть два
        // столбца данных
        QString table = tables.first();
        QSqlRecord record = database.record(table);
        if (record.count() != 2) {
            database.close();
            return false;
        }

        database.close();
        return true;
    }

    QList<QPair<QString, qreal>> GetData(const QString &filePath)
    {
        QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName(filePath);
        database.open();

        QList<QPair<QString, qreal>> data;
        // Получаем все таблицы из базы данных
        QStringList tables = database.tables();
        // Берем первую таблицу
        QString table = tables.first();
        // Создаем объект - запрос
        QSqlQuery query;
        // Достаем все значения
        query.exec("SELECT * FROM " + table + " ");
        // Пока еще есть значения, достаем очередные и помещаем в data.
        while (query.next()) {
            QString Time = query.value(0).toString();
            qreal Value = query.value(1).toReal();
            data.append(qMakePair(Time, Value));
        }
        database.close();
        return data;
    }
};


class JSONDataGetterStrategy : public IDataGetterStrategy
{
public:
    bool CheckFile(const QString& filePath)
    {
        // Проверка открытия файла
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly))
        {
            return false;
        }

        // Считали данные
        QByteArray jsonData = file.readAll();
        file.close();

        // Преобразовали данные в документ json
        // Если документ не Null и содержит массив и этот массив не пуст, то все хорошо
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
        if (jsonDoc.isNull() || !jsonDoc.isArray() || jsonDoc.array().isEmpty())
        {
            return false;
        }
        return true;
    }

    QList<QPair<QString, qreal>> GetData(const QString& filePath)
    {
        // Создали файл
        QFile file(filePath);
        // Открыли на чтение
        file.open(QIODevice::ReadOnly);
        // Считали данные
        QByteArray jsonData = file.readAll();
        // Закрыли файл
        file.close();
        // Поместили данные в документ
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
        // С документа достали массив данных
        QJsonArray jsonArray = jsonDoc.array();
        // Возвращаемые данные
        QList<QPair<QString, qreal>> data;
        // Заполняем данные из массива
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

class CSVDataGetterStrategy : public IDataGetterStrategy
{
public:
    bool CheckFile(const QString& filePath)
    {
        // Проверка файла на открываемость
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly))
        {
            return false;
        }

        // Проверка на наличие данных в файле
        QByteArray fileData = file.readAll();
        if (fileData.isEmpty())
        {
            file.close();
            return false;
        }

        // Проверка корректности формата CSV
        QTextStream stream(&fileData);
        QString line = stream.readLine();
        QStringList fields = line.split(",");
        if (fields.size() < 2)
        {
            qDebug() << "Данный файл CSV некорректен";
            file.close();
            return false;
        }

        file.close();
        return true;
    }

    QList<QPair<QString, qreal>> GetData(const QString& filePath)
    {
        QFile file(filePath);
        file.open(QIODevice::ReadOnly);
        QByteArray fileData = file.readAll();
        file.close();

        QList<QPair<QString, qreal>> data;

        QTextStream stream(&fileData);
        stream.readLine(); // Пропускаем заголовок

        while (!stream.atEnd())
        {
            QString line = stream.readLine();
            QStringList fields = line.split(",");
            if (fields.size() >= 2)
            {
                QString time = fields[0];
                qreal value = fields[1].toDouble();
                data.append(QPair<QString, qreal>(time, value));
            }
        }

        return data;
    }
};


#endif // DATAGETTER_H
