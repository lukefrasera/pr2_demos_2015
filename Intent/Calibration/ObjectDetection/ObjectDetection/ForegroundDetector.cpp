#include "stdafx.h"
#include "ForegroundDetector.h"
#include "SensorReader.h"
#include <iostream>
#include <opencv2\features2d.hpp>

ForegroundDetector* ForegroundDetector::instance = NULL;

void ForegroundDetector::processFrame(cv::Mat* rgbIn, cv::Mat* depthIn, cv::Mat* fgMask)
{
	cv::Mat rgbMask, depthMask;
	*fgMask = cv::Mat(rgbIn->rows, rgbIn->cols, CV_8UC1);
	ForegroundDetector::processRGBFrame(rgbIn, &rgbMask);
	ForegroundDetector::processDepthFrame(depthIn, &depthMask, rgbIn->cols, rgbIn->rows);
	cv::bitwise_and(rgbMask, depthMask, *fgMask);
	cv::threshold(*fgMask, *fgMask, 50, 255, cv::THRESH_BINARY);
	cv::medianBlur(*fgMask, *fgMask, 7);
}

void ForegroundDetector::processRGBFrame(cv::Mat* rgbIn, cv::Mat* fgMask)
{
	if (fgMask == NULL)
		fgMask = new cv::Mat();
	if (this->stopCounting)
		this->rgbBS->apply(*rgbIn, *fgMask, 0.0000001);
	else
	{
		this->frameCount++;
		if (this->frameCount > 200)
			this->stopCounting = true;
		this->rgbBS->apply(*rgbIn, *fgMask, 0.1);
	}
}

void ForegroundDetector::processDepthFrame(cv::Mat* depthIn, cv::Mat* fgMask, int rgbWidth, int rgbHeight)
{
	cv::Mat fgMask1, fgMask2, fgMaskOriginal;
	cv::Mat depthModified1(depthIn->rows, depthIn->cols, CV_8UC1);
	cv::Mat depthModified2(depthIn->rows, depthIn->cols, CV_8UC1);
	*fgMask = cv::Mat(rgbHeight, rgbWidth, CV_8UC1);
	for (int i = 0; i < depthIn->rows * depthIn->cols; i++)
	{
		BYTE convertedPixel1, convertedPixel2;
		ushort pixel = depthIn->at<ushort>(i);
		convertedPixel1 = (BYTE)(pixel & 0x00ff);
		convertedPixel2 = (BYTE)((pixel & 0xff00)>>8);
		depthModified1.at<BYTE>(i) = convertedPixel1;
		depthModified2.at<BYTE>(i) = convertedPixel2;
	}
	if (this->stopCounting) {
		this->depthBS1->apply(depthModified1, fgMask1, 0.0000001);
		this->depthBS2->apply(depthModified2, fgMask2, 0.0000001);
	}
	else
	{
		this->frameCount++;
		if (this->frameCount > 200)
			this->stopCounting = true;
		this->depthBS1->apply(depthModified1, fgMask1, 0.1);
		this->depthBS2->apply(depthModified2, fgMask2, 0.1);
	}
	cv::bitwise_or(fgMask1, fgMask2, fgMaskOriginal);
	
	ICoordinateMapper* mapper = SensorReader::getInstance()->getMapper();
	DepthSpacePoint* points = new DepthSpacePoint[rgbWidth * rgbHeight];
	mapper->MapColorFrameToDepthSpace(depthIn->rows * depthIn->cols,
		reinterpret_cast<UINT16*>(depthIn->data), rgbWidth * rgbHeight, points);
	for (int i = 0; i < fgMask->rows * fgMask->cols; i++)
	{
		int x = (int)(points[i].X + 0.5f);
		int y = (int)(points[i].Y + 0.5f);

		if (x >= 0 && x < depthIn->cols && y >= 0 && y < depthIn->rows 
			&& fgMaskOriginal.at<BYTE>(cv::Point(x, y)) != 0)
		{
			fgMask->at<BYTE>(i) = fgMaskOriginal.at<BYTE>(cv::Point(x, y));
		}
	}
	delete[] points;
}

ForegroundDetector* ForegroundDetector::getInstance()
{
	if (instance == NULL)
		instance = new ForegroundDetector();
	return instance;
}

ForegroundDetector::ForegroundDetector()
{
	this->rgbBS = cv::createBackgroundSubtractorMOG2();
	this->depthBS1 = cv::createBackgroundSubtractorMOG2(500, 32, false);
	this->depthBS2 = cv::createBackgroundSubtractorMOG2(500, 1, false);
	this->frameCount = 0;
	this->stopCounting = false;
}

ForegroundDetector::~ForegroundDetector()
{
}
