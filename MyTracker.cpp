#include "MyTracker.h"

void paintRect(Rect rect, Mat image, int r, int g, int b)
{
	rect.x = min(rect.x, image.cols);
	rect.y = min(rect.y, image.rows);
	rect.width = min(rect.width, image.cols);
	rect.height = min(rect.height, image.rows);
	rectangle(image, rect, Scalar(r, g, b), 2, 1);
}

Point centerOfRect(Rect rect)
{
	return Point(rect.x + rect.width / 2, rect.y + rect.height / 2);
}

bool operator>(Rect r,Size s)
{
	if(r.width>s.width && r.height>s.height) return true;
	else return false;
}

//=====================================================
MyTrack::MyTrack()
{
	lastTime = clock();
}

int MyTrack::countOfPoint()
{
	return myLine.size();
}

Point MyTrack::lastPoint()
{
	return point;
}

void MyTrack::clear()
{
	myLine.clear();
}

bool MyTrack::add(Point point)
{
	try
	{
		int tempSize = myLine.size();
		if (!tempSize && point != Point())
		{
			this->point = point;
			myLine.push_back(point);
			lastTime = clock();
			return 0;
		}
	}
	catch (Exception& ex)
	{
		cout <<"exception "<< ex.what() << endl;
	}
	double dist = cv::norm(point - this->point);
	if (minDistance > dist)
	{
		
			if (myLine.size() > maxPoints)
			{	try
				{
				myLine.erase(myLine.begin());
				}
				catch (Exception& ex)
				{
				cout << "Ошибка " << ex.what() << endl;
				}
			}
		
		myLine.push_back(point);
		this->point = point;
		lastTime = clock();
		return 0;
	}
	else
		return 1;	
}

void MyTrack::draw(Mat img)
{
	Point lastP;
	if (age() != -1)
	{
		for (auto currentLine : myLine)
		{
			if (lastP != Point())
			{
				arrowedLine(img, lastP, currentLine, color, 3, 8, 0, 0.1);
			}
			lastP = currentLine;
		}
	}
}

Point MyTrack::center(Rect rect)
{
	return Point(rect.x+rect.width/2,rect.y+rect.height/2);
}

void MyTrack::showLine()
{
	cout << endl << "---------"<< idTrack <<"---------" << endl;
	std::copy(myLine.begin(), myLine.end(),	std::ostream_iterator<Point>(std::cout, " "));
	cout << endl << "===================" << endl;
}

bool MyTrack::nextTo(Point point)
{
	double dist = cv::norm(point - this->point);
	if (minDistance > dist)
		return true;
	else
		return false;
}

int MyTrack::age()
{
	int now = clock();
	if (lastTime <= 0 || lastTime>10000000)lastTime=now;
	else if ((now - lastTime)>maxAgeUsingTime) return -1;
	return (now - lastTime);
}

void MyTrack::addToKarma()
{
	currentKarma++;
}

//=========================================================================================
MultiTrack::MultiTrack()
{
	srand(time(NULL));
}

vector<pair<int,int>> MultiTrack::intersectionRect(vector<Rect> rects)
{
	vector<pair<int, int>>whoIsIntersects;
	if (rects.size() > 1)
	{
		for (int i = 0; i < rects.size(); i++)
		{
			if (rects[i] > maxSizeRect)continue;
			for (int j = i + 1; j <= rects.size(); j++)
			{
				if (rects[j] > maxSizeRect)continue;
				if ((rects[i] & rects[j]) != Rect())
				{
					whoIsIntersects.push_back(pair<int,int>(rects[i].area() >rects[j].area() ? i : j,
						rects[i].area() >rects[j].area() ? j : i)) ;
					cout << "detect intersection: " << i << " and " << j << endl;
				}
			}
		}
	}
	return whoIsIntersects;
}


void MultiTrack::add(vector<Rect>& rects)
{

	if (!rects.size())return;

	if (rects.size() > 1)
		cout << endl;
	vector<pair<int, int>>intersectionPairs=intersectionRect(rects);

	int tempCurrentStep=0;
	for (auto rect : rects)
	{
		if (rect > maxSizeRect)continue;
		
		//for (auto intersectionPair : intersectionPairs)
		//	if (intersectionPair.first == tempCurrentStep) return;
	
		//if (tempCurrentStep == intersectionPair.second)continue;
		Point point = centerOfRect(rect);
		if (countOfTracks)
		{
			map<int, int>mapOfOverlap;
			int position = 0; //position of track in vector of tracks
			for (auto& currentTrack : vecTrack)
			{
				if (currentTrack.nextTo(point))
				{
					mapOfOverlap.insert(pair<int, int>(position, currentTrack.idTrack));
					currentTrack.currentKarma = 0;
				}
				else
				{
					currentTrack.currentKarma = min((currentTrack.currentKarma + 1), currentTrack.karma);
				}
				position++;
			}

			if (!mapOfOverlap.size()) //no one is near
			{
				int idNewTrack = 0;
				idNewTrack = newTrack();
				vecTrack[idNewTrack - 1].add(point);
				vecTrack[idNewTrack - 1].lastRect = rect;
				continue;
			}
			else if (mapOfOverlap.size() == 1) //only one track near to point
			{
				vecTrack[mapOfOverlap.begin()->first].add(point);
				vecTrack[mapOfOverlap.begin()->first].lastRect = rect;
				continue;
			}
			else //two track have to keep the point
			{
				cout << "Near to few tracks" << endl;
			}
		}
		else
		{
			int newTrackId = newTrack();
			cout << "New track " << newTrackId << endl;
			vecTrack[0].add(point);
			vecTrack[0].lastRect = rect;
			continue;
		}
		tempCurrentStep++;
	}
}

