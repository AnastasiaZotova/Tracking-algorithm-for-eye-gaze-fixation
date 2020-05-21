#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include "Object_Selection.h"


using namespace std;
using namespace cv;

struct TrackingType
{
public:
	TrackingType(string TrackerType, double frame_incr_x, double frame_incr_y);
	virtual int Tracking(string VideoName, Object_Selection* obj_selection, vector<vector<vector<Point>>>& Contours) = 0;
	virtual ~TrackingType() {}
protected:
	Ptr<Tracker> Init();
	double object_frame_increase_x, object_frame_increase_y;
	string TrackerType;
};

struct FacesTracker : TrackingType
{
public:
	FacesTracker(string TrackerType, double frame_incr_x, double frame_incr_y) : TrackingType(TrackerType, frame_incr_x, frame_incr_y) {};
	int Tracking(string VideoName, Object_Selection* obj_selection, vector<vector<vector<Point>>>& Contours);
private:
	void UpdateRois(const Mat& image, vector<Rect2d> faces, vector<Rect2d>& rois, vector<Ptr<Tracker>>& trackers);
};

struct ObjectTracker : TrackingType
{
public:
	ObjectTracker(string TrackerType, double frame_incr_x, double frame_incr_y) : TrackingType(TrackerType, frame_incr_x, frame_incr_y) {};
	int Tracking(string VideoName, Object_Selection* obj_selection, vector<vector<vector<Point>>>& Contours);
};

struct EachFrameFacesTracker : TrackingType
{
public:
	EachFrameFacesTracker(string TrackerType, double frame_incr_x, double frame_incr_y) : TrackingType(TrackerType, frame_incr_x, frame_incr_y) {};
	int Tracking(string VideoName, Object_Selection* obj_selection, vector<vector<vector<Point>>>& Contours);

};

class MyTracker
{
public:
	MyTracker(Algorithms track_type, string TrackerType = "KCF", double frame_incr_x = 1.2, double frame_incr_y = 1.3);
	void GetObjects(string VideoName, Object_Selection* obj_selection, vector<vector<vector<Point>>>& Contours)
	{
		tracker->Tracking(VideoName, obj_selection, Contours);
	}
	~MyTracker() { delete tracker; }
private:
	TrackingType* tracker;
};
