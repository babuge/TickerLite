
#include "thememanager.h"
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QMainWindow>
#include <QStyle>

#include "qcustomplot.h"

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
    , m_currentTheme(ThemeType::Unkonw)
{
    initializeColorMaps();
}

ThemeManager& ThemeManager::instance()
{
    static ThemeManager instance;
    return instance;
}

void ThemeManager::setTheme(ThemeType theme)
{
    if (m_currentTheme != theme) {
        m_currentTheme = theme;
        applyCurrentTheme();
        emit themeChanged(m_currentTheme == Dark);
    }
}

QColor ThemeManager::getColor(const QString& colorName) const
{
    if (m_currentTheme == Dark) {
        return m_darkColors.value(colorName, QColor());
    } else {
        return m_lightColors.value(colorName, QColor());
    }
}

QString ThemeManager::loadStyleSheet(const QString& fileName)
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

void ThemeManager::applyCurrentTheme()
{
    QString styleSheet;
    if (m_currentTheme == Dark) {
        styleSheet = loadStyleSheet(":/resources/dark.qss");
        qApp->setProperty("darkTheme", true);
    } else {
        styleSheet = loadStyleSheet(":/resources/light.qss");
        qApp->setProperty("darkTheme", false);
    }

    qApp->setStyleSheet(styleSheet);
}

void ThemeManager::updateWidgetStyle(QWidget* widget)
{
    if (!widget) return;

    // 强制更新样式
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();


    // 如果是QCustomPlot，确保样式正确应用
    if (QCustomPlot* customPlot = qobject_cast<QCustomPlot*>(widget)) {
        if (m_currentTheme == Dark) {
            // 深色主题
            customPlot->setBackground(QBrush(QColor(43, 43, 43)));
            customPlot->xAxis->setTickLabelColor(Qt::white);
            customPlot->yAxis->setTickLabelColor(Qt::white);
            customPlot->xAxis->setLabelColor(Qt::white);
            customPlot->yAxis->setLabelColor(Qt::white);
            customPlot->xAxis->setBasePen(QPen(Qt::white));
            customPlot->yAxis->setBasePen(QPen(Qt::white));
            customPlot->xAxis->setTickPen(QPen(Qt::white));
            customPlot->yAxis->setTickPen(QPen(Qt::white));
            customPlot->xAxis->setSubTickPen(QPen(Qt::white));
            customPlot->yAxis->setSubTickPen(QPen(Qt::white));
            customPlot->xAxis->grid()->setPen(QPen(QColor(80, 80, 80), 0, Qt::DotLine));
            customPlot->yAxis->grid()->setPen(QPen(QColor(80, 80, 80), 0, Qt::DotLine));
            
            // 更新图表线条颜色
            for (int i = 0; i < customPlot->graphCount(); ++i) {
                customPlot->graph(i)->setPen(QPen(QColor(255, 100, 100), 2));
            }
        } else {
            // 浅色主题
            customPlot->setBackground(QBrush(QColor(255, 255, 255)));
            customPlot->xAxis->setTickLabelColor(Qt::black);
            customPlot->yAxis->setTickLabelColor(Qt::black);
            customPlot->xAxis->setLabelColor(Qt::black);
            customPlot->yAxis->setLabelColor(Qt::black);
            customPlot->xAxis->setBasePen(QPen(Qt::black));
            customPlot->yAxis->setBasePen(QPen(Qt::black));
            customPlot->xAxis->setTickPen(QPen(Qt::black));
            customPlot->yAxis->setTickPen(QPen(Qt::black));
            customPlot->xAxis->setSubTickPen(QPen(Qt::black));
            customPlot->yAxis->setSubTickPen(QPen(Qt::black));
            customPlot->xAxis->grid()->setPen(QPen(QColor(200, 200, 200), 0, Qt::DotLine));
            customPlot->yAxis->grid()->setPen(QPen(QColor(200, 200, 200), 0, Qt::DotLine));
            
            // 更新图表线条颜色
            for (int i = 0; i < customPlot->graphCount(); ++i) {
                customPlot->graph(i)->setPen(QPen(QColor(0, 102, 204), 2));
            }
        }
        
        // 重绘图表
        customPlot->replot();
    }

    // 递归更新子控件
    const QObjectList& children = widget->children();
    for (QObject* child : children) {
        if (QWidget* childWidget = qobject_cast<QWidget*>(child)) {
            updateWidgetStyle(childWidget);
        }
    }
}

void ThemeManager::initializeColorMaps()
{
    // 浅色主题颜色
    m_lightColors["window"] = QColor(255, 255, 255);
    m_lightColors["base"] = QColor(255, 255, 255);
    m_lightColors["text"] = QColor(0, 0, 0);
    m_lightColors["button"] = QColor(240, 240, 240);
    m_lightColors["highlight"] = QColor(224, 224, 224);
    m_lightColors["titleBar"] = QColor(240, 240, 240);
    m_lightColors["border"] = QColor(208, 208, 208);
    m_lightColors["grid"] = QColor(200, 200, 200);
    m_lightColors["statusLabel"] = QColor(240, 240, 240);
    m_lightColors["positive"] = QColor(255, 0, 0);    // 涨为红色
    m_lightColors["negative"] = QColor(0, 128, 0);    // 跌为绿色

    // 深色主题颜色
    m_darkColors["window"] = QColor(43, 43, 43);
    m_darkColors["base"] = QColor(43, 43, 43);
    m_darkColors["text"] = QColor(255, 255, 255);
    m_darkColors["button"] = QColor(68, 68, 68);
    m_darkColors["highlight"] = QColor(68, 68, 68);
    m_darkColors["titleBar"] = QColor(51, 51, 51);
    m_darkColors["border"] = QColor(85, 85, 85);
    m_darkColors["grid"] = QColor(80, 80, 80);
    m_darkColors["statusLabel"] = QColor(37, 37, 37);
    m_darkColors["positive"] = QColor(255, 100, 100);  // 涨为浅红色
    m_darkColors["negative"] = QColor(100, 255, 100);  // 跌为浅绿色
}
