#include "pch.h"
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <algorithm>
#include "Video_Handling.h"


using namespace std;
using namespace cv;

VideoHandling::VideoHandling(string VideoFileName, alg gaze_fixation_algorithm, MyTracker* tracker, Object_Selection* obj_select, double IVT_velocity, int IDT_time_lenght, double IDT_dispersion)
{
	fixation_algorithm = gaze_fixation_algorithm;
	Contours = vector<vector<vector<Point>>>(0);
	this->VideoFileName = VideoFileName;
	ivt_velocity = IVT_velocity;
	idt_dispersion = IDT_dispersion;
	idt_time_lenght = IDT_time_lenght;
	VideoCapture video(VideoFileName);
	frame_width = video.get(CAP_PROP_FRAME_WIDTH);
	frame_height = video.get(CAP_PROP_FRAME_HEIGHT);
	fps = video.get(CAP_PROP_FPS);
	video.release();
	tracker->GetObjects(VideoFileName, obj_select, Contours);
	MasksInit();
}

VideoHandling::VideoHandling(string FileName, alg gaze_fixation_algorithm, double IVT_velocity, int IDT_time_lenght, double IDT_dispersion)
{
	fixation_algorithm = gaze_fixation_algorithm;
	ivt_velocity = IVT_velocity;
	idt_dispersion = IDT_dispersion;
	idt_time_lenght = IDT_time_lenght;
	ifstream fin;
	fin.open(FileName);
	fin >> VideoFileName;
	int num_of_frames, num_of_contours_in_k_frame, num_of_points_in_j_contour, x, y;
	fin >> num_of_frames;
	VideoCapture video(VideoFileName);
	frame_width = video.get(CAP_PROP_FRAME_WIDTH);
	frame_height = video.get(CAP_PROP_FRAME_HEIGHT);
	fps = video.get(CAP_PROP_FPS);
	video.release();
	Contours = vector<vector<vector<Point>>>(num_of_frames);
	for (int k = 0; k < num_of_frames; k++)
	{
		fin >> num_of_contours_in_k_frame;
		Contours[k] = vector<vector<Point>>(num_of_contours_in_k_frame);
		for (int j = 0; j < num_of_contours_in_k_frame; j++)
		{
			fin >> num_of_points_in_j_contour;
			Contours[k][j] = vector<Point>(num_of_points_in_j_contour);
			for (int i = 0; i < num_of_points_in_j_contour; i++)
			{
				fin >> x >> y;
				Contours[k][j][i] = Point(x, y);
			}
		}
	}
	fin.close();
	MasksInit();
}

void VideoHandling::MasksInit()
{
	Masks = vector<Mat>(Contours.size());
	for (int i = 0; i < Masks.size(); i++)
		Masks[i] = Object_Selection::Get_Img_Mask(frame_height, frame_width, Contours[i]);
}

void VideoHandling::SaveContoursInFile(string OutFileName)
{
	if (OutFileName == "")
		OutFileName = VideoFileName + "_contours.txt";
	ofstream fout;
	fout.open(OutFileName);
	fout << VideoFileName << endl << Contours.size() << endl << frame_width << frame_height << endl;
	for (int i = 0; i < Contours.size(); i++)
	{
		fout << Contours[i].size() << endl;
		for (int j = 0; j < Contours[i].size(); j++)
		{
			fout << Contours[i][j].size() << " ";
			for (int k = 0; k < Contours[i][j].size() - 1; k++)
				fout << Contours[i][j][k].x << " " << Contours[i][j][k].y << " ";
			fout << Contours[i][j][Contours[i][j].size() - 1].x << " " << Contours[i][j][Contours[i][j].size() - 1].y << endl;
		}
	}
	fout.close();
}

vector<ClassifiedFixation> VideoHandling::ÑlassifyFixation(string DataFilename)
{
	Mat frame;
	vector<ClassifiedFixation> res;
	Data MyData = Data(DataFilename, fixation_algorithm, ivt_velocity, idt_time_lenght, idt_dispersion);
	vector<data_for_use> fixations = MyData.Get_fixations(fps);
	int good;
	for (int i = 0; i < fixations.size(); i++)
	{
		good = 0;
		for (int j = fixations[i].first_frame; j <= fixations[i].last_frame; j++)
		{
			if (j < Masks.size() && Masks[j].cols > fixations[i].x && Masks[j].rows > fixations[i].y && Masks[j].at<uchar>(fixations[i].y, fixations[i].x) == 1)
				good++;
		}
		if (good > (fixations[i].last_frame - fixations[i].first_frame) / 3)
			res.push_back(ClassifiedFixation(1, fixations[i]));
		else
			res.push_back(ClassifiedFixation(0, fixations[i]));
	}
	return res;
}

void VideoHandling::NumOfGoodFixations(string DataFileName, int& goodfixations, int& allfixations)
{
	vector<ClassifiedFixation> fixations = ÑlassifyFixation(DataFileName);
	int good = 0;
	for (int i = 0; i < fixations.size(); i++)
		if (fixations[i].fix_class == 1)
			good++;
	goodfixations = good;
	allfixations = fixations.size();
}

int VideoHandling::DrawFixations(string DataFileName, string OutVideoFile)
{
	Mat frame;
	Scalar Color;
	VideoCapture video(VideoFileName);
	bool ok = video.read(frame);
	if (!ok)
		return 0;
	int frame_number = 0, frame_width = video.get(CAP_PROP_FRAME_WIDTH), frame_height = video.get(CAP_PROP_FRAME_HEIGHT);
	int fps = video.get(CAP_PROP_FPS), num_of_good_fix = 0, num_of_all_fix = 0;
	VideoWriter video_write(OutVideoFile, VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, Size(frame_width, frame_height));
	vector<ClassifiedFixation> fixations = ÑlassifyFixation(DataFileName);
	for (int i = 0; i < fixations.size(); i++)
	{
		if (fixations[i].fix_class == 0)
			Color = Scalar(0, 0, 255);
		else
		{
			Color = Scalar(0, 255, 0);
			num_of_good_fix++;
		}
		while (fixations[i].fix.first_frame > frame_number)
		{
			video_write.write(frame);
			frame_number++;
			if (!video.read(frame))
			{
				video_write.release();
				return 0;
			}
		}
		while (frame_number <= fixations[i].fix.last_frame)
		{
			putText(frame, "Number of fixations on the object: " + to_string(num_of_good_fix), Point(40, frame_height - 90), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(255, 0, 200), 2);
			putText(frame, "Total fixations: " + to_string(i + 1), Point(40, frame_height - 60), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(255, 0, 200), 2);
			circle(frame, Point(fixations[i].fix.x, fixations[i].fix.y), 2 * min((frame_number - fixations[i].fix.first_frame), 75), Color, -1);
			video_write.write(frame);
			if (!video.read(frame))
			{
				video_write.release();
				return 0;
			}
			frame_number++;
		}
	}
	video_write.release();
	return 1;
}
