#pragma once
class ObjectDetector
{
public:
	static ObjectDetector* getInstace();
	~ObjectDetector();
private:
	ObjectDetector();
	ObjectDetector* instance;
};

