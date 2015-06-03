// ObjectDetection.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SensorReader.h"
#include "CheckerBoardCalibration.h"
#include <iostream>

int _tmain(int argc, _TCHAR* argv[])
{
	SensorReader* reader = SensorReader::getInstance();
	cv::Mat* rgb;
	cv::Mat* depth;
	bool res = false;
	while (!res)
	{
		std::cout << "Trying to find a checker board" << std::endl;
		reader->getColorFrame(&rgb);
		reader->getDepthFrame(&depth);
		res = CheckerBoardCalibration::sendCorners(rgb, depth, false);
	}
	return 0;
}

