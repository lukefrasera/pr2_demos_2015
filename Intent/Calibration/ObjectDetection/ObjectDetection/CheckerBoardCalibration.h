#pragma once
#include <opencv2\core.hpp>

class CheckerBoardCalibration
{
public:
	bool static sendCorners(cv::Mat* rgbIn, cv::Mat* depthIn, bool displayCorners);
	~CheckerBoardCalibration();
private:
	CheckerBoardCalibration();
};

