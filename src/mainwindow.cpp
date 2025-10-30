
#include "mainwindow.h"
#include "httphelper.h"
#include "thememanager.h"
#include "databasehelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QHeaderView>
#include <QApplication>
#include <QMessageBox>
#include <QDateTime>
#include <QUrl>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSplitter>
#include <QTextCodec>
#include <QFile>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>

// 包含QCustomPlot头文件
#include "qcustomplot.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_titleBar(nullptr)
    , m_mainLayout(nullptr)
    , m_controlLayout(nullptr)
    , m_titleLayout(nullptr)
    , m_tableWidget(nullptr)
    , m_chartWidget(nullptr)
    , m_historyButton(nullptr)
    , m_refreshButton(nullptr)
    , m_statusLabel(nullptr)
    , m_titleLabel(nullptr)
    , m_minimizeButton(nullptr)
    , m_maximizeButton(nullptr)
    , m_closeButton(nullptr)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_refreshTimer(new QTimer(this))
    , m_isDragging(false)
    , m_isMaximized(false)
    , m_isDarkTheme(false)
    , m_themeManager(&ThemeManager::instance())
{
    // 初始化股票代码列表
    m_stockCodes << "sh600000" << "sh600036" << "sz000001" << "sz000002";

    // 设置UI
    setupUI();

    // 设置定时器
    connect(m_refreshTimer, &QTimer::timeout, this, &MainWindow::refreshData);
    m_refreshTimer->start(2000); // 每2秒刷新一次

    // 加载历史数据
    historyData();

    // 初始刷新数据
    refreshData();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // 创建中央窗口部件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    // 创建主布局
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // 创建标题栏
    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("titleBar");
    m_titleBar->setFixedHeight(30);

    // 创建标题栏布局
    m_titleLayout = new QHBoxLayout(m_titleBar);
    m_titleLayout->setContentsMargins(5, 0, 5, 0);

    // 创建标题标签
    m_titleLabel = new QLabel("TickerLite - 极简行情软件", this);
    m_titleLabel->setStyleSheet("font-weight: bold;");

    // 创建窗口控制按钮
    m_minimizeButton = new QPushButton("━", this);
    m_minimizeButton->setFixedSize(30, 30);
    connect(m_minimizeButton, &QPushButton::clicked, this, &MainWindow::onMinimizeButtonClicked);

    m_maximizeButton = new QPushButton("□", this);
    m_maximizeButton->setFixedSize(30, 30);
    connect(m_maximizeButton, &QPushButton::clicked, this, &MainWindow::onMaximizeButtonClicked);

    m_closeButton = new QPushButton("✕", this);
    m_closeButton->setFixedSize(30, 30);
    connect(m_closeButton, &QPushButton::clicked, this, &MainWindow::onCloseButtonClicked);

    // 创建主题切换按钮
    QPushButton *themeButton = new QPushButton("Light", this);
    themeButton->setFixedSize(46, 30);
    themeButton->setObjectName("themeButton");
    connect(themeButton, &QPushButton::clicked, this, &MainWindow::toggleTheme);

    // 添加控件到标题栏布局
    m_titleLayout->addWidget(m_titleLabel);
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(themeButton);
    m_titleLayout->addWidget(m_minimizeButton);
    m_titleLayout->addWidget(m_maximizeButton);
    m_titleLayout->addWidget(m_closeButton);

    // 创建控制布局
    m_controlLayout = new QHBoxLayout();

    m_groupBtn = new QButtonGroup(this);
    // 历史数据
    m_historyButton = new QPushButton("历史", this);
    connect(m_historyButton, &QPushButton::clicked, this, &MainWindow::historyData);

    // 创建刷新按钮
    m_refreshButton = new QPushButton("刷新", this);
    connect(m_refreshButton, &QPushButton::clicked, this, &MainWindow::refreshData);

    m_groupBtn->addButton(m_historyButton);
    m_groupBtn->addButton(m_refreshButton);

    // 创建状态标签
    m_statusLabel = new QLabel("准备就绪", this);
    m_statusLabel->setObjectName("statusLabel");

    // 添加控件到控制布局
    m_controlLayout->addWidget(m_historyButton);
    m_controlLayout->addWidget(m_refreshButton);
    m_controlLayout->addWidget(m_statusLabel);
    m_controlLayout->addStretch();

    // 创建分割器
    QSplitter *splitter = new QSplitter(Qt::Vertical, this);

    // 初始化表格
    initializeTable();

    // 初始化图表
    initializeChart();

    // 添加表格和图表到分割器
    splitter->addWidget(m_tableWidget);
    splitter->addWidget(m_chartWidget);

    // 设置分割器比例
    splitter->setSizes({300, 200});

    // 添加控件到主布局
    m_mainLayout->addWidget(m_titleBar);
    m_mainLayout->addLayout(m_controlLayout);
    m_mainLayout->addWidget(splitter);

    // 设置窗口标题和大小
    setWindowTitle("TickerLite - 极简行情软件");
    resize(800, 600);

    // 去掉窗口边框
    setWindowFlags(Qt::FramelessWindowHint);

    // 设置圆角窗口
    setAttribute(Qt::WA_TranslucentBackground);
    m_centralWidget->setObjectName("centralWidget");

    // 应用初始主题
    applyTheme(m_isDarkTheme);

}

