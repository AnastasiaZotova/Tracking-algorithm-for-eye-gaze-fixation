#include "pch.h"
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include <fstream>
#include "Track.h"

using namespace std;
using namespace cv;

MyTracker::MyTracker(Algorithms track_type, string TrackerType, double frame_incr_x, double frame_incr_y)
{
    switch (track_type) {
    case FaceTrack:
        tracker = new FacesTracker(TrackerType, frame_incr_x, frame_incr_y);
        break;
    case ObjectTrack:
        tracker = new ObjectTracker(TrackerType, frame_incr_x, frame_incr_y);
        break;
    case WithoutTracking:
        tracker = new EachFrameFacesTracker(TrackerType, frame_incr_x, frame_incr_y);
        break;
    default:
        tracker = new EachFrameFacesTracker(TrackerType, frame_incr_x, frame_incr_y);
        break;
    }
}


TrackingType::TrackingType(string TrackerType, double frame_incr_x, double frame_incr_y)
{
    this->TrackerType = TrackerType;
    object_frame_increase_x = frame_incr_x;
    object_frame_increase_y = frame_incr_y;
}
Ptr<Tracker> TrackingType::Init()
{
    Ptr<Tracker> track;
    if (TrackerType == "BOOSTING")
        track = TrackerBoosting::create();
    if (TrackerType == "MIL")
        track = TrackerMIL::create();
    if (TrackerType == "KCF")
        track = TrackerKCF::create();
    if (TrackerType == "TLD")
        track = TrackerTLD::create();
    if (TrackerType == "MEDIANFLOW")
        track = TrackerMedianFlow::create();
    if (TrackerType == "GOTURN")
        track = TrackerGOTURN::create();
    if (TrackerType == "MOSSE")
        track = TrackerMOSSE::create();
    if (TrackerType == "CSRT")
        track = TrackerCSRT::create();
    return track;
}

void FacesTracker::UpdateRois(const Mat& image, vector<Rect2d> faces, vector<Rect2d>& rois, vector<Ptr<Tracker>>& trackers)
{
    double face_x, face_y;
    Ptr<Tracker> track;
    for (int i = 0; i < faces.size(); i++)
    {
        face_x = (faces[i].x + faces[i].width) / 2;
        face_y = (faces[i].y + faces[i].height) / 2;
        if (rois.size() < i + 1 || ((face_x - rois[i].x) * (face_x - rois[i].x - rois[i].width) < 0 && (face_y - rois[i].y) * (face_y - rois[i].y - rois[i].height)))
        {
            rois.push_back(faces[i]);
            track = Init();
            track->init(image, rois[rois.size() - 1]);
            trackers.push_back(track);
        }
    }
}

int FacesTracker::Tracking(string VideoName, Object_Selection* obj_selection, vector<vector<vector<Point>>>& Contours)
{
    Mat frame;
    VideoCapture video(VideoName);
    bool ok = video.read(frame), isfail = false;
    int num, frame_number = 0;
    Contours = vector<vector<vector<Point>>>(video.get(CAP_PROP_FRAME_COUNT));
    vector<Ptr<Tracker>> trackers;
    while (obj_selection->Get_Object_Frames(frame).size() == 0)
    {
        frame_number++;
        cout << frame_number << endl;
        if (!video.read(frame))
            return 0;
    }
    vector<Rect2d> cur_faces, rois;
    if (object_frame_increase_x == 1 && object_frame_increase_y == 1)
        rois = obj_selection->Get_Object_Frames(frame);
    else
        rois = obj_selection->Get_Enlarged_Frames(frame, object_frame_increase_x, object_frame_increase_y);
    Contours[frame_number] = obj_selection->Get_All_Contours(frame);
    for (int i = 0; i < rois.size(); i++)
    {
        trackers.push_back(Init());
        trackers[i]->init(frame, rois[i]);
    }
    while (video.read(frame))
    {
        frame_number++;
        cout << frame_number << endl;
        for (int i = 0; i < trackers.size(); i++)
        {
            ok = trackers[i]->update(frame, rois[i]);
            if (!ok)
            {
                isfail = true;
                trackers[i]->clear();
                trackers[i] = nullptr;
                rois[i] = Rect2d(0, 0, 0, 0);
            }
        }
        if (trackers.size() == 0)
        {
            rois = (object_frame_increase_x == 1 && object_frame_increase_y == 1) ? obj_selection->Get_Object_Frames(frame) : obj_selection->Get_Enlarged_Frames(frame, object_frame_increase_x, object_frame_increase_y);
            for (int i = 0; i < rois.size(); i++)
            {
                trackers.push_back(Init());
                trackers[i]->init(frame, rois[i]);
            }
        }
        if (isfail)
        {
            cur_faces = (object_frame_increase_x == 1 && object_frame_increase_y == 1) ? obj_selection->Get_Object_Frames(frame) : obj_selection->Get_Enlarged_Frames(frame, object_frame_increase_x, object_frame_increase_y);
            UpdateRois(frame, cur_faces, rois, trackers);
            for (int i = 0; i < trackers.size(); i++)
            {
                if (trackers[i] == nullptr)
                {
                    trackers.erase(trackers.begin() + i);
                    rois.erase(rois.begin() + i);
                    i--;
                }
            }
            isfail = false;
        }
        Contours[frame_number] = obj_selection->Get_All_Contours(frame, rois);
    }
    return 1;
}

