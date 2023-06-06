#ifndef CHARTDRAWER_H
#define CHARTDRAWER_H
#include <QString>

// Подключаем библиотеки для работы с графиками
#include <QChartView>
#include <QBarSeries>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts>
#include <QPainter>
#include <QPdfWriter>

// Используем шаблонный метод
// Какие шаги алгоритма выделить? Печать добавить тут?

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
        qreal minValue = std::numeric_limits<qreal>::max();
        qreal maxValue = std::numeric_limits<qreal>::lowest();

        // Получение минимального и максимального значения из выборки (пока что не использую, затем добавить отрисовку осей)


        // !!!!!пока что отрисовываем 10 первых значений (группировка данных будет рассмотрена позже)!!!!!


        QBarSeries *series = new QBarSeries();
        for (int i = 0; i < 10; ++i) {
            const QPair<QString, qreal>& pair = data[i];
            QString time = pair.first;
            qreal value = pair.second;
            QBarSet *barSet = new QBarSet(time);
            *barSet << value;
            series->append(barSet);
            minValue = std::min(minValue, value);
            maxValue = std::max(maxValue, value);
        }
        chartView->chart()->addSeries(series);
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
