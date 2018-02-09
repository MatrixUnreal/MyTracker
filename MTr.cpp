#include <opencv2/opencv.hpp>
#include <iostream>
#include <Windows.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <fstream>
#include <vector>

#include "MyTracker.h"
                 
using namespace cv;
using namespace std;

int fourcc = CV_FOURCC('X', 'V', 'I', 'D');//     #does not work
int	fourcc0 = CV_FOURCC('D', 'I', 'V', 'X');//     #does not work
int	fourcc1 = CV_FOURCC('M', 'J', 'P', 'G');//     #does not work
int	fourcc2 = CV_FOURCC('8', 'B', 'P', 'S');//     #works, large
int	fourcc3 = CV_FOURCC('A', 'V', 'R', 'N');//     #does not work
int	fourcc4 = CV_FOURCC('R', 'P', 'Z', 'A');//     #does not work
int	fourcc5 = CV_FOURCC('S', 'V', '1', '0');//     #does not work
int	fourcc6 = CV_FOURCC('S', 'V', 'Q', '3');//     #works, small
int	fourcc7 = CV_FOURCC('Z', 'Y', 'G', 'O');//     #does not work
int	fourcc8 = CV_FOURCC('P', 'I', 'M', '1');

struct Camera
{
	Mat frame_cam;
	string address_cam;
	string name_cam;
	int frame_width_cam, frame_height_cam;

	bool operator<(const Camera& cam) const
	{
		return name_cam < cam.name_cam;
	}
};

cv::CascadeClassifier face_detector;

void init_face_detect()
{
	//std::string face_cascade_name = "C:\\Rustam\\Library\\OpenCV32\\opencv\\sources\\data\\haarcascades\\haarcascade_upperbody.xml";
	//std::string face_cascade_name = "C:\\Rustam\\Library\\OpenCV32\\opencv\\sources\\data\\haarcascades\\haarcascade_profileface.xml";
	//std::string face_cascade_name = "C:\\Rustam\\Library\\OpenCV32\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_alt_tree.xml";
	//std::string face_cascade_name = "C:\\Rustam\\Library\\OpenCV32\\opencv\\sources\\data\\haarcascades\\haarcascade_eye_tree_eyeglasses.xml";
	std::string face_cascade_name = "C:\\Rustam\\Library\\OpenCV32\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_alt2.xml";
	//std::string face_cascade_name = "C:\\Rustam\\Library\\OpenCV32\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalcatface_extended.xml";
	//std::string face_cascade_name = "C:\\Rustam\\Library\\OpenCV32\\opencv\\sources\\data\\lbpcascades\\lbpcascade_frontalface.xml";
	face_detector.load(face_cascade_name);
	if (!face_detector.load(face_cascade_name))
	{
		std::cout << "error loading cascade" << std::endl;
	}
}

std::vector<cv::Rect> getFaces(const cv::Mat& image)
{
	std::vector<cv::Rect>faces;
	cv::Mat gray;
	cvtColor(image, gray, cv::COLOR_BGR2GRAY);
	//gray=gray(Rect(0,0, gray.rows/2,gray.cols/2));
		//copygray

	face_detector.detectMultiScale(gray, faces, 1.1, 8, 0 | 
		//CV_HAAR_DO_CANNY_PRUNING,
		//CV_HAAR_DO_ROUGH_SEARCH,
		//CV_HAAR_FEATURE_MAX,
		CV_HAAR_FIND_BIGGEST_OBJECT,
		//CV_HAAR_SCALE_IMAGE,
		cv::Size(0,0));

	return faces;
}

Point center(Rect rect)
{
	return Point(rect.x + rect.width / 2, rect.y + rect.height / 2);
}

int main()
{

	MultiTrack multiTrack;
	init_face_detect(); 
	Camera camera;
	camera.address_cam = "Video//3.mp4";
	
	//camera.address_cam = "Video//3.avi";
	//camera.address_cam = "rtsp://admin:admin@192.168.90.168";
	VideoCapture capture(camera.address_cam);
	int emptyFrames_cam = 0;
	if (!capture.isOpened())
	{
		cerr << "ERROR: unable to open camera stream 1" << endl;
	}
	const string name = "Output.avi";
	double dWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
	double dHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video
	Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));
	double fps = capture.get(CV_CAP_PROP_FPS);
	VideoWriter oVideoWriter(name, fourcc, 18, frameSize, true); //initialize the VideoWriter object 

	if (!oVideoWriter.isOpened()) //if not initialize the VideoWriter successfully, exit the program
	{
		cout << "ERROR: Failed to write the video" << endl;
		//return -1;
	}

	cvNamedWindow("Stream", 0);
	resizeWindow("Stream", 1200, 700);
	//resizeWindow("Stream", 700, 1200);
	moveWindow("Stream", 0, 200);

	while (1)
	{
		
		int countoffaces = 0;
		vector<Rect> v_lastRect;
		emptyFrames_cam = 0;
		capture >> camera.frame_cam;
		

		while (camera.frame_cam.empty())
		{
			capture >> camera.frame_cam;
			emptyFrames_cam++;
			if (emptyFrames_cam > 10)
			{
				cout << "will release frame1" << endl;
				capture.release();
				capture.open(camera.address_cam);

				system("pause");
			}
			if (emptyFrames_cam > 1000)emptyFrames_cam = 0;
		}
		
		vector<Rect> faces;
		faces=getFaces(camera.frame_cam);
		Point centerOfFace;		

		for (auto face : faces)
		{
			cv::rectangle(camera.frame_cam, face, cv::Scalar(0, 255, 0),3);
			multiTrack.add(face);
			countoffaces++;
		}
		
		
		for (auto& currentTrack : multiTrack.getVecTrack())
		{
			cout << "******************" << endl;
			cout << "IdTrack: " << currentTrack.idTrack << " age: " << currentTrack.age()<<endl;
			
			if (currentTrack.age() > multiTrack.maxAgeUsingTime)
			{
				cout << "Track was cleared" << endl;
				currentTrack.clear();
			}

			cout << "//////////////////" << endl;
			/*currentTrack.addToKarma();
			if (currentTrack.currentKarma >= currentTrack.karma)
			{
				cout << "Old track " << to_string(currentTrack.idTrack) << endl;
				v_lastRect.push_back(currentTrack.lastRect);
			}*/
		}

		if(v_lastRect.size())
		{
			OpenCVMultiTracker openCVMultiTracker(camera.frame_cam, v_lastRect);
			openCVMultiTracker.start(capture, oVideoWriter, multiTrack);
		}		
		multiTrack.tryDestroyAll();
		multiTrack.draw(camera.frame_cam);
		

		waitKey(1);
		imshow("Stream", camera.frame_cam);
		waitKey(1);		
		oVideoWriter.write(camera.frame_cam);
	}

	oVideoWriter.release();
	
    return 0;
}

