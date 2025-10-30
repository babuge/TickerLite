
#ifndef DATABASEHELPER_H
#define DATABASEHELPER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QDateTime>
#include <QDebug>

class DatabaseHelper : public QObject
{
    Q_OBJECT

public:
    static DatabaseHelper& instance();

    // 初始化数据库
    bool initializeDatabase();

    // 保存股票数据
    bool saveStockData(const QString &stockCode, const QString &name, 
                      double price, double prevClose, double change, 
                      double changePercent, double openPrice, 
                      const QString &volume, const QString &outerDisc,
                      const QString &innerDisc, qint64 timestamp);

    // 获取股票历史数据
    QList<QVariantMap> getStockHistory(const QString &stockCode, 
                                     const QDateTime &startTime = QDateTime(), 
                                     const QDateTime &endTime = QDateTime());

    // 获取所有股票代码
    QStringList getAllStockCodes();

private:
    explicit DatabaseHelper(QObject *parent = nullptr);
    ~DatabaseHelper();

    // 禁止拷贝构造和赋值
    DatabaseHelper(const DatabaseHelper&) = delete;
    DatabaseHelper& operator=(const DatabaseHelper&) = delete;

    QSqlDatabase m_db;
    bool m_initialized;
};

#endif // DATABASEHELPER_H
