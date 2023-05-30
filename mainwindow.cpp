#include "mainwindow.h"
#include "ioc_container.h"
#include "datagetter.h"

int IOCContainer::s_nextTypeId = 0;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{

    // Первоначальная настройка главного окна
    ui->setupUi(this);
    this->setGeometry(100, 100, 1500, 500);
    this->setStatusBar(new QStatusBar(this));
    this->statusBar()->showMessage("Выбранный путь : ");    // изменить(?)

    // Первоначальная настройка дерева файлов (файловой системы)
    QString homePath = QDir::homePath();
    leftPartModel = new QFileSystemModel(this);
    leftPartModel -> setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
    leftPartModel -> setRootPath(homePath);

    // Первоначальная настройка таблицы файлов (файловой системы)
    rightPartModel = new QFileSystemModel(this);
    rightPartModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
    rightPartModel->setRootPath(homePath);

    // Настриваем дерево файлов на основе leftPartModel
    treeView = new QTreeView();
    treeView->setModel(leftPartModel);
    treeView->expandAll();
    treeView->header()->resizeSection(0, 200);

    // Настриваем таблицу файлов на основе rightPartModel
    tableView = new QTableView();
    tableView->setModel(rightPartModel);

    // Создаем объект "сплиттер(разделитель)", будет разделять дерево и таблицу
    QSplitter *splitter = new QSplitter(parent);


    //--------------------------------------------------------------------------------------------

    // Первоначальная настройка графика
    // Создание виджета управления для графика
    chartView = new QChartView();
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(600, 400);
    chart = chartView->chart();

    // Установка осей
    axisX = new QBarCategoryAxis();
    chart->addAxis(axisX, Qt::AlignBottom);

    axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);

    //--------------------------------------------------------------------------------------------

    // Добавим выбор графиков - comboBox
    comboBox = new QComboBox();
    comboBox->addItem("Столбчатая диаграмма");
    comboBox->addItem("Круговая диаграмма");

    //---------------------------------------------------------------------------------------------------

    // Добавим выбор цвета - checkBox
    checkBox = new QCheckBox("Черно-белый");


    //---------------------------------------------------------------------------------------------------
    // Создание главного макета
    QHBoxLayout *mainLayout = new QHBoxLayout();
    QVBoxLayout *VerticalBox = new QVBoxLayout();

    // Добавление виджетов на QSplitter
    splitter->addWidget(treeView);
    splitter->addWidget(tableView);

    // Добавление в левую часть кнопок и графика
    VerticalBox->addWidget(chartView);
    VerticalBox->addWidget(comboBox);
    VerticalBox->addWidget(checkBox);

    // Добавление QSplitter и виджета с графиком на главный макет
    mainLayout->addWidget(splitter);
    mainLayout->addLayout(VerticalBox);


    // Создание виджета для главного макета
    QWidget *mainWidget = new QWidget();
    mainWidget->setLayout(mainLayout);

    // Установка виджета на главное окно
    setCentralWidget(mainWidget);

    //---------------------------------------------------------------------------------------

    // Сигналы
    // Отслеживание выбранного файла в дереве
    QItemSelectionModel *treeSelectionModel = treeView->selectionModel();
    connect(treeSelectionModel, &QItemSelectionModel::selectionChanged, this, &MainWindow::on_selectionTreeChangedSlot);

    // Отслеживание выбранного файла в таблице файлов
    QItemSelectionModel *tableSelectionModel = tableView->selectionModel();
    connect(tableSelectionModel, &QItemSelectionModel::selectionChanged, this, &MainWindow::on_selectionTableChangedSlot);

    // Подключение сигнала activated к слоту comboBoxItemSelected
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(comboBoxItemSelected(int)));

    // Соединяем сигнал "stateChanged" CheckBox со слотом для изменения цветов графика
    connect(checkBox, &QCheckBox::stateChanged, this, &MainWindow::onCheckBoxStateChanged);

    //Пример организации установки курсора в TreeView относительно модельного индекса
    QItemSelection toggleSelection;
    QModelIndex topLeft;
    topLeft = leftPartModel->index(homePath);
    toggleSelection.select(topLeft, topLeft);
    treeSelectionModel->select(toggleSelection, QItemSelectionModel::Toggle);
}

