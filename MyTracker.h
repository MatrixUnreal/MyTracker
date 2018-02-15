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

#include "mymath.h"

#ifdef HAVE_OPENCV
#include <opencv2/flann.hpp>
#endif

using namespace cv;
using namespace std;

Point centerOfRect(Rect rect);

struct HowIntersectedRect
{
	pair<int, int> whoAreIntersected;
	int howWereIntersected;
};

enum { LEFT, RIGHT };


struct PointOfRect
{
	Point pointCenter, topCenter, bottomCenter, leftCenter, rightCenter;
	int minDistanse;
	Rect lastInput;
	PointOfRect(Rect rect)
	{
		pointCenter = centerOfRect(rect);
		topCenter = Point(rect.x + rect.width / 2, rect.y);
		bottomCenter = Point(rect.x + rect.width / 2, rect.y + rect.height);
		leftCenter = Point(rect.x, rect.y + rect.height / 2);
		rightCenter = Point(rect.x + rect.width, rect.y + rect.height / 2);
		minDistanse = (rect.width / (2.2));
		lastInput = rect;
	}
};


class MyTrackUsingRect
{
public:
	MyTrackUsingRect();
	vector<PointOfRect> myRects;
	int countOfRect();
	void clear();
	void draw(Mat& img);
	bool nextTo(Rect inputRect);
	bool nextTo(Rect inputRect,int dir);
	void showLine();
	int age();
	int idTrack;
	bool add(Rect inputRect);
	bool visible = false;
	PointOfRect& getLastRect();
	double getWeightedDistance(cv::Rect detect);
	//vector<Point> myLine;
	//Point center(Rect rect);
	//int currentKarma = 0;
	//int karma = 8;	
	//void addToKarma();	
private:	
	int maxAgeUsingTime = 9000;
	clock_t lastTime = clock();
	Scalar color = Scalar(rand() % 255, rand() % 255, rand() % 255);
	int maxRects = 50;	
	int minDistance = 400;
};

class MultiTrack
{
public:
	void addUsingRect(vector<Rect>& rects);
	MultiTrack();
	void draw(Mat img);
	void show();
	vector<int> oldTracks;
	vector<int> whoIsOld();
	int maxAge = 3;
	int maxAgeUsingTime = 9000;
	vector<MyTrackUsingRect>& getVecTrack();
	void destroyTrack(int id);
	bool tryDestroyAll();
private:
	vector<HowIntersectedRect> intersectionRect(vector<Rect> rects);
	Size maxSizeRect = Size(400,400);
	int countOfTracks;
	int lastNumberTrack;
	int whoIsVacant();
	vector<MyTrackUsingRect> vecTrack;
	int newTrack();
};

/*
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
*/
 