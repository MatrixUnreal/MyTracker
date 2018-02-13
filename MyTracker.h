#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include <Windows.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <iterator>
#include <time.h>
#include <memory>

#include <opencv2/core/utility.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include <cstring>
#include <ctime>
#include <conio.h>
#include "samples_utility.hpp"

#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>

#ifdef HAVE_OPENCV
#include <opencv2/flann.hpp>
#endif


using namespace cv;
using namespace std;

class MyTrack
{
public:
	MyTrack();
	int countOfPoint();
	Point lastPoint();
	void clear();
	bool add(Point point);
	void draw(Mat img);
	Point center(Rect rect);
	vector<Point> myLine;
	void showLine();
	bool nextTo(Point point);
	int idTrack;
	int age();	
	int currentKarma=0;
	int karma = 8;
	Rect lastRect;
	void addToKarma();
private:	
	int maxAgeUsingTime=9000;
	Point point;	
	int minDistance = 50;
	int maxPoints = 50;
	clock_t lastTime= clock();
	Scalar color= Scalar(rand() % 255, rand() % 255, rand() % 255);
};

class MultiTrack
{
public:
	void add(vector<Rect>& rects);
	MultiTrack();
	void draw(Mat img);
	void show();
	vector<int> oldTracks;
	vector<int> whoIsOld();
	int maxAge = 3;
	int maxAgeUsingTime = 9000;
	vector<MyTrack>& getVecTrack();
	void destroyTrack(int id);
	bool tryDestroyAll();
private:
	vector<pair<int, int>> intersectionRect(vector<Rect> rects);
	Size maxSizeRect = Size(150,150);
	int countOfTracks;
	int lastNumberTrack;
	int whoIsVacant();
	vector<MyTrack> vecTrack;
	int newTrack();
};


class OpenCVMultiTracker
{
public:
	OpenCVMultiTracker(Mat frame, vector<Rect> ROIs);
	Point start(VideoCapture cap);
	void start(VideoCapture cap,VideoWriter& oVideoWriter, MultiTrack& multiTrack);

private:

	std::string trackingAlg = "KCF";
	vector<Rect2d> objects;
	Rect lastRect;
	vector<Rect> ROIs;
	Mat fullImage;
	std::vector<Ptr<Tracker> > algorithms;
	void addROIs();
	MultiTracker trackers;

	int countOfMotion = 0;
	int maxCountOfMotion = 2;
};
