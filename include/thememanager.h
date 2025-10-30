
#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <QObject>
#include <QColor>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>

/**
 * @brief 主题管理器类，负责管理应用程序的主题切换
 */
class ThemeManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 主题类型枚举
     */
    enum ThemeType {
        Light,  // 浅色主题
        Dark,   // 深色主题
        Unkonw, // none
    };

    /**
     * @brief 获取主题管理器单例实例
     * @return 主题管理器实例
     */
    static ThemeManager& instance();

    /**
     * @brief 设置当前主题
     * @param theme 主题类型
     */
    void setTheme(ThemeType theme);

    /**
     * @brief 获取当前主题类型
     * @return 当前主题类型
     */
    ThemeType currentTheme() const { return m_currentTheme; }

    /**
     * @brief 检查当前是否为深色主题
     * @return 如果是深色主题返回true，否则返回false
     */
    bool isDarkTheme() const { return m_currentTheme == Dark; }

    /**
     * @brief 获取主题相关的颜色
     * @param colorName 颜色名称
     * @return 对应的颜色值
     */
    QColor getColor(const QString& colorName) const;

    /**
     * @brief 加载并应用样式表
     * @param fileName 样式表文件名
     * @return 样式表内容
     */
    QString loadStyleSheet(const QString& fileName);

    /**
     * @brief 应用当前主题样式
     */
    void applyCurrentTheme();

    /**
     * @brief 更新特定控件的样式
     * @param widget 要更新的控件
     */
    void updateWidgetStyle(QWidget* widget);

signals:
    /**
     * @brief 主题变化信号
     * @param isDarkTheme 是否为深色主题
     */
    void themeChanged(bool isDarkTheme);

private:
    /**
     * @brief 私有构造函数，实现单例模式
     */
    ThemeManager(QObject* parent = nullptr);

    /**
     * @brief 初始化主题颜色映射
     */
    void initializeColorMaps();

    ThemeType m_currentTheme;  // 当前主题类型
    QMap<QString, QColor> m_lightColors;  // 浅色主题颜色映射
    QMap<QString, QColor> m_darkColors;    // 深色主题颜色映射
};

#endif // THEME_MANAGER_H