int ObjectTracker::Tracking(string VideoName, Object_Selection* obj_selection, vector<vector<vector<Point>>>& Contours)
{

    Mat frame;
    VideoCapture video(VideoName);
    bool ok = video.read(frame), isfail = false;
    int num, frame_number = 0;
    Contours = vector<vector<vector<Point>>>(video.get(CAP_PROP_FRAME_COUNT));
    Ptr<Tracker> tracker = Init();
    Rect2d roi;
    int l = 10;
    imshow("1", frame);
    char key = (char)cv::waitKey();
    while (video.read(frame) && key != 'y')
    {
        l++;
        frame_number++;
        if (l % 10 == 0)
        {
            imshow("1", frame);
            key = (char)cv::waitKey();
        }
    }
    if (key == 'y')
    {
        Mat img;
        resize(frame, img, Size(frame.cols / 2, frame.rows / 2));
        roi = selectROI("select object", img);
        roi.x *= 2;
        roi.y *= 2;
        roi.height *= 2;
        roi.width *= 2;
        tracker->init(frame, roi);
    }
    else return 0;
    Contours[frame_number].push_back(obj_selection->Get_Object_Contour(frame, roi));
    tracker->init(frame, roi);
    while (video.read(frame))
    {
        frame_number++;
        rectangle(frame, roi, Scalar(0, 255, 0));
        imshow("1", frame);
        waitKey(1);
        ok = tracker->update(frame, roi);
        if (ok)
        {
            Contours[frame_number].push_back(obj_selection->Get_Object_Contour(frame, roi));
        }
        else
        {
            int l = 10;
            imshow("1", frame);
            char key = (char)cv::waitKey();
            while (video.read(frame) && key != 'y')
            {
                frame_number++;
                l++;
                if (l % 10 == 0)
                {
                    imshow("1", frame);
                    key = (char)cv::waitKey();
                }
            }
            if (key == 'e')
                return 1;
            if (key == 'y')
            {
                Mat img;
                resize(frame, img, Size(frame.cols / 2, frame.rows / 2));
                roi = selectROI("select object", img);
                roi.x *= 2;
                roi.y *= 2;
                roi.height *= 2;
                roi.width *= 2;
                Contours[frame_number].push_back(obj_selection->Get_Object_Contour(frame, roi));
                tracker = Init();
                tracker->init(frame, roi);
            }
            else return 1;
        }
    }
    return 1;
}

int EachFrameFacesTracker::Tracking(string VideoName, Object_Selection* obj_selection, vector<vector<vector<Point>>>& Contours)
{
    Mat frame;
    VideoCapture video(VideoName);
    int num, frame_number = 0;
    vector<Rect2d> frames;
    Contours = vector<vector<vector<Point>>>(video.get(CAP_PROP_FRAME_COUNT));
    while (video.read(frame))
    {
        cout << frame_number << endl;
        frames = obj_selection->Get_Enlarged_Frames(frame, object_frame_increase_x, object_frame_increase_y);
        Contours[frame_number] = obj_selection->Get_All_Contours(frame, frames);
        frame_number++;
    }
    return 1;
}