void MainWindow::initializeTable()
{
    // 创建表格
    m_tableWidget = new QTableWidget(this);

    // 设置列
    QStringList headers;
    headers << "股票代码" << "名称" << "当前价" << "涨跌额" << "涨跌幅(%)" 
            << "昨收价" << "开盘价" << "成交量" << "外盘" << "内盘" << "更新时间";
    m_tableWidget->setColumnCount(headers.size());
    m_tableWidget->setHorizontalHeaderLabels(headers);

    // 设置行数
    m_tableWidget->setRowCount(m_stockCodes.size());

    // 设置表格属性
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableWidget->setAlternatingRowColors(true); // 启用交替行颜色
    m_tableWidget->verticalHeader()->setVisible(false); // 隐藏垂直表头

    // 初始化表格内容
    for (int i = 0; i < m_stockCodes.size(); ++i) {
        // 显示股票代码时去除前缀"v_"
        QString displayCode = m_stockCodes[i].mid(2); // 去掉"v_"前缀
        m_tableWidget->setItem(i, 0, new QTableWidgetItem(displayCode));
        for (int j = 1; j < headers.size(); ++j) {
            m_tableWidget->setItem(i, j, new QTableWidgetItem("--"));
        }
    }
}

void MainWindow::initializeChart()
{
    // 创建图表
    m_chartWidget = new QCustomPlot(this);

    // 配置图表
    m_chartWidget->addGraph();
    m_chartWidget->graph(0)->setPen(QPen(Qt::blue));
    m_chartWidget->graph(0)->setName("价格走势");

    // 设置坐标轴标签
    m_chartWidget->xAxis->setLabel("时间");
    m_chartWidget->yAxis->setLabel("价格");

    // 设置坐标轴范围
    m_chartWidget->xAxis->setRange(0, 10);
    m_chartWidget->yAxis->setRange(0, 20);

    // 显示图例
    m_chartWidget->legend->setVisible(true);

    // 设置网格
    m_chartWidget->xAxis->grid()->setVisible(true);
    m_chartWidget->yAxis->grid()->setVisible(true);

    // 设置交互
    m_chartWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void MainWindow::historyData()
{
    // 默认查询sh600000股票过去5分钟的历史数据
    QString stockCode = "sh600000";
    
    // 确保数据库已初始化
    if (!DatabaseHelper::instance().initializeDatabase()) {
        m_statusLabel->setText("数据库初始化失败");
        return;
    }
    
    // 计算过去5分钟的时间范围
    QDateTime endTime = QDateTime::currentDateTime();
    QDateTime startTime = endTime.addSecs(-300); // 5分钟 = 300秒
    
    // 从数据库查询历史数据
    QList<QVariantMap> historyData = DatabaseHelper::instance().getStockHistory(stockCode, startTime, endTime);
    
    if (historyData.isEmpty()) {
        qDebug() << "没有找到历史数据:" << stockCode;
        m_statusLabel->setText(QString("没有找到 %1 的历史数据").arg(stockCode));
        return;
    }

    // 数据已经按时间顺序排列，直接使用
    for (const auto &record : historyData) {
        // 获取时间戳并转换为秒
        double timestampSec = record["timestamp"].toLongLong() / 1000.0;
        
        // 获取价格
        double price = record["price"].toDouble();
        
        if (!m_datas.HasTimestamp(timestampSec))
        {
            m_datas.Update(timestampSec, price);
        }
    }
    
    // 更新图表
    updateChart();
    
    // 更新状态标签
    m_statusLabel->setText(QString("已加载 %1 的历史数据").arg(stockCode));
}

void MainWindow::refreshData()
{
    m_statusLabel->setText("正在刷新数据...");
    // 遍历所有股票代码，请求数据
    for (const QString &code : m_stockCodes) {
        
        // 构建腾讯行情接口URL    
        QString url = QString("http://qt.gtimg.cn/q=%1").arg(code);        
        QNetworkRequest request;    
        request.setUrl(QUrl(url));    
        request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
        // 腾讯接口始终返回GBK编码，需要在客户端进行转换
        QNetworkReply *reply = m_networkManager->get(request);

        // 连接请求完成信号
        connect(reply, &QNetworkReply::finished, this, [this, reply, code]() {
            onNetworkReplyFinished(reply);
        });
    }
}

void MainWindow::onNetworkReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network error:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    // 获取URL以确定是哪个股票的数据
    QString url = reply->url().toString();
    QString code;

    // 从URL中提取股票代码
    int index = url.lastIndexOf("=");
    if (index > 0) {
        code = url.mid(index + 1);
    }

    // 读取数据
    QByteArray data = reply->readAll();
    // 腾讯接口返回的是GBK编码，需要转换为UTF-8
    QTextCodec *gbk = QTextCodec::codecForName("GBK");
    QString dataStr = gbk->toUnicode(data);

    // 解析数据
    QString parsedData = HttpHelper::parseSinaData(dataStr, code);

    if (!parsedData.isEmpty()) {
        QStringList parts = parsedData.split("|");
        if (parts.size() > 9) {
            QString name = parts[0];
            QString price = parts[1];
            QString change = parts[2];
            QString changePercent = parts[3];
            QString prevClose = parts[4];
            QString openPrice = parts[5];
            QString volume = parts[6];
            QString outerDisc = parts[7];
            QString innerDisc = parts[8];
            QString timestamp = parts[9];
            // 保存到SQLite数据库
            DatabaseHelper::instance().saveStockData(
                code, name, price.toDouble(), prevClose.toDouble(), change.toDouble(),
                changePercent.toDouble(), openPrice.toDouble(),
                volume, outerDisc, innerDisc, timestamp.toLongLong()
            );

            // 找到对应的行
            int row = m_stockCodes.indexOf(code);
            if (row >= 0) {
                // 更新表格
                m_tableWidget->setItem(row, 1, new QTableWidgetItem(name));
                m_tableWidget->setItem(row, 2, new QTableWidgetItem(price));
                m_tableWidget->setItem(row, 3, new QTableWidgetItem(change));
                m_tableWidget->setItem(row, 4, new QTableWidgetItem(changePercent));
                m_tableWidget->setItem(row, 5, new QTableWidgetItem(prevClose));
                m_tableWidget->setItem(row, 6, new QTableWidgetItem(openPrice));
                m_tableWidget->setItem(row, 7, new QTableWidgetItem(volume));
                m_tableWidget->setItem(row, 8, new QTableWidgetItem(outerDisc));
                m_tableWidget->setItem(row, 9, new QTableWidgetItem(innerDisc));
                m_tableWidget->setItem(row, 10, new QTableWidgetItem(QDateTime::fromMSecsSinceEpoch(timestamp.toLongLong()).toString("hh:mm:ss")));

                // 根据涨跌设置颜色
                double changeValue = change.toDouble();
                QColor color;
                if (m_isDarkTheme) {
                    // 深色主题：涨为浅红色，跌为浅绿色
                    color = (changeValue >= 0) ? QColor(255, 100, 100) : QColor(100, 255, 100);
                } else {
                    // 浅色主题：涨为红色，跌为绿色
                    color = (changeValue >= 0) ? Qt::red : Qt::green;
                }
                m_tableWidget->item(row, 2)->setForeground(color);
                m_tableWidget->item(row, 3)->setForeground(color);
                m_tableWidget->item(row, 4)->setForeground(color);

                // 更新图表（以第一个股票为例）
                if (row == 0) {
                    // 添加新数据点
                    double currentTime = timestamp.toLongLong() / 1000.0;
                    double currentPrice = price.toDouble();

                    if (!m_datas.HasTimestamp(currentTime))
                    {
                        m_datas.Update(currentTime, currentPrice);
                    }

                    // 更新图表
                    updateChart();
                }
            }
        }
    }
    else {
        qDebug() << "Failed to parse data for code:" << code;
    }

    reply->deleteLater();

    // 检查是否所有请求都已完成
    static int pendingRequests = m_stockCodes.size();
    pendingRequests--;

    if (pendingRequests <= 0) {
        m_statusLabel->setText("数据更新完成");
        pendingRequests = m_stockCodes.size(); // 重置计数器
    }
}

