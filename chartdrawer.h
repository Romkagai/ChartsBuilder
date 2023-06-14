#ifndef CHARTDRAWER_H
#define CHARTDRAWER_H
#include <QString>

// Подключаем библиотеки для работы с графиками
#include <QChartView>
#include <QBarSeries>
#include <QtCharts>

// Используем шаблонный метод
class ChartDrawer
{
public:
    // Общий реализуемый алгоритм
    void DrawChart(QChartView* chartView, const QList<QPair<QString, qreal>>& data)
    {
        // Очистка окна рисования
        chartView->chart()->removeAllSeries();

        // Первый шаг - подготовка полученных данных (здесь закладываем их в chartView)
        PrepareData(chartView, data);

        // Второй шаг - настройка диаграммы (в зависимости от типа)
        ConfigureChart(chartView);

        // Обновление окна вывода
        chartView->update();
    };

protected:
    // Данные функции должны быть переопределены наследниками в зависимости от их предпочтений
    virtual void PrepareData(QChartView* chartView, const QList<QPair<QString, qreal>>& data) = 0;
    virtual void ConfigureChart(QChartView* chartView) = 0;
};


// Класс для создания столбчатых диаграмм
class BarChartDrawer : public ChartDrawer
{
protected:
    void PrepareData(QChartView* chartView, const QList<QPair<QString, qreal>>& data)
    {
        // Создаем два словаря для группировки данных
        QMap<QString, qreal> groupedData;
        QMap<QString, int> groupCount;
        // Группировка данных по месяцам и годам
        for (const QPair<QString, qreal>& pair : data) {
            QString time = pair.first;
            qreal value = pair.second;

            // Пример:
            // time = 01.01.2005 01:00
            // отсекаем часы
            // И берем год и месяц в виде
            // yearMonth  = 2005-01

            // Извлекаем год и месяц из строки времени
            QStringList dateParts = time.split(" ")[0].split(".");
            QString yearMonth = dateParts[2] + "-" + dateParts[1];

            // Добавляем взятые значения к словарям
            groupedData[yearMonth] += value;
            groupCount[yearMonth]++;
        }

        // Создаем серии и добавляем их на график
        std::unique_ptr<QBarSeries> series = std::make_unique<QBarSeries>();

        for (const QString& yearMonth : groupedData.keys()) {
            // Вычисляем среднее значение для каждого года-месяца
            qreal averageValue = groupedData[yearMonth] / groupCount[yearMonth];
            std::unique_ptr<QBarSet> barSet = std::make_unique<QBarSet>(yearMonth);
            *barSet << averageValue;
            series->append(barSet.release());
        }

        chartView->chart()->addSeries(series.release());
    }

    void ConfigureChart(QChartView* chartView)
    {
        chartView->chart()->setTitle("Столбчатая диаграмма");
        chartView->chart()->setAnimationOptions(QChart::SeriesAnimations);
        chartView->setRenderHint(QPainter::Antialiasing);
    }

};

// Класс для создания круговых диаграмм

class PieChartDrawer : public ChartDrawer
{
protected:
    void PrepareData(QChartView* chartView, const QList<QPair<QString, qreal>>& data)
    {
        QPieSeries *series = new QPieSeries();
        for (int i = 0; i < 10; ++i) {
            const QPair<QString, qreal>& pair = data[i];
            QString time = pair.first;
            qreal value = pair.second;
            series->append(time, value);
        }
        chartView->chart()->addSeries(series);
    }

    void ConfigureChart(QChartView* chartView)
    {
        chartView->chart()->setTitle("Круговая диаграмма");
        chartView->chart()->setAnimationOptions(QChart::SeriesAnimations);
        chartView->setRenderHint(QPainter::Antialiasing);
    }

};


#endif // CHARTDRAWER_H
