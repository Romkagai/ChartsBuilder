#include "mainwindow.h"
#include "ioc_container.h"
#include "datagetter.h"
#include "chartdrawer.h"

int IOCContainer::s_nextTypeId = 0;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{

    //--------------------------------------------------------------------------------------------
    // Первоначальная настройка главного окна
    ui->setupUi(this);
    this->setGeometry(100, 100, 1500, 500);
    this->setStatusBar(new QStatusBar(this));
    this->statusBar()->showMessage("Выбранный путь : ");    // изменить(?)

    //--------------------------------------------------------------------------------------------
    // Настройка файловых систем
    // Первоначальная настройка дерева файлов (файловой системы)
    QString homePath = QDir::homePath();
    leftPartModel = new QFileSystemModel(this);
    leftPartModel -> setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
    leftPartModel -> setRootPath(homePath);

    // Первоначальная настройка таблицы файлов (файловой системы)
    rightPartModel = new QFileSystemModel(this);
    rightPartModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
    rightPartModel->setRootPath(homePath);

    QStringList formats;
    formats << "sqlite" << "json" << "csv";
    QStringList filters;
    for (const QString& format : formats) {
        filters.append(QString("*.%1").arg(format));
    }

    rightPartModel->setNameFilters(filters);
    rightPartModel->setNameFilterDisables(false);


    // Настриваем дерево файлов на основе leftPartModel
    treeView = new QTreeView();
    treeView->setModel(leftPartModel);
    treeView->expandAll();
    treeView->header()->resizeSection(0, 200);

    // Настриваем таблицу файлов на основе rightPartModel
    listView = new QListView();
    listView->setModel(rightPartModel);


    //--------------------------------------------------------------------------------------------
    // Первоначальная настройка графика
    chartView = new QChartView();
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(600, 400);


    // Установка осей
//    axisX = new QBarCategoryAxis();
//    chart->addAxis(axisX, Qt::AlignBottom);
//    axisY = new QValueAxis();
//    chart->addAxis(axisY, Qt::AlignLeft);

    //--------------------------------------------------------------------------------------------

    // Настройка выбора графиков
    comboBox = new QComboBox();
    comboBox->addItem("Столбчатая диаграмма");
    comboBox->addItem("Круговая диаграмма");

    //---------------------------------------------------------------------------------------------------

    // Добавим выбор цвета - checkBox
    checkBox = new QCheckBox("Черно-белый");

    //---------------------------------------------------------------------------------------------------

    // Настройка кнопки для открытия дерева выбора папки
    openTreeView = new QPushButton("Открыть");

    //---------------------------------------------------------------------------------------------------

    // Настройка строки "Выберите тип диаграммы"
    diagrammType = new QLabel("Выберите тип диаграммы");

    //---------------------------------------------------------------------------------------------------
    // Компоновка окна
    // Создание главного макета
    // Часть окна с функциями и часть окна с выводом
    QHBoxLayout *functionLayout = new QHBoxLayout();
    QSplitter *splitter = new QSplitter(parent);

    // Добавление виджетов на часть с функциями
    functionLayout->addWidget(openTreeView);
    functionLayout->addWidget(diagrammType);
    functionLayout->addWidget(comboBox);
    functionLayout->addWidget(checkBox);

    // Добавление виджетов на QSplitter
    splitter->addWidget(listView);
    splitter->addWidget(chartView);

    // Основное окно
    QVBoxLayout *mainLayout = new QVBoxLayout();

    // Добавление QSplitter и виджета с графиком на главный макет
    mainLayout->addLayout(functionLayout);
    mainLayout->addWidget(splitter);

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

    QItemSelectionModel *listSelectionModel = listView->selectionModel();
    connect(listSelectionModel, &QItemSelectionModel::selectionChanged, this, &MainWindow::on_selectionListChangedSlot);

    // Подключение сигнала activated к слоту comboBoxItemSelected
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(comboBoxItemSelected(int)));

    // Соединяем сигнал "stateChanged" CheckBox со слотом для изменения цветов графика
    connect(checkBox, &QCheckBox::stateChanged, this, &MainWindow::onCheckBoxStateChanged);

    // Соединим сигнал от кнопки
    connect(openTreeView, &QPushButton::clicked, this, &MainWindow::onButtonOpenTreeView);


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

    listView->setRootIndex(rightPartModel->setRootPath(filePath));
}


/*
 * Слот для обработки выбора элемента в TableView.
 * Добавить проверку новых данных перед рисованием
 */
void MainWindow::on_selectionListChangedSlot(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);

    QModelIndex index = listView->selectionModel()->currentIndex();
    filePath = rightPartModel->filePath(index);
    this->statusBar()->showMessage("Выбранный файл : " + filePath);

    DataGetterContainer.RegisterInstance<IDataGetter, SQLiteDataGetter>();

    if(DataGetterContainer.GetObject<IDataGetter>()->CheckFile(filePath))
    {
        fileData = DataGetterContainer.GetObject<IDataGetter>()->getData(filePath);
        if(!fileData.isEmpty())
        {
            QString selectedText = comboBox->currentText();
            if (selectedText == "Столбчатая диаграмма")
            {
                DataGetterContainer.RegisterInstance<ChartStrategy, BarChartStrategy>();
                setChartStrategy(DataGetterContainer.GetObject<ChartStrategy>());
                drawChart();
            }

            if (selectedText == "Круговая диаграмма")
            {
                DataGetterContainer.RegisterInstance<ChartStrategy, PieChartStrategy>();
                setChartStrategy(DataGetterContainer.GetObject<ChartStrategy>());
                drawChart();
            }
        }
        else
        {
        //вывод отсутствия данных на экран
        }
    }
}

void MainWindow::onCheckBoxStateChanged(int state)
{
    if (state == Qt::Checked)
    {
    QGraphicsColorizeEffect* effect = new QGraphicsColorizeEffect;
    effect->setColor(Qt::black);
    chartView->chart()->setGraphicsEffect(effect);
    }
    else
    {
    chartView->chart()->setGraphicsEffect(nullptr);
    }
}

void MainWindow::onButtonOpenTreeView()
{
    treeView -> show();
}

void MainWindow::comboBoxItemSelected(int index)
{
    if(!fileData.isEmpty())
    {
            QString selectedText = comboBox->currentText();
            if (selectedText == "Столбчатая диаграмма")
            {
                DataGetterContainer.RegisterInstance<ChartStrategy, BarChartStrategy>();
                setChartStrategy(DataGetterContainer.GetObject<ChartStrategy>());
                drawChart();
            }

            if (selectedText == "Круговая диаграмма")
            {
                DataGetterContainer.RegisterInstance<ChartStrategy, PieChartStrategy>();
                setChartStrategy(DataGetterContainer.GetObject<ChartStrategy>());
                drawChart();
            }
    }
    else
    {
            //вывод отсутствия данных на экран
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}
