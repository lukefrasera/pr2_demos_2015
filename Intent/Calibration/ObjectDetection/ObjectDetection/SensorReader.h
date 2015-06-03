#pragma once
#include <Kinect.h>
#include <opencv2\core.hpp>

class SensorReader
{
public:
	static SensorReader* getInstance();
	~SensorReader();
	void getColorFrame(cv::Mat** in);
	void getDepthFrame(cv::Mat** in);
	void getBodies(IBody*** in);
	void getActiveBody(IBody** in);
	ICoordinateMapper* getMapper();
private:
	SensorReader();
	static SensorReader* instance;
	IKinectSensor* sensor;
	IColorFrameReader* colorFrameReader;
	IDepthFrameReader* depthFrameReader;
	IBodyFrameReader* bodyFrameReader;
	int colorWidth, colorHeight, depthWidth, depthHeight;
};

