#pragma once
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

class ForegroundDetector
{
public:
	static ForegroundDetector* getInstance();
	~ForegroundDetector();
	void processFrame(cv::Mat* rgbIn, cv::Mat* depthIn, cv::Mat* fgMask);
private:
	int frameCount;
	bool stopCounting;
	ForegroundDetector();
	void processRGBFrame(cv::Mat* rgbIn, cv::Mat* fgMask);
	void processDepthFrame(cv::Mat* depthIn, cv::Mat* alignedFgMask, int rgbWidth, int rgbDepth);
	static ForegroundDetector* instance;
	cv::Ptr<cv::BackgroundSubtractor> rgbBS;
	cv::Ptr<cv::BackgroundSubtractor> depthBS1;
	cv::Ptr<cv::BackgroundSubtractor> depthBS2;
	
};

