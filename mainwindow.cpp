#include "mainwindow.h"
#include "ioc_container.h"
#include "datagetter.h"
#include "chartdrawer.h"

int IOCContainer::s_nextTypeId = 0;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{

    //---------------------------------------------------------------------------------------------------

    // Первоначальная настройка главного окна
    this->setWindowTitle("ChartsBuilder");                  // Задаем название
    auto statusBar = std::make_unique<QStatusBar>(this);    // Создаем статус-бар (показывает путь)
    this->setStatusBar(statusBar.release());
    this->statusBar()->showMessage("Выбранный путь: ");     // Заполняем статус-бар

    //---------------------------------------------------------------------------------------------------

    // Настройка файловых систем
    // Первоначальная настройка дерева файлов (файловой системы)
    QString homePath = QDir::homePath();                            // Путь к папке
    leftPartModel = std::make_unique<QFileSystemModel>(this);
    leftPartModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs); // Отображаем только директории
    leftPartModel->setRootPath(homePath);

    // Первоначальная настройка таблицы файлов (файловой системы)
    rightPartModel = std::make_unique<QFileSystemModel>(this);
    rightPartModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);  // Отображаем только файлы
    rightPartModel->setRootPath(homePath);

    // Применяем фильтрацию типов данных для листа файлов по условию задачи
    // Вместо переписывания MVC под себя
    QStringList formats;
    formats << "sqlite" << "json" << "csv";
    QStringList filters;
    for (const QString& format : formats) {
        filters.append(QString("*.%1").arg(format));
    }

    rightPartModel->setNameFilters(filters);
    rightPartModel->setNameFilterDisables(false);


    // Настриваем дерево файлов на основе leftPartModel
    treeView = std::make_unique<QTreeView>();
    treeView->setModel(leftPartModel.get());
    treeView->expandAll();
    treeView->header()->resizeSection(0, 200);

    // Настриваем таблицу файлов на основе rightPartModel
    listView = std::make_unique<QListView>(this);
    listView->setModel(rightPartModel.get());
    listView->setMinimumSize(300, 600);



    //---------------------------------------------------------------------------------------------------

    // Первоначальная настройка окна графика
    chartView = std::make_unique<QChartView>(this);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(600, 600);

    //---------------------------------------------------------------------------------------------------

    // Настройка выбора графиков
    comboBox = std::make_unique<QComboBox>(this);
    comboBox->addItem("Столбчатая диаграмма");
    comboBox->addItem("Круговая диаграмма");

    //---------------------------------------------------------------------------------------------------

    // Добавим выбор цвета - checkBox
    checkBox = std::make_unique<QCheckBox>("Черно-белый", this);

    //---------------------------------------------------------------------------------------------------

    // Настройка кнопки для открытия дерева выбора папки
    openTreeView = std::make_unique<QPushButton>("Открыть", this);

    //---------------------------------------------------------------------------------------------------

    // Настройка кнопки для сохранения графика для печати
    printChart = std::make_unique<QPushButton>("Сохранить график", this);
    printChart->setEnabled(false);  // По умолчанию печатать нечего

    //---------------------------------------------------------------------------------------------------

    // Настройка строки "Выберите тип диаграммы"
    diagrammType = std::make_unique<QLabel>("Выберите тип диаграммы", this);

    //---------------------------------------------------------------------------------------------------
    // Компоновка окна
    // Создание главного макета
    // Часть окна с функциями и часть окна с выводом
    std::unique_ptr<QHBoxLayout> functionLayout = std::make_unique<QHBoxLayout>();
    std::unique_ptr<QSplitter> splitter = std::make_unique<QSplitter>(parent);
    // Добавим "пружину", чтобы склеить текст выбора диаграммы с комбо-боксом
    std::unique_ptr<QSpacerItem> spacer = std::make_unique<QSpacerItem>(10, 10, QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Добавление виджетов на часть с функциями
    // Дополнительно добавим фиксированные растояния для красоты
    functionLayout->addSpacing(10);
    functionLayout->addWidget(openTreeView.get());
    functionLayout->addItem(spacer.release());
    functionLayout->addWidget(diagrammType.get());
    functionLayout->addWidget(comboBox.get());
    functionLayout->addSpacing(10);
    functionLayout->addWidget(checkBox.get());
    functionLayout->addSpacing(10);
    functionLayout->addWidget(printChart.get());
    functionLayout->addSpacing(10);

    // Добавление виджетов на QSplitter
    splitter->addWidget(listView.get());
    splitter->addWidget(chartView.get());

    // Основное окно
    std::unique_ptr<QVBoxLayout> mainLayout = std::make_unique<QVBoxLayout>();

    // Добавление QSplitter и виджета с графиком на главный макет
    mainLayout->addLayout(functionLayout.release());
    mainLayout->addWidget(splitter.release());

    // Создание виджета для главного макета
    std::unique_ptr<QWidget> mainWidget = std::make_unique<QWidget>();
    mainWidget->setLayout(mainLayout.release());

    // Установка виджета на главное окно
    setCentralWidget(mainWidget.release());

    //---------------------------------------------------------------------------------------------------

    // Установка сигнально-слотовых соединений
    // Отслеживание выбранного файла в дереве
    QItemSelectionModel *treeSelectionModel = treeView->selectionModel();
    connect(treeSelectionModel, &QItemSelectionModel::selectionChanged, this, &MainWindow::on_selectionTreeChangedSlot);

    // Отслеживание выбранного файла в таблице файлов
    QItemSelectionModel *listSelectionModel = listView->selectionModel();
    connect(listSelectionModel, &QItemSelectionModel::selectionChanged, this, &MainWindow::on_selectionListChangedSlot);

    // Подключение сигнала activated к слоту comboBoxItemSelected (выбор другого типа графика)
    connect(comboBox.get(), &QComboBox::activated, this, &MainWindow::comboBoxItemSelected);

    // Соединяем сигнал "stateChanged" CheckBox со слотом для изменения цветов графика (переход от цветного к черно-белому)
    connect(checkBox.get(), &QCheckBox::stateChanged, this, &MainWindow::onCheckBoxStateChanged);

    // Соединим сигнал от кнопки "clicked" с открытием окна дерева файлов
    connect(openTreeView.get(), &QPushButton::clicked, this, &MainWindow::onButtonOpenTreeView);

    // Соединим сигнал от кнопки "clicked" с открытием печатью графика
    connect(printChart.get(), &QPushButton::clicked, this, &MainWindow::onPrintChartButton);

    // Пример организации установки курсора в TreeView относительно модельного индекса
    QItemSelection toggleSelection;
    QModelIndex topLeft;
    topLeft = leftPartModel->index(homePath);
    toggleSelection.select(topLeft, topLeft);
    treeSelectionModel->select(toggleSelection, QItemSelectionModel::Toggle);
}

