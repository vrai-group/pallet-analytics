// oos-rgbd
//
// Copyright (C) 2016 Grottini Lab S.R.L.
//
// Authors: Rocco Pietrini <rocco.pietrini@grottinilab.com>


#include "camera.h"
#include <fstream>
#include <string>
#include <cmath>

using namespace std;
using namespace openni;
/// camera setup
Camera::Camera(const QString oni_filename)
{
    oniPath = oni_filename;
}

Camera::~Camera()
{
    depthStream.stop();
    colorStream.stop();
    depthStream.destroy();
    colorStream.destroy();
    device.close();
    OpenNI::shutdown();
}



/// camera init
int Camera::init(int res_x, int res_y, int fps)
{
    openni::Status rc = openni::STATUS_OK;
    qDebug() << "Start Initialization";

    rc = OpenNI::initialize();
    if (rc != openni::STATUS_OK)
    {
        qDebug() << OpenNI::getExtendedError();
    }

    rc = device.open(ANY_DEVICE);
    if (rc != openni::STATUS_OK)
      {
        qDebug() << OpenNI::getExtendedError();
      }
    if(rc != openni::STATUS_OK)
      {
        qDebug() << "Failed to open the device!";
        rc = device.open(oniPath.toLocal8Bit());
        if(rc != openni::STATUS_OK)
          {
            qDebug() << "Failed to open the registration!";
            OpenNI::shutdown();
            return -1;
          } else {
            qDebug() << QString("Playback from: %1 - ").arg(oniPath);
            device.getPlaybackControl()->setRepeatEnabled(true);
            setupDepthStream(res_x, res_y, fps);
            setupColorStream(res_x, res_y, fps);
          }
      } else {
        qDebug() << (QString("Device: %1").arg(device.getDeviceInfo().getName()));
        setupDepthStream(res_x, res_y, fps);
        setupColorStream(res_x, res_y, fps);
        rc = device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
      }

    qDebug() << "Sensor Initialized";

    return 0;
}

int Camera::getMode() const
{
    return mode;
}

void Camera::setMode(int value)
{
    mode = value;
}

/// camera stop
void Camera::stop(){
    depthStream.stop();
    colorStream.stop();
    depthStream.destroy();
    colorStream.destroy();
    device.close();
    OpenNI::shutdown();
}

void Camera::getFrame(){
    getDepthFrame(false).copyTo(depthMat);
    getColorFrame().copyTo(bgrMat);
    if (flipflag) {
        cv::flip(depthMat,depthMat,1);
        cv::flip(bgrMat,bgrMat,1);
      }
}
/// camera run
void Camera::run(){

    int framecount=0;
    while (1)
    {
        try
        {
            getFrame();
            framecount++;
            if (framecount % 60 == 0)  //Process only every 2 seconds (60 frames)
            {
                process();
                framecount=0;
            }
            if (mode==3)
            {
                if(pezzi_now.size()==0)
                    //pezzi_now=pezzi_old;
                {
                    for(int i=0;i<points.size();i++)
                        cv::putText(bgrMat,QString::number(pezzi_old[i]).toStdString(),cv::Point(points[i].x+5,points[i].y+5),cv::FONT_HERSHEY_PLAIN,1,CV_RGB(255,255,255),2);
                }else
                {
                    for(int i=0;i<points.size();i++)
                        cv::putText(bgrMat,QString::number(pezzi_now[i]).toStdString(),cv::Point(points[i].x+5,points[i].y+5),cv::FONT_HERSHEY_PLAIN,1,CV_RGB(255,255,255),2);
                }

                emit sigFrameImageReady(bgrMat.clone()) ;
            }
        }
        catch(std::exception &ex)
        {
            std::cerr << "getFrame()" << ex.what() << std::endl;
        }
    }
}

