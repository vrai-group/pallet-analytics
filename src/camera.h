// oos-rgbd
//
// Copyright (C) 2016 Grottini Lab S.R.L.
//
// Authors: Rocco Pietrini <rocco.pietrini@grottinilab.com>
//


#ifndef CAMERA_H
#define CAMERA_H
#define CHECK_RC

#include <QtCore>
#include <QList>
#include <QQueue>

#include <iostream>

#include <unistd.h>
#include <OpenNI.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <csignal>

using namespace std;
using namespace openni;

class Camera : public QThread
{
    Q_OBJECT
public:
    Camera(const QString oni_filename);
    ~Camera();
    void drawCircles(cv::Mat FrameImage);
    void drawInitialCircles(cv::Mat FrameImage);
    void getRealPoints();
    void updatePoints();
    int init(int res_x, int res_y, int fps);
    void stop();
    void run();
    void getFrame();
    void process();
    void outputError(const string &what);
    void setupDepthStream(int res_x, int res_y, int fps);
    void setupColorStream(int res_x, int res_y, int fps);
    cv::Mat getDepthFrame(const bool convert_to_8bit);
    cv::Mat getColorFrame();
    void getRDepth();
    DepthPixel *getDepthPixels();
    char *getColorPixels();
    cv::Mat getBgrMat();
    void setResolution(int x, int y);
    void setFps(int frameFps);

    // variable

    int getMode() const;
    void setMode(int value);
    void loadValues();
    void loadPoints();
    bool getFlipflag() const;
    void setFlipflag(bool value);

signals:
    void signMeasure(std::vector<int> pezzi_now);
    void sigFrameImageReady(cv::Mat);

private:
    QString oniPath;
    openni::Device device;

    VideoMode depthMode;
    VideoMode colorMode;

    VideoFrameRef colorFrameRef;
    VideoFrameRef depthFrameRef;

    VideoStream depthStream;
    VideoStream colorStream;
    std::vector < openni::VideoStream * >streams;

    cv::Mat depthMat;

    cv::Mat rDepth;

    cv::Mat depthMatFilter;
    cv::Mat bgrMat;

    cv::Mat debugMat;

    vector<int> hp0,hp1,pezzi_old,pezzi_now;

    bool flipflag;

    int mode;


    vector<cv::Point> points;
    vector<cv::Point> oldpoints;

    struct Realpoints{
      float x,y;
    };

    vector<Realpoints> realpoints;




};

#endif // CAMERA_H
