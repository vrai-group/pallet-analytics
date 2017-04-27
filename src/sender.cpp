// oos-rgbd
//
// Copyright (C) 2016 Grottini Lab S.R.L.
//
// Authors: Rocco Pietrini <rocco.pietrini@grottinilab.com>
//

#include "sender.h"
#include <csignal>
#include <fstream>
#include <string>
#include <iostream>


Sender::Sender(QObject *parent) :
    QObject(parent)
{
}

void Sender::send()
{
    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/x-www-form-urlencoded");

    manager->post(request, QString("data="+messages).toLocal8Bit());
    qDebug()<< messages;
}

void Sender::setMsg(std::vector<int> h)
{
    messages = "";

    for(unsigned int i=0;i<h.size();i++)
    {
        messages += QString::number(h[i]) + "_";
    }

    messages.chop(1);  //remove the last underscore
}

void Sender::replyFinished(QNetworkReply *reply)
{
    QString replyContent;
    if(reply->error())
    {
        qDebug() << "* * * * Host Error!!! * * * *";
        qDebug() << reply->errorString();
        if (messages != "") {
            outfile.open(QString("fail-" + QDate::currentDate().toString("yyyy-MM-dd") + ".log").toStdString(), std::ios_base::app);
            outfile << url.toStdString() << "data=" << messages.toStdString() << "\n";
            outfile.close();
        }
    }
    else
    {
        replyContent=QString(reply->readAll());
        // Server replies with a page containing " 1 " in case of error (empty in case of success), hence 3 characters.
        // This check is necessary to avoid page redirect (for example simcard traffic expired), this condition is
        // not handled as a network error, but surely it will produce a reply longer than 3 characters

        if (messages !="" && replyContent.size()>2){
            qDebug() << "* * * * Reply Server Error!!! * * * *";
            outfile.open(QString("fail-" + QDate::currentDate().toString("yyyy-MM-dd") + ".log").toStdString(), std::ios_base::app);
            outfile << url.toStdString() << "data=" << messages.toStdString() << "\n";
            outfile.close();
        }
    }

    reply->close();
    reply->manager()->deleteLater();
    reply->deleteLater();
}

QString Sender::getUrl() const
{
    return url;
}

void Sender::setUrl(const QString &value)
{
    url = value;
}
