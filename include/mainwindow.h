
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMouseEvent>
#include <QPoint>
#include <QButtonGroup>
#include <QReadWriteLock>

// 前向声明 QCustomPlot，避免包含整个头文件
class QCustomPlot;
class ThemeManager;

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void historyData();
    void refreshData();
    void onNetworkReplyFinished(QNetworkReply* reply);
    void onMinimizeButtonClicked();
    void onMaximizeButtonClicked();
    void onCloseButtonClicked();
    void toggleTheme();

private:
    void setupUI();
    void initializeTable();
    void initializeChart();
    void updateChart();

    // 鼠标事件处理
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    // 辅助方法
    QString loadStyleSheet(const QString &fileName);
    void applyTheme(bool isDark);

    // UI组件
    QWidget *m_centralWidget;
    QWidget *m_titleBar;          // 标题栏
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_controlLayout; // 控制按钮布局
    QHBoxLayout *m_titleLayout;   // 标题栏布局
    QTableWidget *m_tableWidget;
    QCustomPlot *m_chartWidget;
    QButtonGroup *m_groupBtn;
    QPushButton *m_historyButton;
    QPushButton *m_refreshButton;
    QLabel *m_statusLabel;
    QLabel *m_titleLabel;         // 标题标签
    QPushButton *m_minimizeButton; // 最小化按钮
    QPushButton *m_maximizeButton; // 最大化按钮
    QPushButton *m_closeButton;    // 关闭按钮

    // 网络和数据
    QNetworkAccessManager *m_networkManager;
    QTimer *m_refreshTimer;

    // 示例股票代码列表
    QStringList m_stockCodes;

    // 窗口拖动相关
    bool m_isDragging;
    QPoint m_dragPosition;
    bool m_isMaximized;

    // 主题相关
    bool m_isDarkTheme;
    ThemeManager* m_themeManager;

    class PrivateDatas {
    public:
        PrivateDatas();
        QReadWriteLock lock;
        QVector<double> timestamps;
        QVector<double> prices;
        void Update(double, double);
        bool HasTimestamp(double);
    private:
        const int maxSize = 150;
    };
    friend class PrivateDatas;
    PrivateDatas m_datas;
};

#endif // MAINWINDOW_H