int MultiTrack::newTrack()
{
	int vacantNumber = whoIsVacant();
	vecTrack.emplace_back(MyTrack());
	vecTrack.back().idTrack = vacantNumber;
	lastNumberTrack = max(vacantNumber, lastNumberTrack);
	countOfTracks++;
	return vacantNumber;
}

int MultiTrack::whoIsVacant()
{
	return (lastNumberTrack + 1);
}

void MultiTrack::destroyTrack(int id)
{
	int position = 0;
	for (int i = 0; i < vecTrack.size(); i++)
	{
		if (id == vecTrack[i].idTrack)
		{
			vecTrack.erase(vecTrack.begin()+i);
			countOfTracks--;
			return;
		}
	}
}

void MultiTrack::draw(Mat img)
{
	for (auto currentTrack : vecTrack)
	{
		currentTrack.draw(img);
	}
}

void MultiTrack::show()
{
	for (auto currentTrack : vecTrack)
	{
		currentTrack.showLine();
	}
}

vector<int> MultiTrack::whoIsOld()
{
	for (auto currentTrack : vecTrack)
	{
		if (currentTrack.age() >= maxAge)oldTracks.push_back(currentTrack.idTrack);
	}

	return oldTracks;
}

vector<MyTrack>& MultiTrack::getVecTrack()
{
	return vecTrack;
}

bool MultiTrack::tryDestroyAll()
{   
	cout<<endl;
	int tempCountOfTracks = 0;
	for (auto currentTrack : vecTrack)
	{
		int tempAgeTrack=currentTrack.age();
		if ((tempAgeTrack >= maxAgeUsingTime)|| !currentTrack.countOfPoint() || tempAgeTrack==-1)
		tempCountOfTracks++;
	}
	if (tempCountOfTracks >= vecTrack.size())
	{
		vecTrack.clear();
		countOfTracks=0;
		lastNumberTrack=0;
		cout << "." << endl;
		return false;
	}
	else
	{
		return true;
	}
}

//==============================================================================
OpenCVMultiTracker::OpenCVMultiTracker( Mat frame, vector<Rect> ROIs)
{
	fullImage = frame;
	this->ROIs = ROIs;
}

Point OpenCVMultiTracker::start(VideoCapture cap)
{
	addROIs();
	printf("Start the tracking process, press ESC to quit.\n");
	for (;; )
	{
		countOfMotion++;

		cap >> fullImage;

		// не вести трекер если нет кадров
		if (fullImage.rows == 0 || fullImage.cols == 0)
			//  break;
			continue;

		//обновить трекер
		trackers.update(fullImage);

		// нарисовать
		for (unsigned i = 0; i < trackers.getObjects().size(); i++)
			paintRect(trackers.getObjects()[i], fullImage, 0, 0, 255);
		//rectangle(frame, trackers.getObjects()[i], Scalar(255, 0, 0), 2, 1);

		imshow("Stream", fullImage);
		waitKey(1);
		if (countOfMotion > maxCountOfMotion)return centerOfRect(trackers.getObjects()[0]);

		if (waitKey(1) == 27)
		{
			break;
		}
		//if(getFaces(frame).size())break;		
	}
}

void OpenCVMultiTracker::start(VideoCapture cap,  VideoWriter& oVideoWriter, MultiTrack& multiTrack)
{
	addROIs();
	printf("Start the tracking process, press ESC to quit.\n");
	for (;; )
	{
		countOfMotion++;

		cap >> fullImage;

		// не вести трекер если нет кадров
		if (fullImage.rows == 0 || fullImage.cols == 0)
			//  break;
			continue;

		//обновить трекер
		trackers.update(fullImage);

		// нарисовать
		for (unsigned i = 0; i < trackers.getObjects().size(); i++)
		{
			paintRect(trackers.getObjects()[i], fullImage, 0, 255, 255);
			//rectangle(fullImage, trackers.getObjects()[i], Scalar(0, 0, 255), 2, 1);
			//multiTrack.add(trackers.getObjects()[i]);
		}
		
		//multiTrack.draw(fullImage);
		oVideoWriter.write(fullImage);
		
		imshow("Stream", fullImage);
		waitKey(1);
		if (countOfMotion > maxCountOfMotion)return;

		if (waitKey(1) == 27)
		{
			break;
			return ;
		}
		//if(getFaces(frame).size())break;
	}
}

void OpenCVMultiTracker::addROIs()
{
	objects.clear();
	for (size_t i = 0; i < ROIs.size(); i++)
	{
		algorithms.push_back(createTrackerByName(trackingAlg));
		objects.push_back(ROIs[i]);
	}
	trackers.add(algorithms, fullImage, objects);
}
