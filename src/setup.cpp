// oos-rgbd
//
// Copyright (C) 2016 Grottini Lab S.R.L.
//
// Authors: Rocco Pietrini <rocco.pietrini@grottinilab.com>
//

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <fstream>

using namespace std;
using namespace cv;

Mat img,cpy;

std::vector<cv::Point> points;

void mouseEvent(int evt, int x, int y, int flags, void* param)
{

    cv::Point punto;

    //Point selection, left click
    if(evt==CV_EVENT_LBUTTONDOWN)
    {
            punto.x = x;
            punto.y = y;
            cv::circle(cpy,punto,5,cv::Scalar( 0, 0, 255 ), 1, 8,0);
            cv::imshow("SETUP", cpy);
            points.push_back(punto);
    }

    //Stop, right click and save points
    if(evt==CV_EVENT_RBUTTONDOWN)
    {
         ofstream out("points.csv");
         for(unsigned int i=0; i<points.size(); i++) {
             out << points[i].x << " " << points[i].y << endl;
         }
         out.close();
         cv::destroyWindow("SETUP");
    }
}


void setup(char* pathScena)
{
    img = cv::imread(pathScena);
    if(img.empty()) {
        printf("Image not found %s!\n", pathScena);
        exit(1);
    }
    cpy = img.clone();
    cv::namedWindow("SETUP", CV_WINDOW_NORMAL);
    cv::setMouseCallback("SETUP", mouseEvent, 0);
    cv::imshow("SETUP", img);
    cv::waitKey();
}
