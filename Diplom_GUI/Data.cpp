#include "pch.h"
#include <fstream>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <cmath>
#include "Data.h"

using namespace std;
using namespace cv;

void Data::ColumnsInit(const string& s)
{
	stringstream temp(s);
	string item;
	int n = 0;
	while (getline(temp, item, ';'))
	{
		if (item == "RecordingTimestamp")
			ColRecordingTimestamp = n;
		else if (item == "EyePosLeftX (ADCSmm)")
			ColEyePosLeftX = n;
		else if (item == "EyePosLeftY (ADCSmm)")
			ColEyePosLeftY = n;
		else if (item == "EyePosLeftZ (ADCSmm)")
			ColEyePosLeftZ = n;
		else if (item == "EyePosRightX (ADCSmm)")
			ColEyePosRightX = n;
		else if (item == "EyePosRightY (ADCSmm)")
			ColEyePosRightY = n;
		else if (item == "EyePosRightZ (ADCSmm)")
			ColEyePosRightZ = n;
		else if (item == "GazePointX (ADCSpx)")
			ColGazePointXmm = n;
		else if (item == "GazePointY (ADCSpx)")
			ColGazePointYmm = n;
		else if (item == "GazePointX (MCSpx)")
			ColGazePointXpx = n;
		else if (item == "GazePointY (MCSpx)")
			ColGazePointYpx = n;
		else if (item == "")
			break;
		n++;
	}
	ColumnMaxNum = max(max(ColEyePosRightZ, ColGazePointYmm), ColRecordingTimestamp);
}

void Data::Read(const string& s)
{
	stringstream temp(s);
	raw_data read_data;
	string item;
	float left_x = 0, right_x = 0, left_y = 0, right_y = 0, left_z = 0, right_z = 0;
	int count = 0, n = 0;
	while (getline(temp, item, ';'))
	{
		if (n == ColRecordingTimestamp && item != "")
		{
			read_data.Timestamp = stoi(item);
			count++;
		}
		else if (n == ColGazePointXpx && item != "")
		{
			read_data.GazePointXpx = stoi(item);
			count++;
		}
		else if (n == ColGazePointYpx && item != "")
		{
			read_data.GazePointYpx = stoi(item);
			count++;
		}
		else if (n == ColGazePointXmm && item != "")
		{
			read_data.GazePointXmm = stoi(item);
			count++;
		}
		else if (n == ColGazePointYmm && item != "")
		{
			read_data.GazePointYmm = stoi(item);
			count++;
		}
		else if (n == ColEyePosLeftX && item != "")
			left_x = stoi(item);
		else if (n == ColEyePosLeftY && item != "")
			left_y = stoi(item);
		else if (n == ColEyePosRightX && item != "")
			right_x = stoi(item);
		else if (n == ColEyePosRightY && item != "")
			right_y = stoi(item);
		else if (n == ColEyePosLeftZ && item != "")
			left_z = stoi(item);
		else if (n == ColEyePosRightZ && item != "")
			right_z = stoi(item);
		else if (n > ColumnMaxNum)
			break;
		n++;
	}
	if (left_x != 0 && right_x != 0)
	{
		read_data.EyePosX = (left_x + right_x) / 2;
		count++;
	}
	if (left_y != 0 && right_y != 0)
	{
		read_data.EyePosY = (left_y + right_y) / 2;
		count++;
	}
	if (left_z != 0 && right_z != 0)
	{
		read_data.EyePosZ = (left_z + right_z) / 2;
		count++;
	}
	if (count == 8)
		GazeData.push_back(read_data);
}

Data::Data(string Filename, alg Algorithm, double IVT_velocity, int IDT_time_lenght, double IDT_dispersion)
{
	this->Filename = Filename;
	this->Algorithm = Algorithm;
	if (Algorithm == IDT_alg)
	{
		idt_dispersion = IDT_dispersion;
		idt_timelenght = IDT_time_lenght;
	}
	else
		ivt_velocity = IVT_velocity;
	ifstream File(Filename);
	string str;
	getline(File, str);
	ColumnsInit(str);
	while (getline(File, str))
		Read(str);
	File.close();
}

