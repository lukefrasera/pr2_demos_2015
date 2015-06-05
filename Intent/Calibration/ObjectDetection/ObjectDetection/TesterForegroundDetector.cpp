// ObjectDetection.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SensorReader.h"
#include "opencv2\highgui.hpp"
#include "ForegroundDetector.h"


int _tmain(int argc, _TCHAR* argv[])
{
	SensorReader* reader = SensorReader::getInstance();
	ForegroundDetector* fgDetector = ForegroundDetector::getInstance();
	cv::Mat* rgb, *depth;
	cv::Mat fgMask;
	cv::namedWindow("depth", cv::WINDOW_NORMAL);
	cv::namedWindow("color", cv::WINDOW_NORMAL);
	cv::namedWindow("Foreground", cv::WINDOW_NORMAL);
	while (true)
	{
		reader->getColorFrame(&rgb);
		reader->getDepthFrame(&depth);
		fgDetector->processFrame(rgb, depth, &fgMask);
		cv::imshow("depth", *depth);
		cv::imshow("color", *rgb);
		cv::imshow("Foreground", fgMask);
		delete rgb;
		delete depth;
		cv::waitKey(1);
	}
	return 0;
}

