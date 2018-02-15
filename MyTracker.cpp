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


//=========================================================================================

MyTrackUsingRect::MyTrackUsingRect()
{
	lastTime = clock();
}

int MyTrackUsingRect::countOfRect()
{
	return myRects.size();
}

void MyTrackUsingRect::clear()
{
	myRects.clear();
}

void MyTrackUsingRect::draw(Mat& img)
{
	if (age() != -1)
	{
		for (auto currentRect : myRects)
		{
			cv::rectangle(img, currentRect.lastInput, color, 3);
		}
	}
}

int MyTrackUsingRect::age()
{
	int now = clock();
	if (lastTime <= 0 || lastTime>10000000)lastTime = now;
	else if ((now - lastTime)>maxAgeUsingTime) return -1;
	return (now - lastTime);
}

void MyTrackUsingRect::showLine()
{
	cout << endl << "---------" << idTrack << "---------" << endl;
	for (auto myRect:myRects)
	{
		cout << "x=" << to_string(myRect.lastInput.x)<<" y=" << to_string(myRect.lastInput.y)<<" | ";
	}
	cout << endl << "===================" << endl;
	
}

bool MyTrackUsingRect::add(Rect inputRect)
{
	if (inputRect == Rect())return false;
	PointOfRect inputRect_(inputRect);

	myRects.push_back(inputRect_);
	lastTime = clock();	

	if (myRects.size() > maxRects)
	{
		try
		{
			myRects.erase(myRects.begin());
		}
		catch (Exception& ex)
		{
			cout << "Ошибка " << ex.what() << endl;
		}
	}

	return true;
}

bool MyTrackUsingRect::nextTo(Rect inputRect)
{
	if (inputRect == Rect())return false;
	PointOfRect inputRect_(inputRect);

	double distBC = cv::norm(inputRect_.bottomCenter - myRects.back().bottomCenter);
	double distTC = cv::norm(inputRect_.topCenter - myRects.back().topCenter);
	double distLC = cv::norm(inputRect_.leftCenter - myRects.back().leftCenter);
	double distRC = cv::norm(inputRect_.rightCenter - myRects.back().rightCenter);
	if (minDistance > distBC && minDistance > distTC && minDistance > distLC && minDistance > distRC)
		return true;
	else
		return false;
}

bool MyTrackUsingRect::nextTo(Rect inputRect, int dir)
{
	if (inputRect == Rect())return false;
	PointOfRect inputRect_(inputRect);

	double distBC = cv::norm(inputRect_.bottomCenter - myRects.end()->bottomCenter);
	double distTC = cv::norm(inputRect_.topCenter - myRects.end()->topCenter);
	double distLC = cv::norm(inputRect_.leftCenter - myRects.end()->leftCenter);
	double distRC = cv::norm(inputRect_.rightCenter - myRects.end()->rightCenter);
	if (minDistance > distBC && minDistance > distTC && (dir==LEFT? minDistance > distLC:minDistance > distRC))
		return true;
	else
		return false;
}

PointOfRect& MyTrackUsingRect::getLastRect()
{
	if (myRects.size())
		return myRects.back();
	else
		return PointOfRect(Rect());
}

double MyTrackUsingRect::getWeightedDistance(cv::Rect detect)
{
	double leftDist = distPointToPoint(getLastRect().leftCenter,
		cv::Point(detect.x, detect.y + detect.height / 2));
	double rightDist = distPointToPoint(getLastRect().rightCenter,
		cv::Point(detect.br().x, detect.y + detect.height / 2));
	double topDist = distPointToPoint(getLastRect().topCenter,
		cv::Point(detect.x + detect.width / 2, detect.y));
	double bottomDist = distPointToPoint(getLastRect().bottomCenter,
		cv::Point(detect.x + detect.width / 2, detect.br().y));
	double averageDist = (leftDist+rightDist+topDist+bottomDist) / 4.0;

	double coef = getLastRect().lastInput.area() / detect.area() - 1;
	averageDist *= abs(coef) + 1;
	return averageDist;
}