QString MainWindow::loadStyleSheet(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "无法加载样式表文件：" << fileName;
        return QString();
    }

    QTextStream in(&file);
    QString styleSheet = in.readAll();
    file.close();

    return styleSheet;
}

void MainWindow::applyTheme(bool isDark)
{
    // 使用主题管理器设置主题
    m_themeManager->setTheme(isDark ? ThemeManager::Dark : ThemeManager::Light);
    
    // 设置标题栏样式
    if (isDark) {
        // 更新图表样式
        m_chartWidget->setBackground(QBrush(QColor(43, 43, 43)));
        m_chartWidget->xAxis->setTickLabelColor(Qt::white);
        m_chartWidget->yAxis->setTickLabelColor(Qt::white);
        m_chartWidget->xAxis->setLabelColor(Qt::white);
        m_chartWidget->yAxis->setLabelColor(Qt::white);
        m_chartWidget->xAxis->setBasePen(QPen(Qt::white));
        m_chartWidget->yAxis->setBasePen(QPen(Qt::white));
        m_chartWidget->xAxis->setTickPen(QPen(Qt::white));
        m_chartWidget->yAxis->setTickPen(QPen(Qt::white));
        m_chartWidget->xAxis->setSubTickPen(QPen(Qt::white));
        m_chartWidget->yAxis->setSubTickPen(QPen(Qt::white));
        m_chartWidget->xAxis->grid()->setPen(QPen(QColor(80, 80, 80), 0, Qt::DotLine));
        m_chartWidget->yAxis->grid()->setPen(QPen(QColor(80, 80, 80), 0, Qt::DotLine));
        m_chartWidget->graph(0)->setPen(QPen(Qt::red));
    } else {
        // 更新图表样式
        m_chartWidget->setBackground(QBrush(QColor(255, 255, 255)));
        m_chartWidget->xAxis->setTickLabelColor(Qt::black);
        m_chartWidget->yAxis->setTickLabelColor(Qt::black);
        m_chartWidget->xAxis->setLabelColor(Qt::black);
        m_chartWidget->yAxis->setLabelColor(Qt::black);
        m_chartWidget->xAxis->setBasePen(QPen(Qt::black));
        m_chartWidget->yAxis->setBasePen(QPen(Qt::black));
        m_chartWidget->xAxis->setTickPen(QPen(Qt::black));
        m_chartWidget->yAxis->setTickPen(QPen(Qt::black));
        m_chartWidget->xAxis->setSubTickPen(QPen(Qt::black));
        m_chartWidget->yAxis->setSubTickPen(QPen(Qt::black));
        m_chartWidget->xAxis->grid()->setPen(QPen(QColor(200, 200, 200), 0, Qt::DotLine));
        m_chartWidget->yAxis->grid()->setPen(QPen(QColor(200, 200, 200), 0, Qt::DotLine));
        m_chartWidget->graph(0)->setPen(QPen(Qt::blue));
    }
    
    // 更新表格项颜色
    for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
        QTableWidgetItem *changeItem = m_tableWidget->item(row, 3); // 涨跌额
        if (changeItem) {
            double changeValue = changeItem->text().toDouble();
            QColor color;
            if (isDark) {
                // 深色主题：涨为浅红色，跌为浅绿色
                color = (changeValue >= 0) ? QColor(255, 100, 100) : QColor(100, 255, 100);
            } else {
                // 浅色主题：涨为红色，跌为绿色
                color = (changeValue >= 0) ? Qt::red : Qt::green;
            }
            m_tableWidget->item(row, 2)->setForeground(color); // 当前价
            m_tableWidget->item(row, 3)->setForeground(color); // 涨跌额
            m_tableWidget->item(row, 4)->setForeground(color); // 涨跌幅
        }
    }
    
    // 更新主题切换按钮图标和样式
    QPushButton *themeButton = m_titleBar->findChild<QPushButton*>("themeButton");
    if (themeButton) {
        themeButton->setText(isDark ? "Dark" : "Light");
        m_themeManager->updateWidgetStyle(themeButton);
    }
    
    // 更新状态标签样式
    m_themeManager->updateWidgetStyle(m_statusLabel);
    
    // 更新历史数据按钮样式
    m_themeManager->updateWidgetStyle(m_historyButton);
    
    // 更新主窗口样式
    m_themeManager->updateWidgetStyle(this);
    
    // 更新图表样式
    m_themeManager->updateWidgetStyle(m_chartWidget);
}

