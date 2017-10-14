// oos-rgbd
//
// Copyright (C) 2016 Grottini Lab S.R.L.
//
// Authors: Rocco Pietrini <rocco.pietrini@grottinilab.com>
//


#include "process.h"
#include "setup.h"


Process::Process(QObject *parent) : QObject(parent)
{

}

void Process::init() {
    openni::OpenNI::initialize();
    openni::OpenNI::enumerateDevices(&deviceList);

    qDebug() << "There are" << deviceList.getSize() << "devices on this system.";

    for(int i=0; i<deviceList.getSize(); ++i) {
        const DeviceInfo& rDevInfo = deviceList[i];
        qDebug() << rDevInfo.getName() << " by " << rDevInfo.getVendor();
        qDebug() << "PID" << rDevInfo.getUsbProductId();
        qDebug() << "VID" << rDevInfo.getUsbVendorId();
        qDebug() << "URI" << rDevInfo.getUri();
    }
    openni::OpenNI::shutdown();
    readJson();   //Load configuration parameters
}

Process::~Process()
{
    if (!cam)
    {
        delete cam;
    }
}

void Process::send(std::vector<int> dati)
{
    sender.setMsg(dati);
    sender.send();
}

void Process::start(int x, int y, int mode)
{
    cam = new Camera("../Captured.oni");
    qRegisterMetaType<std::vector<int>>("std::vector<int>");
    connect(cam, SIGNAL(signMeasure(std::vector<int>)), this, SLOT(send(std::vector<int>)));
    cam->setMode(mode);

    if (cam->init(x, y, 30) ==  0)
    {
        if (mode==1)
        {
            for(unsigned int i=0;i<5;i++)
            cam->getFrame();
            cv::imwrite(QString("modello.png").toLocal8Bit().data(),cam->getBgrMat());
            setup("modello.png");      //Point Selection
            cam->loadPoints();
            cam->getFrame();
            cam->process();
            cout << "Metti uno strato di prodotti sul pallet e premi un tasto" << endl;
            cin.get();
            cam->setMode(2);
            cam->getFrame();
            cam->process();
            cout << "Setup completato" << endl;
        }
        if (mode==3)
        {
            qRegisterMetaType<cv::Mat>("cv::Mat");
            connect(cam, SIGNAL(sigFrameImageReady(cv::Mat)),this,SLOT(FrameImageReady(cv::Mat)));
            cam->loadPoints();
            cam->loadValues();
            cam->getRealPoints();
            cam->start();
        }
    }
    else
    {
        // device not found or registration not found
        if (!cam)
            delete cam;
        raise(SIGTERM);
    }
}

void Process::readJson()
{
    QString val;
    QFile file;
    file.setFileName("../config.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
#if (QT_VERSION > QT_VERSION_CHECK(5, 4, 0))   //Beaviour of qWarning changed
    qWarning().noquote() << val;
#else
    qWarning() << val;
#endif
    QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());

    sender.setUrl(d.object().value(QString("param")).toObject()["url"].toString());
    cam->setFlipflag(d.object().value(QString("param")).toObject()["flip"].toBool());
}
void Process::FrameImageReady(cv::Mat FrameImage)
{

    cam->drawCircles(FrameImage);
    cv::imshow("bgr", FrameImage);
}