//=========================================================================================
MultiTrack::MultiTrack()
{
	srand(time(NULL));
}

vector<HowIntersectedRect> MultiTrack::intersectionRect(vector<Rect> rects)
{
	
	vector<HowIntersectedRect>v_howIntersectedRect;
	HowIntersectedRect howIntersectedRect;
	if (rects.size() > 1)
	{
		for (int i = 0; i < rects.size(); i++)
		{
			if (rects[i] > maxSizeRect)continue;
			for (int j = i + 1; j <= rects.size(); j++)
			{
				if (rects[j] > maxSizeRect)continue;
				Rect resultRect=rects[i] & rects[j];
				if (resultRect != Rect())
				{
					howIntersectedRect.whoAreIntersected=pair<int,int>(rects[i].area() >rects[j].area() ? i : j,
						rects[i].area() >rects[j].area() ? j : i);
					howIntersectedRect.howWereIntersected = -1;
					cout << "detect intersection: " << i << " and " << j << endl;

					if (resultRect == rects[i])
					{
						cout << "Rect " << i << "(is smaller)" << " inside " << j << endl;
						howIntersectedRect.howWereIntersected = i;
					}
					else if (resultRect == rects[j])
					{
						cout << "Rect " << j << "(is smaller)" << " inside " << i << endl;
						howIntersectedRect.howWereIntersected = j;
					}
				}
				v_howIntersectedRect.push_back(howIntersectedRect);
			}
		}
	}
	return v_howIntersectedRect;
	
}

void MultiTrack::addUsingRect(vector<Rect>& rects)
{
	
	if (!rects.size())return;

	vector<HowIntersectedRect> intersectionStruct = intersectionRect(rects);

	for (auto rect : rects)
	{

		if (rect > maxSizeRect)continue;

		PointOfRect pointOfRect(rect);

		if (countOfTracks)
		{
			map<int, int>mapOfOverlap;
			int position = 0; //position of track in vector of tracks
			for (auto& currentTrack : vecTrack)
			{
				// TODO I'v forgot what is it
				//currentTrack.minDistance = pointOfRect.minDistanse;
				//---------
				if (currentTrack.nextTo(rect))
				{
					mapOfOverlap.insert(pair<int, int>(position, currentTrack.idTrack));
				}
				position++;
			}

			if (!mapOfOverlap.size()) //no one is near
			{
				
				int idNewTrack = newTrack();
				vecTrack[idNewTrack - 1].add(rect);
				vecTrack[0].getLastRect().lastInput = rect;
				continue;
			}
			else if (mapOfOverlap.size() == 1) //only one track near to point
			{
				vecTrack[mapOfOverlap.begin()->first].add(rect);
				vecTrack[mapOfOverlap.begin()->first].getLastRect().lastInput= rect;
				continue;
			}
			else //two track have to keep the point
			{
				cout << "Near to few tracks" << endl;
				auto bestTrackIt = vecTrack.begin() + mapOfOverlap.begin()->first;
				double bestDist = bestTrackIt->getWeightedDistance(rect);
				for (auto it = mapOfOverlap.begin(); it != mapOfOverlap.end(); it++)
				{
					double dist = (vecTrack.begin() + it->first)->getWeightedDistance(rect);
					if (dist < bestDist)
					{
						bestTrackIt = vecTrack.begin() + it->first;
						bestDist = dist;
					}
				}
				bestTrackIt->add(rect);
			}
		}
		else
		{
			int newTrackId = newTrack();
			cout << "New track " << newTrackId << endl;
			vecTrack[0].add(rect);
			vecTrack[0].getLastRect().lastInput = rect;
			//v_assigment[0] = 0;
			continue;
		}
		
	}
}

int MultiTrack::newTrack()
{
	int vacantNumber = whoIsVacant();
	vecTrack.emplace_back(MyTrackUsingRect());
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

vector<MyTrackUsingRect>& MultiTrack::getVecTrack()
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
		if ((tempAgeTrack >= maxAgeUsingTime)|| !currentTrack.countOfRect() || tempAgeTrack==-1)
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
/*
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
*/