void MainWindow::updateChart()
{
    QReadLocker rlock(&m_datas.lock);
    if (m_datas.timestamps.isEmpty() || m_datas.prices.isEmpty()) {
        return;
    }

    // 更新图表数据
    m_chartWidget->graph(0)->setData(m_datas.timestamps, m_datas.prices);

    // 调整坐标轴范围
    if (m_datas.timestamps.size() > 1) {
        double minTime = m_datas.timestamps.first();
        double maxTime = m_datas.timestamps.last();
        double timeRange = maxTime - minTime;

        // 扩展范围以留出边距
        m_chartWidget->xAxis->setRange(minTime - timeRange * 0.1, maxTime + timeRange * 0.1);

        // 计算价格范围
        double minPrice = *std::min_element(m_datas.prices.begin(), m_datas.prices.end());
        double maxPrice = *std::max_element(m_datas.prices.begin(), m_datas.prices.end());
        double priceRange = maxPrice - minPrice;

        // 扩展范围以留出边距
        m_chartWidget->yAxis->setRange(minPrice - priceRange * 0.1, maxPrice + priceRange * 0.1);
    }

    // 格式化X轴为时间
    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat("hh:mm:ss");
    m_chartWidget->xAxis->setTicker(dateTicker);

    // 重绘图表
    m_chartWidget->replot();
}

