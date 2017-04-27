// oos-rgbd
//
// Copyright (C) 2016 Grottini Lab S.R.L.
//
// Authors: Rocco Pietrini <rocco.pietrini@grottinilab.com>
//


#ifndef PROCESS_H
#define PROCESS_H
#include <QtWidgets>
#include <QtCore>

#include "camera.h"
#include "sender.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <OpenNI.h>
#include <math.h>
#include <csignal>


class Process : public QObject
{
    Q_OBJECT
public:
    explicit Process(QObject *parent = 0);
    ~Process();
    void start(int x, int y, int mode);

    void init();

    void readJson();
private slots:
    void send(std::vector<int> dati);
    void FrameImageReady(cv::Mat FrameImage);
private:
    Camera *cam;
    openni::Array<openni::DeviceInfo> deviceList;
    Sender sender;
};

#endif // PROCESS_H