void Camera::outputError(const string &what)
{
    qDebug() << QString("%1 %2").arg(what.c_str()).arg(OpenNI::getExtendedError());
}
/// setup depth
void Camera::setupDepthStream(int res_x, int res_y, int fps)
{
    if(depthStream.create(device, openni::SENSOR_DEPTH) == openni::STATUS_OK)
    {
        depthStream.getVideoMode();

        depthMode.setPixelFormat(PIXEL_FORMAT_DEPTH_1_MM);
        depthMode.setResolution(res_x, res_y);
        depthMode.setFps(fps);

        depthStream.setVideoMode(depthMode);
        depthStream.setMirroringEnabled(false);

        if(depthStream.start() != openni::STATUS_OK)
        {
            outputError("Couldn't start depth stream");
            depthStream.destroy();
        } else {
            streams.push_back(&depthStream);
            qDebug() << QString("Depth Resolution: %1 x %2 - %3 fps").
                        arg(depthStream.getVideoMode().getResolutionX()).
                        arg(depthStream.getVideoMode().getResolutionY()).
                        arg(depthStream.getVideoMode().getFps());
            qDebug() << "Depth stream started...";
        }
    }
    else
    {
        outputError("Couldn't find depth stream");
    }
}
/// setup color
void Camera::setupColorStream(int res_x, int res_y, int fps)
{
    if(colorStream.create(device, SENSOR_COLOR) == openni::STATUS_OK)
    {
        colorStream.getVideoMode();

        colorMode.setPixelFormat(PIXEL_FORMAT_RGB888);
        colorMode.setResolution(res_x, res_y);
        colorMode.setFps(fps);

        colorStream.setVideoMode(colorMode);
        colorStream.setMirroringEnabled(false);

        if(colorStream.start() != openni::STATUS_OK)
        {
            outputError("Couldn't start color stream");
            colorStream.destroy();
        } else {
            streams.push_back(&colorStream);
        }
    }
    else
    {
        outputError("Couldn't find color stream");
    }
}
/// depth frame
DepthPixel *Camera::getDepthPixels()
{
    DepthPixel *depth_pixel;

    int ready_index = 0;
    Status rc = openni::OpenNI::waitForAnyStream(&streams[0],
            streams.size()/2,
            &ready_index);
    if (rc != STATUS_OK)
    {
        qDebug() << "Initialize failed" << OpenNI::getExtendedError();
        raise(SIGTERM);
    }
    if(ready_index == 0)
    {
        depthStream.readFrame(&depthFrameRef);

        if(depthFrameRef.isValid())
        {
            depth_pixel = (DepthPixel*)depthFrameRef.getData();
        }
    }
    return depth_pixel;
}
/// color frame
char *Camera::getColorPixels()
{
    char *color_pixel;

    int ready_index = 0;
#define SAMPLE_READ_WAIT_TIMEOUT 10000 //10000ms

    Status rc = openni::OpenNI::waitForAnyStream(&streams[1],
            streams.size()/2,
            &ready_index,
            SAMPLE_READ_WAIT_TIMEOUT);
    if (rc != STATUS_OK)
    {
        qDebug() << "Initialize failed" << OpenNI::getExtendedError();
        raise(SIGTERM);
    }
    if(ready_index == 0)
    {
        colorStream.readFrame(&colorFrameRef);

        if(colorFrameRef.isValid())
        {
            color_pixel = (char*)colorFrameRef.getData();
        }
    }

    return color_pixel;
}
/// point cloud map frame
void Camera::getRDepth()
{
    if (depthFrameRef.isValid()) {
        rDepth = cv::Mat::zeros(depthMat.rows, depthMat.cols, CV_32FC3);
        int cols = depthFrameRef.getWidth();
        int rows = depthFrameRef.getHeight();
        float worldX, worldY, worldZ;

        for( int y = 0; y < rows; y++ )
        {
            for (int x = 0; x < cols; x++)
            {
                openni::CoordinateConverter::convertDepthToWorld(depthStream,
                                                                 x,
                                                                 y,
                                                                 depthMat.at<unsigned short>(y, x),
                                                                 &worldX,
                                                                 &worldY,
                                                                 &worldZ);

                if (depthMat.at<unsigned short>(y, x) == 0) // not valid
                    rDepth.at<cv::Point3f>(y, x) = cv::Point3f(0, 0, 0);
                else
                {
                    rDepth.at<cv::Point3f>(y, x) = cv::Point3f(worldX,worldY,worldZ);
                    // from mm to meters

                }

            }
        }
    }
}


///from depth to real
void Camera::getRealPoints(){

    float realX,realY,realZ;

    oldpoints=points;

    for (unsigned int i=0;i<points.size();i++){

     openni::CoordinateConverter::convertDepthToWorld(depthStream,points[i].x,points[i].y,hp1[i],&realX,&realY,&realZ);

     if(realpoints.size()==0)
        realpoints.push_back(Realpoints());

     realpoints[i].x=realX;
     realpoints[i].y=realY;
    }

}

/// draw circles over the frame
void Camera:: drawCircles(cv::Mat FrameImage){

    for(unsigned int i=0; i<points.size(); i++)
    {
     cv::circle(FrameImage,cv::Point(points[i].x,points[i].y), 4, cv::Scalar(0,255,0), -1);
    }
}

/// draw initials circles over the frame
void Camera:: drawInitialCircles(cv::Mat FrameImage){

    for(unsigned int i=0; i<oldpoints.size(); i++)
    {
     cv::circle(FrameImage,cv::Point(oldpoints[i].x,oldpoints[i].y), 4, cv::Scalar(0,0,255), -1);
    }
}

