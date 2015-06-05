#!/usr/bin/env python

import time
import zmq
from msgTypes.points_pb2 import Points, Point
import cv2 as cv
import numpy as np
import rospy as ros
import roslib
from sensor_msgs.msg import Image, PointCloud2, CameraInfo
import sensor_msgs.point_cloud2 as pc2
import cv_bridge

class Connection(object):
    """docstring for Connection"""
    def __init__(self, connection_string):
        super(Connection, self).__init__()
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REP)
        self.socket.bind(connection_string)


class ReceiveKinectImageInfo(object):
    def __init__(self):
        self.depth_image_received = False
        self.color_image_received = False
        self.bridge = cv_bridge.CvBridge()
        self.corners = (False, 0)
    def DepthImage(self, depth_image):
        image = self.bridge.imgmsg_to_cv2(depth_image)
        if self.depth_image_received == False and self.corners[0]:
            print "Depth", self.corners[0]
            self.depth_image = image
            self.depth_image_received = True

    def ColorImage(self, color_image):
        image = self.bridge.imgmsg_to_cv2(color_image)
        self.corners = cv.findChessboardCorners(image, (7,6))
        if self.color_image_received == False and self.corners[0]:
            print "Color", self.corners[0]
            self.color_image = color_image
            self.color_image_received = True


def main():
    '''Read point correspondences from kinects and computer transformation
    between the two spaces
    '''
    ros.init_node('Calibrate_Receive', anonymous=True)

    receiver = ReceiveKinectImageInfo()
    # print ros.get_param_names()
    # print ros.get_param("~depth_topic")
    ros.Subscriber(ros.get_param('~depth_topic'), Image, receiver.DepthImage)
    ros.Subscriber(ros.get_param('~color_topic'), Image, receiver.ColorImage)


    # Wait until images are received
    while not (receiver.color_image_received and receiver.depth_image_received):
        time.sleep(.05)
        if ros.is_shutdown():
            break
    ros.loginfo("Received Messages: %s", ros.get_param('~depth_topic'))

    # find Checkboard points
    bool, corners  = receiver.corners

    rgb_camera_info = ros.wait_for_message(ros.get_param('~color_camera_params'), CameraInfo, 3)
    # Project Points Into 3D
    K = rgb_camera_info.K
    depth_points = []
    for corner in corners:
        Z_c = receiver.depth_image[int(corner[0][0]+0.5)][int(corner[0][1]+0.5)] * 0.001
        X_c = (corner[0][0] - K[2]) * (1.0/K[0]) * Z_c
        Y_c = (corner[0][1] - K[5]) * (1.0/K[4]) * Z_c
        depth_points.append((X_c, Y_c, Z_c))
    # print depth_points



    # Get message from Kinect 2.0
    context = zmq.Context()
    socket = context.socket(zmq.REP)
    socket.bind("tcp://*:5555")

    message = socket.recv()
    ros.loginfo("Received Checker Points %d", len(message))
    points = Points()
    points.ParseFromString(message)

    # print points
    # Solve for Transformation Matrices
    X = np.zeros((4,1))
    A = np.zeros((len(corners), 4))
    B = np.zeros((len(corners), 1))
    T = np.zeros((4,4))
    for i, point in enumerate(depth_points):
        A[i,:] = [x for x in point]+ [1]
    # Initialize B for X
    for i, point in enumerate(points.points):
        B[i] = point.x

    cv.solve(A,B,X, cv.DECOMP_SVD)
    T[0,:] = np.transpose(X)
    for i, point in enumerate(points.points):
        B[i] = point.y

    cv.solve(A,B,X, cv.DECOMP_SVD)
    T[1,:] = np.transpose(X)
    for i, point in enumerate(points.points):
        B[i] = point.z

    cv.solve(A,B,X, cv.DECOMP_SVD)
    T[2,:] = np.transpose(X)
    T[3,:] = np.array([0,0,0,1])
    print T

    coord = np.mat(T) * np.mat(A[0,:]).transpose()
    print coord
    print points.points[0]


if __name__ == '__main__':
    main()
