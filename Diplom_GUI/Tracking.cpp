#include "pch.h"
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include <fstream>
#include "Tracking.h"

using namespace std;
using namespace cv;

MyTracker::MyTracker(string TrackerType, double frame_incr_x, double frame_incr_y)
{
    this->TrackerType = TrackerType;
    Masks = vector<vector<vector<int>>>(0);
    Obj_Frames = vector<vector<Rect2d>>(0);
    object_frame_increase_x = frame_incr_x;
    object_frame_increase_y = frame_incr_y;
}

Ptr<Tracker> MyTracker::Init()
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

vector<vector<vector<int>>> MyTracker::GetMasks(string VideoName, Object_Selection* obj_selection, bool with_contours)
{
    if (with_contours)
        Tracking(VideoName, obj_selection);
    else
        TrackingOnlyFrames(VideoName, obj_selection);
    return Masks;
}

void FacesTracker::UpdateRois(const Mat& image, vector<Rect2d> faces, vector<Rect2d>& rois, vector<Ptr<Tracker>>& trackers)
{
    double face_x, face_y;
    Ptr<Tracker> track;
    for (int i = 0; i < faces.size(); i++)
    {
        face_x = (faces[i].x + faces[i].width) / 2;
        face_y = (faces[i].y + faces[i].height) / 2;
        if ((face_x - rois[i].x)*(face_x - rois[i].x - rois[i].width) < 0 && (face_y - rois[i].y)*(face_y - rois[i].y - rois[i].height))
        {
            rois.push_back(faces[i]);
            track = Init();
            track->init(image, rois[rois.size() - 1]);
            trackers.push_back(track);
        }
    }
}

int FacesTracker::Tracking(string VideoName, Object_Selection* obj_selection)
{
    Mat frame;
    ofstream f;
    f.open("1.txt");
    VideoCapture video(VideoName);
    bool ok = video.read(frame), isfail = false;
    int num, frame_number = 0;
    Masks = vector<vector<vector<int>>>(video.get(CAP_PROP_FRAME_COUNT));
    Obj_Frames = vector<vector<Rect2d>>(video.get(CAP_PROP_FRAME_COUNT));
 //   CurrentVideoName = VideoName;
    vector<int> failing;
    vector<Ptr<Tracker>> trackers;
    Progress = 0;
    while (obj_selection->Get_Object_Frames(frame).size() == 0)
    {
        frame_number++;
        f << frame_number << endl;
        if (!video.read(frame))
            return 0;
    }
    vector<Rect2d> cur_faces, rois = (object_frame_increase_x == 1 && object_frame_increase_y == 1) ? obj_selection->Get_Object_Frames(frame) : obj_selection->Get_Enlarged_Frames(frame, object_frame_increase_x, object_frame_increase_y);
    Masks[frame_number] = obj_selection->Get_Img_Mask(frame);
    Obj_Frames[frame_number] = rois;
    for (int i = 0; i < rois.size(); i++)
    {
        trackers.push_back(Init());
        trackers[i]->init(frame, rois[i]);
    }
    while (video.read(frame))
    {
        frame_number++;
        f << frame_number << endl;
        if (frame_number == 105)
            frame_number = 105;
        for (int i = 0; i < trackers.size(); i++)
        {
            ok = trackers[i]->update(frame, rois[i]);
            if (!ok)
            {
                //failing.push_back(i);
                isfail = true;
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
        Masks[frame_number] = obj_selection->Get_Img_Mask(frame, rois);
        Obj_Frames[frame_number] = rois;
    }
    f.close();
    return 1;
}

int FacesTracker::TrackingOnlyFrames(string VideoName, Object_Selection* obj_selection)
{
    Mat frame;
    VideoCapture video(VideoName);
    bool ok = video.read(frame);
    int num, frame_number = 0;
    Obj_Frames = vector<vector<Rect2d>>(video.get(CAP_PROP_FRAME_COUNT));
    vector<int> failing;
    vector<Ptr<Tracker>> trackers;
//    CurrentVideoName = VideoName;
    while (obj_selection->Get_Object_Frames(frame).size() == 0)
    {
        frame_number++;
        if (!video.read(frame))
            return 0;
    }
    vector<Rect2d> cur_faces, rois = (object_frame_increase_x == 1 && object_frame_increase_y == 1) ? obj_selection->Get_Object_Frames(frame) : obj_selection->Get_Enlarged_Frames(frame, object_frame_increase_x, object_frame_increase_y);
    Obj_Frames[frame_number] = rois;
    for (int i = 0; i < rois.size(); i++)
    {
        trackers.push_back(Init());
        trackers[i]->init(frame, rois[i]);
    }
    while (video.read(frame))
    {
        frame_number++;
        for (int i = 0; i < trackers.size(); i++)
        {
            ok = trackers[i]->update(frame, rois[i]);
            if (!ok)
                failing.push_back(i);
        }
        while (!failing.empty())
        {
            num = failing[0];
            failing.erase(failing.begin());
            delete trackers[num];
            trackers[num] = nullptr;
            rois[num] = Rect2d(0, 0, 0, 0);
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
        }
        Obj_Frames[frame_number] = rois;
    }
    video.release();
    return 1;
}


int ObjectTracker::Tracking(string VideoName, Object_Selection* obj_selection)
{
    return 0;
}

int ObjectTracker::TrackingOnlyFrames(string VideoName, Object_Selection* obj_selection)
{
    return 0;
}

/*void FacesTracker::DrawMasks(string OutVideoName)
{
    VideoCapture video(CurrentVideoName);
    Mat frame;
    int num_of_frame = 0;
    bool ok = video.read(frame);
    if (!ok)
        return;
    if (OutVideoName == "")
        OutVideoName = CurrentVideoName + "_masks.avi";
    int frame_width = video.get(CAP_PROP_FRAME_WIDTH);
    int frame_height = video.get(CAP_PROP_FRAME_HEIGHT);
    int fps = video.get(CAP_PROP_FPS);
    VideoWriter video_write(OutVideoName, VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, Size(frame_width, frame_height));
    while (video.read(frame))
    {
        Object_Selection::Draw_Mask(frame, Masks[num_of_frame]);
        video_write.write(frame);
        num_of_frame++;
    }
    video_write.release();
    video.release();
}

void FacesTracker::DrawFrames(string OutVideoName)
{
    VideoCapture video(CurrentVideoName);
    Mat frame;
    int num_of_frame = 0;
    bool ok = video.read(frame);
    if (!ok)
        return;
    if (OutVideoName == "")
        OutVideoName = CurrentVideoName + "_masks.avi";
    int frame_width = video.get(CAP_PROP_FRAME_WIDTH);
    int frame_height = video.get(CAP_PROP_FRAME_HEIGHT);
    int fps = video.get(CAP_PROP_FPS);
    VideoWriter video_write(OutVideoName, VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, Size(frame_width, frame_height));
    while (video.read(frame))
    {
        Object_Selection::Draw_Frames(frame, Obj_Frames[num_of_frame]);
        video_write.write(frame);
        num_of_frame++;
    }
    video_write.release();
    video.release();
}*/