//---------------------------------------------------------------------------------------------------

// Слот для обработки выбора элемента в TreeView. Выбор осуществляется с помощью курсора.
void MainWindow::on_selectionTreeChangedSlot(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);

    QModelIndexList indexes =  selected.indexes();
    QString filePath = "";

    // Размещаем информацию в StatusBar относительно выделенного модельного индекса
    // Смотрим, сколько индексов было выделено.
    // В нашем случае выделяем только один, следовательно всегда берем только первый.
    if (indexes.count() >= 1) {
        QModelIndex ix =  indexes.constFirst();
        filePath = leftPartModel->filePath(ix);
        this->statusBar()->showMessage("Выбранный путь : " + leftPartModel->filePath(indexes.constFirst()));
    }

    // Получив выбранные данные из левой части filePath(путь к папке/файлу).
    // Для представления в правой части устанваливаем корневой индекс относительно filePath.
    // Листовое представление отображает только файлы, находящиеся в filePath (папки не отображает)
    listView->setRootIndex(rightPartModel->setRootPath(filePath));
}

//---------------------------------------------------------------------------------------------------

// Слот для обработки выбора элемента в TableView. Добавить проверку новых данных перед рисованием (!)
// Предлагается сразу переходить к отрисовке графика при нажатии на файл
void MainWindow::on_selectionListChangedSlot(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    Q_UNUSED(selected);

    QModelIndex index = listView->selectionModel()->currentIndex();
    filePath = rightPartModel->filePath(index);
    this->statusBar()->showMessage("Выбранный файл : " + filePath);


    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix();

    // Устанавливаем стратегию
    if (extension == "json")
    {
        Container.RegisterInstance<IDataGetterStrategy, JSONDataGetterStrategy>();
        setDataGetterStrategy(Container.GetObject<IDataGetterStrategy>());
    }
    else if (extension == "sqlite")
    {
        Container.RegisterInstance<IDataGetterStrategy, SQLiteDataGetterStrategy>();
        setDataGetterStrategy(Container.GetObject<IDataGetterStrategy>());
    }
    else if (extension == "csv")
    {
        Container.RegisterInstance<IDataGetterStrategy, CSVDataGetterStrategy>();
        setDataGetterStrategy(Container.GetObject<IDataGetterStrategy>());
    }

    // Если файл проходит проверку
    if(CheckFile())
    {
        // Забираем из него данные
        fileData = GetData();

        // В зависимости от выбранного типа графика
        if (comboBox->currentText() == "Столбчатая диаграмма")
        {
            // Рисуем столбчатую диаграмму
            Container.RegisterInstance<ChartDrawer, BarChartDrawer>();
            Container.GetObject<ChartDrawer>()->DrawChart(chartView.get(), fileData);
        }

        if (comboBox->currentText() == "Круговая диаграмма")
        {
            // Рисуем круговую диаграмму
            Container.RegisterInstance<ChartDrawer, PieChartDrawer>();
            Container.GetObject<ChartDrawer>()->DrawChart(chartView.get(), fileData);
        }
        // Разрешаем печать
        printChart->setEnabled(true);
    }
    else
    {
        // Если файл не прошел проверку
        // То выведем соответствующий текст на график
        chartView->chart()->removeAllSeries();
        chartView->chart()->setTitle("Выбранный файл некорректен!");
        chartView->update();
        // Запрещаем печать
        printChart->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------

// Слот для обработки нажатия Чек-бокса: изменяем эффект на графике
void MainWindow::onCheckBoxStateChanged(int state)
{
    // Если выбран, то применяем эффект
    if (state == Qt::Checked)
    {
        std::unique_ptr<QGraphicsColorizeEffect> effect = std::make_unique<QGraphicsColorizeEffect>();
        effect->setColor(Qt::black);
        chartView->chart()->setGraphicsEffect(effect.get());
        effect.release();
    }
    else
    {
        // Иначе эффект сбрасываем
        chartView->chart()->setGraphicsEffect(nullptr);
    }
}

//---------------------------------------------------------------------------------------------------

// Слот для обработки нажатия кнопки "Открыть" : кнопка открытия дерева
void MainWindow::onButtonOpenTreeView()
{
    treeView->resize(600, 800); // Подредактируем размер
    if (treeView->isVisible())  // Если окно уже отображается (открыто)
    {
        treeView->raise();      // Выведем его поверх
    }
    else
    {
        treeView->show();       // Иначе отобразим его
    }
}

//---------------------------------------------------------------------------------------------------

// Слот для обработки нажатия кнопки "Печать" : сохранение графика
void MainWindow::onPrintChartButton()
{
    QString savePath = QFileDialog::getSaveFileName(this, "Сохранить график", "", "PDF (*.pdf)");
    // Создаем экземпляр класса QPdfWriter для записи в PDF-файл
    QPdfWriter pdfWriter(savePath);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setResolution(300);

    // Создаем экземпляр класса QPainter для рисования на QPdfWriter
    QPainter painter(&pdfWriter);

    // Рисуем график на QPainter
    chartView->render(&painter);

    // Закрываем рисование
    painter.end();
}

//---------------------------------------------------------------------------------------------------

// Слот для обработки выбора из ComboBox - при выборе предлагается сразу отрисовывать график согласно выбранному варианту
void MainWindow::comboBoxItemSelected()
{
    // Если данные не пусты и файл проходит проверку
    if(!fileData.isEmpty() && CheckFile())
    {
        if (comboBox->currentText() == "Столбчатая диаграмма")
        {
            Container.RegisterInstance<ChartDrawer, BarChartDrawer>();
            Container.GetObject<ChartDrawer>()->DrawChart(chartView.get(), fileData);
        }

        if (comboBox->currentText() == "Круговая диаграмма")
        {
            Container.RegisterInstance<ChartDrawer, PieChartDrawer>();
            Container.GetObject<ChartDrawer>()->DrawChart(chartView.get(), fileData);
        }
        // Разрешаем печать
        printChart->setEnabled(true);
    }
    else
    {
        chartView->chart()->removeAllSeries();
        chartView->chart()->setTitle("Выберите файл!");
        chartView->update();
        // Запрещаем печать
        printChart->setEnabled(false);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // "Переопределим" closeEvent
    // Спрячем treeView, чтобы избежать ошибок
    // Хотя программа и использует умные указатели
    // Если открыто treeView и закрывается главное окно, это приводит к поломкам
    treeView->hide();
    // Удалим то, что не удалось создать с помощью умных указателей
    delete treeView->selectionModel();
    delete listView->selectionModel();
    // Вызовем реализацию из QMainWindow
    QMainWindow::closeEvent(event);
}