/*
 * Слот для обработки выбора элемента в TreeView.
 * Выбор осуществляется с помощью курсора.
 */

void MainWindow::on_selectionTreeChangedSlot(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);

    QModelIndexList indexs =  selected.indexes();
    QString filePath = "";

    // Размещаем информацию в statusbar относительно выделенного модельного индекса
    // Смотрим, сколько индексов было выделено.
    // В нашем случае выделяем только один, следовательно всегда берем только первый.
    if (indexs.count() >= 1) {
        QModelIndex ix =  indexs.constFirst();
        filePath = leftPartModel->filePath(ix);
        this->statusBar()->showMessage("Выбранный путь : " + leftPartModel->filePath(indexs.constFirst()));
    }

    // Получив выбранные данные из левой части filePath(путь к папке/файлу).
    // Для представления в правой части устанваливаем корневой индекс относительно filePath.
    // Табличное представление отображает только файлы, находящиеся в filePath (папки не отображает)
    tableView->setRootIndex(rightPartModel->setRootPath(filePath));
}


/*
 * Слот для обработки выбора элемента в TableView.
 * Выбор осуществляется с помощью курсора.
 */
void MainWindow::on_selectionTableChangedSlot(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);

    QModelIndex index = tableView->selectionModel()->currentIndex();
    filePath = rightPartModel->filePath(index);
    this->statusBar()->showMessage("Выбранный файл : " + filePath);

    IOCContainer DataGetterContainer;
    DataGetterContainer.RegisterInstance<IDataGetter, SQLiteDataGetter>();
    if(DataGetterContainer.GetObject<IDataGetter>()->CheckFile(filePath))
    {
        fileData = DataGetterContainer.GetObject<IDataGetter>()->getData(filePath);
    }

    if(!fileData.isEmpty())
    {
    QString selectedText = comboBox->currentText();
    if (selectedText == "Столбчатая диаграмма")
    {
        DrawBar();
    }

    if (selectedText == "Круговая диаграмма")
    {
        DrawPie();
    }
    }

}

void MainWindow::onCheckBoxStateChanged(int state)
{
    if (state == Qt::Checked)
    {
    QGraphicsColorizeEffect* effect = new QGraphicsColorizeEffect;
    effect->setColor(Qt::black);
    chart->setGraphicsEffect(effect);
    }
    else
    {

    chart->setGraphicsEffect(nullptr);
    }
}

void MainWindow::comboBoxItemSelected(int index)
{

    if(!fileData.isEmpty()){
    QString selectedText = comboBox->itemText(index);
    if (selectedText == "Столбчатая диаграмма")
    {
        DrawBar();
    }

    if (selectedText == "Круговая диаграмма")
    {
        DrawPie();
    }
    }
}

void MainWindow::DrawBar()
{
    chart->removeAllSeries();
    // Получение минимального и максимального значения из выборки
    qreal minValue = std::numeric_limits<qreal>::max();
    qreal maxValue = std::numeric_limits<qreal>::lowest();

    // Получение минимального и максимального значения из выборки
    QBarSeries *series = new QBarSeries();
    for (const QPair<QString, qreal>& pair : fileData) {
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
    chart->addSeries(series);
    chart->setTitle("Среднее значение по месяцам");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    axisY->setRange(minValue - 5, maxValue + 5 );

    // Установка осей
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    // Создание представления диаграммы
    chartView->setRenderHint(QPainter::Antialiasing);

    // Отображение окна
    chartView->update();
}

void MainWindow::DrawPie()
{
    chart->removeAllSeries();

    // Создание серии данных для круговой диаграммы
    QPieSeries *series = new QPieSeries();
    for (const QPair<QString, qreal>& pair : fileData) {
        QString month = pair.first;
        qreal average = pair.second;
        series->append(month, average);
    }

    // Создание диаграммы
    chart->addSeries(series);
    chart->setTitle("Среднее значение по месяцам");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Отображение окна
    chartView->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}
