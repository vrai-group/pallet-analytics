// oos-rgbd
//
// Copyright (C) 2016 Grottini Lab S.R.L.
//
// Authors: Rocco Pietrini <rocco.pietrini@grottinilab.com>
//

#ifndef SENDER_H
#define SENDER_H
#include <QObject>
#include <QDebug>

#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <fstream>

class Sender: public QObject
{
    Q_OBJECT
public:
    explicit Sender(QObject *parent = 0);

    void send();

    void setMsg(std::vector<int> h);

    QString getUrl() const;

    void setUrl(const QString &value);

public slots:
    void replyFinished(QNetworkReply *reply);
private:
    QNetworkAccessManager *manager;

    QUrlQuery postData;
    QString url;
    QString messages;

    std::ofstream outfile;
};

#endif // SENDER_H