///from real to depth
void Camera::updatePoints(){

    int cols = depthFrameRef.getWidth();
    int rows = depthFrameRef.getHeight();
    int x,y;
    DepthPixel  z=0;

    for(int i=0; i<points.size(); i++)
    {
        for(float j=3000; j>0; j--)  //scandisco tutte le altezze possibili fino a 3 metri
        {
        openni::CoordinateConverter::convertWorldToDepth(depthStream, realpoints[i].x, realpoints[i].y,j, &x, &y, &z);
               if(x>=0 && x<=cols && y>=0 && y<=rows)
                {
                    if(z==static_cast<int>(rDepth.at<cv::Point3f>(y, x).z))
                    {
                        points[i].x=x;
                        points[i].y=y;
                        break;
                    }
                }

         }
    }

}

cv::Mat Camera::getDepthFrame(const bool convert_to_8bit)
{
    cv::Mat image(depthStream.getVideoMode().getResolutionY(),
                  depthStream.getVideoMode().getResolutionX(),
                  CV_16UC1,
                  getDepthPixels());
    if(convert_to_8bit)
    {
        image.convertTo(image,
                        CV_8U,
                        255.0 / (double)depthStream.getMaxPixelValue());
    }
    return image;
}
/// color Mat
cv::Mat Camera::getColorFrame()
{
    cv::Mat image(colorStream.getVideoMode().getResolutionY(),
                  colorStream.getVideoMode().getResolutionX(),
                  CV_8UC3,
                  getColorPixels());

    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
    return image;
}

cv::Mat Camera::getBgrMat() {
    return bgrMat;
}

/// process
void Camera::process() {

    int cols = depthFrameRef.getWidth();
    int rows = depthFrameRef.getHeight();
    double totframe =cv::sqrt(cols*cols+rows*rows);

    string filename;     //Fase di run

    switch (getMode())
    {
    case 1: filename="hpoints0.csv"; break;  //fase di setup floor
    case 2: filename="hpoints1.csv"; break;  //fase di setup primo livello
    }
    getRDepth();

    if (getMode()==3)              //Fase di run
    {
        for(unsigned int i=0; i<points.size(); i++)
        {
            updatePoints();
            cv::Vec3f realPoint = rDepth.at<cv::Vec3f>(points[i].y, points[i].x);
            double pezzicalc=(hp0[i]-realPoint[2])/(hp0[i]-hp1[i]);
            qDebug() << "x=" << points[i].x << "y=" << points[i].y << "Pavimento:" << hp0[i] << "Altezza Pacco:" << hp1[i] << "Altezza pila:"
                     << realPoint[2] << "Pezzi:" << QString::number(pezzicalc,'f',1);
            int pezzi=round(pezzicalc);
            if (pezzi<0 || pezzi >10) pezzi=0;
            pezzi_now.push_back(pezzi);
            cv::Point2f diff =  points[i]-oldpoints[i];
            double res = cv::sqrt(diff.x*diff.x+diff.y*diff.y);
            qDebug()<<"scostamento "<<res<<" pixel pari al "<<(res/totframe)*100<<"% del frame";
        }
        qDebug()<< "now:"<<pezzi_now[0]<<pezzi_now[1]<<pezzi_now[2]<<pezzi_now[3]<<pezzi_now[4]<<pezzi_now[5];
        qDebug()<<"old:"<<pezzi_old[0]<<pezzi_old[1]<<pezzi_old[2]<<pezzi_old[3]<<pezzi_old[4]<<pezzi_old[5];
        qDebug()<<"...............";
        if (pezzi_now!=pezzi_old)
        {
            emit signMeasure(pezzi_now);
            qDebug()<<pezzi_now.size() << pezzi_old.size();
        }
    }
    else    //fase di setup
    {
        ofstream out(filename);

        for(unsigned int i=0; i<points.size(); i++)
        {
            cv::Vec3f realPoint = rDepth.at<cv::Vec3f>(points[i].y, points[i].x);
            qDebug() << "x=" << points[i].x << "y=" << points[i].y << "altezza=" << realPoint[2];
            out << realPoint[2] << endl;
        }
        out.close();

    }
    pezzi_old=pezzi_now;
    pezzi_now.clear();

}

void Camera::loadValues()
{
    ifstream h0,h1;
    h0.open("hpoints0.csv");
    h1.open("hpoints1.csv");


    int a;
    while (h0 >> a)
    {
        hp0.push_back(a);
    }

    while (h1 >> a)
    {
        hp1.push_back(a);
    }
    h0.close();
    h1.close();
}



void Camera::loadPoints()
{
    vector<int> coordinate;

    ifstream fPoints;
    fPoints.open("points.csv");

    string line;
    while (getline(fPoints,line))  //line by line
    {
        int temp;
        stringstream ss;
        ss.str(line);
        while (ss >> temp)         //Split x and y
        {
            coordinate.push_back(temp);
        }
        cv::Point temp_punto;
        temp_punto.x = coordinate[0];
        temp_punto.y = coordinate[1];
        points.push_back(temp_punto);
        coordinate.clear();
        pezzi_old.push_back(0);
    }
    fPoints.close();
}

bool Camera::getFlipflag() const
{
    return flipflag;
}

void Camera::setFlipflag(bool value)
{
    flipflag = value;
}