double Data::Velocity(raw_data p1, raw_data p2)
{
	double cos_a, r1, r2, v;
	r1 = sqrt((p1.EyePosX - p1.GazePointXmm) * (p1.EyePosX - p1.GazePointXmm) + (p1.EyePosY - p1.GazePointYmm) * (p1.EyePosY - p1.GazePointYmm) + p1.EyePosZ * p1.EyePosZ);
	r2 = sqrt((p2.EyePosX - p2.GazePointXmm) * (p2.EyePosX - p2.GazePointXmm) + (p2.EyePosY - p2.GazePointYmm) * (p2.EyePosY - p2.GazePointYmm) + p2.EyePosZ * p2.EyePosZ);
	cos_a = ((p1.EyePosX - p1.GazePointXmm) * (p2.EyePosX - p2.GazePointXmm) + (p1.EyePosY - p1.GazePointYmm) * (p2.EyePosY - p2.GazePointYmm) + p1.EyePosZ * p2.EyePosZ) / (r1 * r2);
	v = acos(cos_a) * pow(10, 5) / (p2.Timestamp - p1.Timestamp);
	return v;
}

double Data::Get_Angle(raw_data p1, raw_data p2)
{
	double cos_a, r1, r2;
	r1 = sqrt((p1.EyePosX - p1.GazePointXmm) * (p1.EyePosX - p1.GazePointXmm) + (p1.EyePosY - p1.GazePointYmm) * (p1.EyePosY - p1.GazePointYmm) + p1.EyePosZ * p1.EyePosZ);
	r2 = sqrt((p2.EyePosX - p2.GazePointXmm) * (p2.EyePosX - p2.GazePointXmm) + (p2.EyePosY - p2.GazePointYmm) * (p2.EyePosY - p2.GazePointYmm) + p2.EyePosZ * p2.EyePosZ);
	cos_a = ((p1.EyePosX - p1.GazePointXmm) * (p2.EyePosX - p2.GazePointXmm) + (p1.EyePosY - p1.GazePointYmm) * (p2.EyePosY - p2.GazePointYmm) + p1.EyePosZ * p2.EyePosZ) / (r1 * r2);
	return acos(cos_a) * 20;
}

fix_data Data::Get_Fixation_IVT(int i)
{
	fix_data fix;
	fix.first_point = 0; fix.last_point = 0;
	if (i >= GazeData.size() - 1)
		return fix;
	fix.first_point = i;
	fix.start_time = GazeData[i].Timestamp;
	raw_data p1 = GazeData[i], p2 = GazeData[i + 1];
	int count = 1, sum_x = p1.GazePointXpx, sum_y = p1.GazePointYpx;
	double v = Velocity(p1, p2);
	while (v < ivt_velocity && i + count < GazeData.size() - 1)
	{
		count++;
		sum_x += p2.GazePointXpx;
		sum_y += p2.GazePointYpx;
		p1 = p2;
		p2 = GazeData[i + count];
		v = Velocity(p1, p2);
	}
	fix.x = sum_x / count;
	fix.y = sum_y / count;
	fix.last_point = fix.first_point + count - 1;
	fix.end_time = p1.Timestamp;
	return fix;
}

