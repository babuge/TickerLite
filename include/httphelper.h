
#ifndef HTTPHELPER_H
#define HTTPHELPER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>

class HttpHelper : public QObject
{
    Q_OBJECT

public:
    explicit HttpHelper(QObject *parent = nullptr);
    ~HttpHelper();

    // 发送GET请求
    void get(const QString &url);

    // 解析新浪财经数据
    static QString parseSinaData(const QString &rawData, const QString &stockCode);

    // 解析JSON数据
    static QJsonObject parseJsonData(const QString &jsonData);

signals:
    // 请求完成信号
    void requestFinished(const QString &url, const QByteArray &data, bool error);

private slots:
    void onRequestFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_networkManager;
};

#endif // HTTPHELPER_H
