#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include "Object_Selection.h"


extern int Progress;

using namespace std;
using namespace cv;

class MyTracker
{
public:
	//Trackertypes: "BOOSTING", "MIL", "KCF", "TLD","MEDIANFLOW", "GOTURN", "MOSSE", "CSRT"
	MyTracker(string TrackerType = "KCF", double frame_incr_x = 1, double frame_incr_y = 1);
	vector<vector<vector<int>>> GetMasks(string VideoName, Object_Selection* obj_selection, bool with_contours = true);
	//virtual void DrawFrames(string OutVideoName = "") = 0;
	//virtual void DrawMasks(string OutVideoName = "") = 0;
	~MyTracker() {}
protected:
	double object_frame_increase_x, object_frame_increase_y;
	string TrackerType;
//	string CurrentVideoName;
	Ptr<Tracker> Init();
	vector<vector<vector<int>>> Masks; //обновляется Tracking-ом, когда with_contours=true
	vector<vector<Rect2d>> Obj_Frames; //обновляется Tracking-ом
	virtual int Tracking(string VideoName, Object_Selection* obj_selection) = 0;
	virtual int TrackingOnlyFrames(string VideoName, Object_Selection* obj_selection) = 0;
};

class TrackingType
{
	TrackingType(string TrackerType = "KCF", double frame_incr_x = 1, double frame_incr_y = 1);
	virtual int Tracking(string VideoName, Object_Selection* obj_selection) = 0;
	virtual int TrackingOnlyFrames(string VideoName, Object_Selection* obj_selection) = 0;
};

class FacesTracker : TrackingType
{
public:
	FacesTracker(string TrackerType = "KCF", double frame_incr_x = 1, double frame_incr_y = 1) : TrackingType(TrackerType, frame_incr_x, frame_incr_y) {};
	int Tracking(string VideoName, Object_Selection* obj_selection);
	int TrackingOnlyFrames(string VideoName, Object_Selection* obj_selection);
//	void DrawFrames(string OutVideoName = "");
//	void DrawMasks(string OutVideoName = "");
private:
	void UpdateRois(const Mat& image, vector<Rect2d> faces, vector<Rect2d>& rois, vector<Ptr<Tracker>>& trackers);

};

class ObjectTracker : MyTracker
{
public:
	int Tracking(string VideoName, Object_Selection* obj_selection);
	int TrackingOnlyFrames(string VideoName, Object_Selection* obj_selection);
//	void DrawFrames(string VideoName);
//	void DrawContours(string VideoName);
};
