/***********************************************************************************
 2D Hue-Saturation Histogram feature 
 
 Created:
 TCL Research America
 Armin Kappeler, 09/18/2013

 Modification:
 
 ************************************************************************************/
#include "TCL_UView_HSColorHistogram.h"

using namespace cv;
using namespace std;

TCL_UView_HSColorHistogram::TCL_UView_HSColorHistogram(void)
{
	hbins = 16; 
	sbins = 8; 

}


TCL_UView_HSColorHistogram::TCL_UView_HSColorHistogram(int hb, int sb)
{
	hbins = hb;
	sbins = sb;	
}

TCL_UView_HSColorHistogram::TCL_UView_HSColorHistogram(FeatureSettings values)
{
	hbins = values.Bins1;
	sbins = values.Bins2;
	
}

TCL_UView_HSColorHistogram::~TCL_UView_HSColorHistogram(void)
{
}

Mat TCL_UView_HSColorHistogram::getFeatures(list<Mat> clip)
{
	Mat all_hist(hbins*sbins,(int)clip.size(),CV_32FC1); //float

	// main loop for clip frames
	int i=0;
	for (list<Mat>::iterator it = clip.begin(); it != clip.end(); it++)
	{
		Mat tmp = calculateHistogram(*it).t();	
		tmp.copyTo(all_hist.col(i));
		i++;
	}

	return all_hist;
}

Mat TCL_UView_HSColorHistogram::getFeature(cv::Mat img)
{
	//return calculateHistogram(img).t();
	return calculateHistogram(img);
}

Mat TCL_UView_HSColorHistogram::calculateHistogram(Mat src)
{
	Mat hsv;

    cvtColor(src, hsv, CV_BGR2HSV);

    // Quantize the hue and the saturation 
    int histSize[] = {hbins, sbins};
    // hue varies from 0 to 179, see cvtColor
    float hranges[] = { 0, 180 };
    // saturation varies from 0 (black-gray-white) to
    // 255 (pure spectrum color)
    float sranges[] = { 0, 256 };
    const float* ranges[] = { hranges, sranges };
    //MatND hist;
    Mat hist;
    // we compute the histogram from the 0-th and 1-st channels
    int channels[] = {0, 1};

    calcHist( &hsv, 1, channels, Mat(), // do not use mask
             hist, 2, histSize, ranges,
             true, // the histogram is uniform
             false );

	hist /= (src.cols*src.rows); //L1 normalization

	//return hist.reshape(1,hbins*sbins);
	return hist.reshape(1,1); //(nr channels, nr rows)
	
}


