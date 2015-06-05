#include "stdafx.h"
#include "CheckerBoardCalibration.h"
#include <opencv2\calib3d.hpp>
#include <opencv2\highgui.hpp>
#include "points.pb.h"
#include "SensorReader.h"
#include <iostream>

bool CheckerBoardCalibration::sendCorners(cv::Mat* rgbIn, cv::Mat* depthIn, bool displayCorners)
{
	cv::Size board(6, 7);
	cv::Mat corners;
	if (!cv::findChessboardCorners(*rgbIn, board, corners))
		return false;
	if (displayCorners)
		cv::drawChessboardCorners(*rgbIn, board, corners, true);
	ICoordinateMapper* mapper = SensorReader::getInstance()->getMapper();
	CameraSpacePoint* points = new CameraSpacePoint[rgbIn->rows * rgbIn->cols];
	mapper->MapColorFrameToCameraSpace(depthIn->rows * depthIn->cols, 
		reinterpret_cast<UINT16*>(depthIn->data), rgbIn->rows * rgbIn->cols, points);
	Points message;
	for (int col = 0; col < 6; col++)
	{
		for (int row = 6; row >= 0; row--)
		{
			int i = row * 6 + col;
			std::cout << corners.at<cv::Point2f>(i, 0).x << ", " << corners.at<cv::Point2f>(i, 0).y << " is mapped to: ";
			int colorX = (int)(corners.at<cv::Point2f>(i, 0).x + 0.5f);
			int colorY = (int)(corners.at<cv::Point2f>(i, 0).y + 0.5f);
			int index = (colorY * rgbIn->cols) + colorX;
			std::cout << points[index].X << ", " << points[index].Y << ", " << points[index].Z << std::endl;
			Point* point = message.add_points();
			point->set_x(points[index].X);
			point->set_y(points[index].Y);
			point->set_z(points[index].Z);
		}
	}
	
	std::string contents = message.SerializeAsString();
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REQ);
	socket.connect("tcp://10.68.255.6:5555");
	zmq::message_t request(contents.size());
	memcpy(request.data(), contents.c_str(), contents.size());
	std::cout << "Now sending data to server" << std::endl;
	socket.send(request);
	delete[] points;
	return true;
}

CheckerBoardCalibration::CheckerBoardCalibration()
{
}

CheckerBoardCalibration::~CheckerBoardCalibration()
{
}
