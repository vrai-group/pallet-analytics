// oos-rgbd
//
// Copyright (C) 2016 Grottini Lab S.R.L.
//
// Authors: Rocco Pietrini <rocco.pietrini@grottinilab.com>
//


#include <QCoreApplication>
#include "process.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Process process;
    process.init();

    int stp = atoi(argv[1]);

    if(stp  == 1) {
        process.start(320,240,1);  //Setup
    }
    else
    {
            process.start(320,240,3); //Run
     }
    return a.exec();
}