void MainWindow::onMinimizeButtonClicked()
{
    showMinimized();
}

void MainWindow::onMaximizeButtonClicked()
{
    if (m_isMaximized) {
        // 恢复窗口大小
        showNormal();
        m_maximizeButton->setText("□");
        m_isMaximized = false;
    } else {
        // 最大化窗口
        // 获取屏幕可用区域大小
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->availableGeometry();
        
        // 考虑自定义标题栏的高度，设置窗口位置和大小
        setGeometry(screenGeometry);
        m_maximizeButton->setText("❐");
        m_isMaximized = true;
    }
}

void MainWindow::onCloseButtonClicked()
{
    close();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 只有点击标题栏区域才能拖动
        if (m_titleBar->geometry().contains(event->pos())) {
            m_isDragging = true;
            m_dragPosition = event->globalPos() - frameGeometry().topLeft();
            event->accept();
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && m_isDragging) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_isDragging = false;
    event->accept();
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    // 双击标题栏最大化/还原窗口
    if (m_titleBar->geometry().contains(event->pos())) {
        onMaximizeButtonClicked();
        event->accept();
    }
}

void MainWindow::toggleTheme()
{
    m_isDarkTheme = !m_isDarkTheme;
    applyTheme(m_isDarkTheme);
}

MainWindow::PrivateDatas::PrivateDatas()
{
    timestamps.reserve(maxSize);
    prices.reserve(maxSize);
}

void MainWindow::PrivateDatas::Update(double timestamp, double price)
{
    QWriteLocker wLock(&lock);
    if (timestamps.isEmpty() || timestamp > timestamps.last())
    {
        timestamps.push_back(timestamp);
        prices.push_back(price);
        if (timestamps.size() > maxSize)
        {
            timestamps.removeFirst();
            prices.removeFirst();
        }
    }

}
bool MainWindow::PrivateDatas::HasTimestamp(double timestamp)
{
    QReadLocker wLock(&lock);
    auto item = std::find_if(timestamps.cbegin(), timestamps.cend(), [timestamp](const double &a){
        return (a == timestamp);
    });
    if (item != timestamps.cend())
    {
        return true;
    }
    return false;
}
