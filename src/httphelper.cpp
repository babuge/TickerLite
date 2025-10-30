
#include "httphelper.h"
#include "databasehelper.h"
#include <QDebug>

HttpHelper::HttpHelper(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

HttpHelper::~HttpHelper()
{
}

void HttpHelper::get(const QString &url)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));

    // 发送GET请求
    QNetworkReply *reply = m_networkManager->get(request);

    // 连接请求完成信号
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onRequestFinished(reply);
    });
}

void HttpHelper::onRequestFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network error:" << reply->errorString();
        emit requestFinished(reply->url().toString(), QByteArray(), true);
    } else {
        QByteArray data = reply->readAll();
        emit requestFinished(reply->url().toString(), data, false);
    }

    reply->deleteLater();
}

QString HttpHelper::parseSinaData(const QString &rawData, const QString &stockCode)
{
    // qDebug() << "Parsing data for stock code:" << stockCode << "Raw data:" << rawData;

    // 找到等号后面的内容
    int equalPos = rawData.indexOf("=");
    if (equalPos < 0) return QString();

    // 提取引号内的内容
    int startQuote = rawData.indexOf("\"", equalPos);
    int endQuote = rawData.indexOf("\"", startQuote + 1);

    if (startQuote < 0 || endQuote < 0) return QString();

    QString data = rawData.mid(startQuote + 1, endQuote - startQuote - 1);
    QStringList items = data.split("~");

    if (items.size() < 40) return QString();

    QString name = items[1];
    double price = items[3].toDouble();
    double prevClose = items[4].toDouble();
    double change = items[33].toDouble();
    double changePercent = items[34].toDouble();
    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    // 保存到SQLite数据库
    DatabaseHelper::instance().saveStockData(
        stockCode, name, price, prevClose, change, 
        changePercent, items[5].toDouble(), items[6], 
        items[7], items[8], timestamp
    );

    // 格式化结果
    // 返回：名称|当前价|涨跌额|涨跌幅|昨收价|开盘价|成交量|外盘|内盘
    return QString("%1|%2|%3|%4|%5|%6|%7|%8|%9|%10")
           .arg(name)
           .arg(price, 0, 'f', 2)
           .arg(change, 0, 'f', 2)
           .arg(changePercent, 0, 'f', 2)
           .arg(prevClose, 0, 'f', 2)
           .arg(items[5].toDouble(), 0, 'f', 2)  // 开盘价
           .arg(items[6])  // 成交量
           .arg(items[7])  // 外盘
           .arg(items[8])  // 内盘
           .arg(timestamp);
}

QJsonObject HttpHelper::parseJsonData(const QString &jsonData)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());
    return doc.object();
}
