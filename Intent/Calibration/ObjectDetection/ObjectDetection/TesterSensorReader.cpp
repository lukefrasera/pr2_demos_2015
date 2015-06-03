// ObjectDetection.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SensorReader.h"
#include "opencv2\highgui.hpp"


int _tmain(int argc, _TCHAR* argv[])
{
	SensorReader* reader = SensorReader::getInstance();
	cv::Mat* rgb;
	cv::Mat adjMap;
	cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);
	while (true) 
	{
		reader->getDepthFrame(&rgb);
		double min;
		double max;
		cv::minMaxIdx(*rgb, &min, &max);
		cv::convertScaleAbs(*rgb, adjMap, 255 / max);
		cv::imshow("Display window", adjMap);
		delete rgb;
		cv::waitKey(1);
	}
	return 0;
}

