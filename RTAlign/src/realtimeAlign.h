/*
 * realtimeAlign.h
 *
 *  Created on: Oct 14, 2014
 *      Author: Armin Kappeler
 */

#ifndef SRC_REALTIMEALIGN_H_
#define SRC_REALTIMEALIGN_H_

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <string>
#include <random>

#include "cvplot/cvplot.h"
#include "Keyframe.h"

void realtimeAlign(Keyframes db_frames,char * videoFilename);
//void plot(string windowName,vector<float> data );


#endif /* SRC_REALTIMEALIGN_H_ */
