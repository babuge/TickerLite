
#include "databasehelper.h"
#include <QDir>
#include <QStandardPaths>
#include <QVariantList>
#include <QVariantMap>

DatabaseHelper& DatabaseHelper::instance()
{
    static DatabaseHelper instance;
    return instance;
}

DatabaseHelper::DatabaseHelper(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
{
}

DatabaseHelper::~DatabaseHelper()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseHelper::initializeDatabase()
{
    if (m_initialized) {
        return true;
    }

    // 获取应用程序数据目录
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(dataPath);
    }

    // 数据库文件路径
    QString dbPath = dataPath + "/ticker_data.db";
    qDebug() << "dbpaht:" << dbPath;

    // 连接数据库
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qDebug() << "无法打开数据库:" << m_db.lastError().text();
        return false;
    }

    // 创建股票数据表
    QSqlQuery query;
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS stock_data ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "stock_code TEXT NOT NULL, "
        "name TEXT NOT NULL, "
        "price REAL NOT NULL, "
        "prev_close REAL NOT NULL, "
        "change REAL NOT NULL, "
        "change_percent REAL NOT NULL, "
        "open_price REAL NOT NULL, "
        "volume TEXT, "
        "outer_disc TEXT, "
        "inner_disc TEXT, "
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ")"
    );

    if (!success) {
        qDebug() << "创建表失败:" << query.lastError().text();
        return false;
    }

    // 创建股票代码索引，提高查询效率
    success = query.exec("CREATE INDEX IF NOT EXISTS idx_stock_code ON stock_data(stock_code)");
    if (!success) {
        qDebug() << "创建索引失败:" << query.lastError().text();
        return false;
    }

    // 创建时间戳索引，提高时间范围查询效率
    success = query.exec("CREATE INDEX IF NOT EXISTS idx_timestamp ON stock_data(timestamp)");
    if (!success) {
        qDebug() << "创建索引失败:" << query.lastError().text();
        return false;
    }

    m_initialized = true;
    return true;
}

bool DatabaseHelper::saveStockData(const QString &stockCode, const QString &name,
                                 double price, double prevClose, double change,
                                 double changePercent, double openPrice,
                                 const QString &volume, const QString &outerDisc,
                                 const QString &innerDisc,
                                 qint64 timestamp
)
{
    if (!m_initialized && !initializeDatabase()) {
        return false;
    }

    QSqlQuery query;
    query.prepare(
        "INSERT INTO stock_data (stock_code, name, price, prev_close, change, "
        "change_percent, open_price, volume, outer_disc, inner_disc, timestamp) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    );

    query.addBindValue(stockCode);
    query.addBindValue(name);
    query.addBindValue(price);
    query.addBindValue(prevClose);
    query.addBindValue(change);
    query.addBindValue(changePercent);
    query.addBindValue(openPrice);
    query.addBindValue(volume);
    query.addBindValue(outerDisc);
    query.addBindValue(innerDisc);
    query.addBindValue(timestamp);

    if (!query.exec()) {
        qDebug() << "保存股票数据失败:" << query.lastError().text();
        return false;
    }

    return true;
}

QList<QVariantMap> DatabaseHelper::getStockHistory(const QString &stockCode, 
                                                  const QDateTime &startTime, 
                                                  const QDateTime &endTime)
{
    QList<QVariantMap> result;

    if (!m_initialized && !initializeDatabase()) {
        return result;
    }

    QSqlQuery query;
    QString sql = "SELECT * FROM stock_data WHERE stock_code = ?";

    if (startTime.isValid()) {
        sql += " AND timestamp >= ?";
    }

    if (endTime.isValid()) {
        sql += " AND timestamp <= ?";
    }

    // 先获取最后100条数据（按时间戳降序）
    sql += " ORDER BY timestamp DESC LIMIT 150";

    query.prepare(sql);
    query.addBindValue(stockCode);

    if (startTime.isValid()) {
        query.addBindValue(startTime.toMSecsSinceEpoch());
    }

    if (endTime.isValid()) {
        query.addBindValue(endTime.toMSecsSinceEpoch());
    }

    if (!query.exec()) {
        qDebug() << "查询股票历史数据失败:" << query.lastError().text();
        return result;
    }

    // 先存储结果，然后按时间戳升序排列
    while (query.next()) {
        QVariantMap record;
        record["id"] = query.value("id");
        record["stock_code"] = query.value("stock_code");
        record["name"] = query.value("name");
        record["price"] = query.value("price");
        record["prev_close"] = query.value("prev_close");
        record["change"] = query.value("change");
        record["change_percent"] = query.value("change_percent");
        record["open_price"] = query.value("open_price");
        record["volume"] = query.value("volume");
        record["outer_disc"] = query.value("outer_disc");
        record["inner_disc"] = query.value("inner_disc");
        record["timestamp"] = query.value("timestamp");

        result.append(record);
    }

    // 按时间戳升序排列（从旧到新）
    std::sort(result.begin(), result.end(), [](const QVariantMap &a, const QVariantMap &b) {
        return a["timestamp"].toLongLong() < b["timestamp"].toLongLong();
    });

    return result;
}

QStringList DatabaseHelper::getAllStockCodes()
{
    QStringList result;

    if (!m_initialized && !initializeDatabase()) {
        return result;
    }

    QSqlQuery query;
    query.exec("SELECT DISTINCT stock_code FROM stock_data ORDER BY stock_code");

    while (query.next()) {
        result.append(query.value(0).toString());
    }

    return result;
}