fix_data Data::Get_Fixation_IDT(int k)
{
	fix_data fix;
	int sum_x = GazeData[k].GazePointXpx, sum_y = GazeData[k].GazePointYpx, first_point = k, last_point = k;
	double r, max_r = 0, cur_disp = 0;
	while (GazeData[last_point].Timestamp - GazeData[first_point].Timestamp < idt_timelenght)
	{
		last_point++;
		if (last_point >= GazeData.size())
			break;
		sum_x += GazeData[last_point].GazePointXpx;
		sum_y += GazeData[last_point].GazePointYpx;
		for (int i = first_point; i < last_point; i++)
		{
			r = sqrt((GazeData[last_point].GazePointXmm - GazeData[i].GazePointXmm) * (GazeData[last_point].GazePointXmm - GazeData[i].GazePointXmm) + (GazeData[last_point].GazePointYmm - GazeData[i].GazePointYmm) * (GazeData[last_point].GazePointYmm - GazeData[i].GazePointYmm));
			if (r > max_r)
			{
				cur_disp = Get_Angle(GazeData[i], GazeData[last_point]);
				if (cur_disp < idt_dispersion)
					max_r = r;
				else
				{
					first_point++;
					last_point = first_point;
					sum_x = GazeData[k].GazePointXpx;
					sum_y = GazeData[k].GazePointYpx;
					max_r = 0;
					break;
				}
			}
		}
	}
	fix.first_point = first_point;
	fix.start_time = GazeData[first_point].Timestamp;
	while (cur_disp < idt_dispersion && last_point < GazeData.size() - 1)
	{
		sum_x += GazeData[last_point].GazePointXpx;
		sum_y += GazeData[last_point].GazePointYpx;
		last_point++;
		for (int i = first_point; i < last_point; i++)
		{
			r = sqrt((GazeData[last_point].GazePointXmm - GazeData[i].GazePointXmm) * (GazeData[last_point].GazePointXmm - GazeData[i].GazePointXmm) + (GazeData[last_point].GazePointYmm - GazeData[i].GazePointYmm) * (GazeData[last_point].GazePointYmm - GazeData[i].GazePointYmm));
			if (r > max_r)
			{
				cur_disp = Get_Angle(GazeData[i], GazeData[last_point]);
				if (cur_disp < idt_dispersion)
					max_r = r;
			}
		}
	}
	fix.last_point = last_point - 1;
	fix.end_time = GazeData[last_point - 1].Timestamp;
	fix.x = sum_x / (last_point - 1 - first_point);
	fix.y = sum_y / (last_point - 1 - first_point);
	return fix;
}

vector<data_for_use> Data::Get_fixations(double Fps)
{
	vector<data_for_use> res;
	data_for_use temp_res;
	double frame_time = 1000 / Fps, start_time = GazeData[0].Timestamp;
	fix_data fix;
	int i = 0;
	while (i < GazeData.size() - 2)
	{
		if (Algorithm == IDT_alg)
			fix = Get_Fixation_IDT(i);
		else
			fix = Get_Fixation_IVT(i);
		i = fix.last_point + 1;
		if (fix.last_point - fix.first_point > 5)
		{
			temp_res.x = fix.x;
			temp_res.y = fix.y;
			temp_res.first_frame = (int)((fix.start_time - GazeData[0].Timestamp) / frame_time);
			temp_res.last_frame = (int)((fix.end_time - GazeData[0].Timestamp) / frame_time);
			res.push_back(temp_res);
		}
	}
	return res;
}

int Data::Show(VideoCapture video)
{
	Mat frame;
	bool ok = video.read(frame);
	int frame_number = 0, frame_width = video.get(CAP_PROP_FRAME_WIDTH), frame_height = video.get(CAP_PROP_FRAME_HEIGHT);
	int fps = video.get(CAP_PROP_FPS);
	VideoWriter video_write("out.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, Size(frame_width, frame_height));
	vector<data_for_use> Data = Get_fixations(fps);
	for (int i = 0; i < Data.size(); i++)
	{
		while (Data[i].first_frame > frame_number)
		{
			video_write.write(frame);
			frame_number++;
			if (!video.read(frame))
			{
				video_write.release();
				return 1;
			}
		}
		while (frame_number <= Data[i].last_frame)
		{
			circle(frame, Point(Data[i].x, Data[i].y), 2 * min((frame_number - Data[i].first_frame), 50), Scalar(0, 255, 0), -1);
			video_write.write(frame);
			if (!video.read(frame))
			{
				video_write.release();
				return 1;
			}
			frame_number++;
		}
	}
	video_write.release();
	return 1;
}
