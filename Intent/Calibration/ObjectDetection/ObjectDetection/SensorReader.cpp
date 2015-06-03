#include "stdafx.h"
#include "SensorReader.h"

SensorReader* SensorReader::instance = NULL;

SensorReader::SensorReader()
{
	HRESULT res;
	res = GetDefaultKinectSensor(&this->sensor);
	res = this->sensor->Open();
	IColorFrameSource* colorSource;
	IFrameDescription* desc;
	res = this->sensor->get_ColorFrameSource(&colorSource);
	res = colorSource->OpenReader(&this->colorFrameReader);
	IDepthFrameSource* depthSource;
	res = this->sensor->get_DepthFrameSource(&depthSource);
	res = depthSource->OpenReader(&this->depthFrameReader);
	IBodyFrameSource* bodySource;
	res = this->sensor->get_BodyFrameSource(&bodySource);
	res = bodySource->OpenReader(&this->bodyFrameReader);
	
	ColorImageFormat imageFormat;
	UINT bytesPerPixel;
	IColorFrame* colorFrame;
	do {
		res = this->colorFrameReader->AcquireLatestFrame(&colorFrame);
	} while (res == E_PENDING);
	res = colorFrame->get_FrameDescription(&desc);
	res = colorFrame->get_RawColorImageFormat(&imageFormat);
	desc->get_BytesPerPixel(&bytesPerPixel);
	res = desc->get_Width(&this->colorWidth);
	res = desc->get_Height(&this->colorHeight);
	IDepthFrame* depthFrame;
	do 
	{
		res = this->depthFrameReader->AcquireLatestFrame(&depthFrame);
	} while (res == E_PENDING);
	res = depthFrame->get_FrameDescription(&desc);
	res = desc->get_Width(&this->depthWidth);
	res = desc->get_Height(&this->depthHeight);
	colorFrame->Release();
	depthFrame->Release();
	colorSource->Release();
	depthSource->Release();
	bodySource->Release();
	desc->Release();
}

void SensorReader::getColorFrame(cv::Mat** in)
{
	IColorFrame* frame;
	HRESULT res;
	do 
	{
		res = this->colorFrameReader->AcquireLatestFrame(&frame);
	} while (res == E_PENDING);
	
	*in = new cv::Mat(this->colorHeight, this->colorWidth, CV_8UC4);
	frame->CopyConvertedFrameDataToArray(this->colorHeight * this->colorWidth * 4, reinterpret_cast<BYTE*>((*in)->data), 
		ColorImageFormat_Bgra);
	frame->Release();
}

void SensorReader::getDepthFrame(cv::Mat** in) 
{
	IDepthFrame* frame;
	HRESULT res;
	do
	{
		res = this->depthFrameReader->AcquireLatestFrame(&frame);
	} while (res == E_PENDING);
	*in = new cv::Mat(this->depthHeight, this->depthWidth, CV_16UC1);
	frame->CopyFrameDataToArray(this->depthWidth * this->depthHeight, reinterpret_cast<UINT16*>((*in)->data));
	frame->Release();
}

void SensorReader::getBodies(IBody*** in)
{
	IBodyFrame* frame;
	HRESULT res;
	do
	{
		res = this->bodyFrameReader->AcquireLatestFrame(&frame);
	} while (res == E_PENDING);
	*in = new IBody*[BODY_COUNT];
	frame->GetAndRefreshBodyData(BODY_COUNT, *in);
	frame->Release();
}

void SensorReader::getActiveBody(IBody** in) {
	IBody** bodies;
	SensorReader::getBodies(&bodies);
	for (int i = 0; i < BODY_COUNT; i++)
	{
		BOOLEAN tracked;
		bodies[i]->get_IsTracked(&tracked);
		if (tracked) 
		{
			*in = bodies[i];
			return;
		}
	}
}

ICoordinateMapper* SensorReader::getMapper()
{
	ICoordinateMapper* ret;
	this->sensor->get_CoordinateMapper(&ret);
	return ret;
}

SensorReader::~SensorReader()
{
	this->sensor->Close();
	delete[] this->instance;
}

SensorReader* SensorReader::getInstance() 
{
	if (instance == NULL) 
	{
		instance = new SensorReader();
	}
	return instance;
}