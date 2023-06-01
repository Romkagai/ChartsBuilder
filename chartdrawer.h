#ifndef CHARTDRAWER_H
#define CHARTDRAWER_H
#include <QString>
#include <QChartView>
#include <QBarSeries>
// Подключаем библиотеки для работы с графиками
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts>
#include <QPainter>
#include <QPdfWriter>

class ChartStrategy
{
public:
    virtual void draw(QChartView* chartView, const QList<QPair<QString, qreal>>& data) = 0;
};

class BarChartStrategy : public ChartStrategy
{
public:
    void draw(QChartView* chartView, const QList<QPair<QString, qreal>>& data)
    {
        chartView->chart()->removeAllSeries();
        // Получение минимального и максимального значения из выборки
        qreal minValue = std::numeric_limits<qreal>::max();
        qreal maxValue = std::numeric_limits<qreal>::lowest();

        // Получение минимального и максимального значения из выборки
        QBarSeries *series = new QBarSeries();
        for (const QPair<QString, qreal>& pair : data) {
            QString month = pair.first;
            qreal average = pair.second;
            QBarSet *barSet = new QBarSet(month);
            *barSet << average;
            series->append(barSet);

            minValue = std::min(minValue, average);
            maxValue = std::max(maxValue, average);
        }

        // --------------------------------------------------------------------------------

        // Создание диаграммы
        chartView->chart()->addSeries(series);
        chartView->chart()->setTitle("Среднее значение по месяцам");
        chartView->chart()->setAnimationOptions(QChart::SeriesAnimations);

        //chartView->chart()->axisY()->setRange(minValue - 5, maxValue + 5 );

        // Установка осей
//        series->attachAxis(axisX);
//        series->attachAxis(axisY);

        // Создание представления диаграммы
        chartView->setRenderHint(QPainter::Antialiasing);

        // Отображение окна
        chartView->update();
    }

};

class PieChartStrategy : public ChartStrategy
{
public:
    void draw(QChartView* chartView, const QList<QPair<QString, qreal>>& data)
    {
        chartView->chart()->removeAllSeries();

        // Создание серии данных для круговой диаграммы
        QPieSeries *series = new QPieSeries();
        for (const QPair<QString, qreal>& pair : data) {
            QString month = pair.first;
            qreal average = pair.second;
            series->append(month, average);
        }

        // Создание диаграммы
        chartView->chart()->addSeries(series);
        chartView->chart()->setTitle("Среднее значение по месяцам");
        chartView->chart()->setAnimationOptions(QChart::SeriesAnimations);

        // Отображение окна
        chartView->show();
    }

};


#endif // CHARTDRAWER_H
