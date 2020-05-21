
// Diplom_GUIDlg.cpp: файл реализации
//

#include "pch.h"
#include "framework.h"
#include "Video_Handling.h"
#include "EyeFixationsHandler.h"
#include "EyeFixationsHandlerDlg.h"
#include "afxdialogex.h"
#include <iostream>
#include <string>

using namespace std;
using namespace cv;

VideoCapture viddeo;
Mat image;
Object_Selection* Obj_Select = new Object_Selection(None, None);
MyTracker* tracker = new MyTracker(WithoutTracking);
VideoHandling* myvideo = 0;
alg FixAlg = IVT_alg;
Algorithms cur_select=None, cur_crop = None, cur_tracker = WithoutTracking;
CString FileName;
string dataname = "", filename = "";
bool IsItFirst = true, IsAlgorithmsChange = false, IsVideoNameChanged = true;
double ivt_velocity = 110, idt_dispersion = 0.8;
int idt_minlenght = 100;
int good_fix = 0, all_fix = 0;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Диалоговое окно CDiplomGUIDlg




CDiplomGUIDlg::CDiplomGUIDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIPLOM_GUI_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDiplomGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, Button1);
	DDX_Control(pDX, IDC_EDIT1, FileName);
	//  DDX_Control(pDX, IDC_CHECK1, IsItPicture);
	DDX_Control(pDX, IDC_COMBO2, AlgorithmName);
	DDX_Control(pDX, IDC_COMBO3, CropAlgName);
	DDX_Control(pDX, IDC_COMBO1, Act);
	DDX_Control(pDX, IDC_EDIT3, DataFilename);
	DDX_Control(pDX, IDC_EDIT4, OutFilename);
	//  DDX_Control(pDX, IDC_CHECK2, TrackingType);
}

BEGIN_MESSAGE_MAP(CDiplomGUIDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
//	ON_CBN_SELCHANGE(IDC_COMBO1, &CDiplomGUIDlg::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON1, &CDiplomGUIDlg::OnBnClickedButton1)
//	ON_EN_CHANGE(IDC_EDIT1, &CDiplomGUIDlg::OnEnChangeEdit1)
//	ON_CBN_SELCHANGE(IDC_COMBO2, &CDiplomGUIDlg::OnCbnSelchangeCombo2)
//	ON_CBN_SELCHANGE(IDC_COMBO3, &CDiplomGUIDlg::OnCbnSelchangeCombo3)
	ON_BN_CLICKED(IDC_BUTTON2, &CDiplomGUIDlg::OnBnClickedButton2)
//	ON_BN_CLICKED(IDC_CHECK2, &CDiplomGUIDlg::OnBnClickedCheck2)
//	ON_BN_CLICKED(IDC_BUTTON3, &CDiplomGUIDlg::OnBnClickedButton3)
//	ON_BN_CLICKED(IDC_BUTTON4, &CDiplomGUIDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDOK, &CDiplomGUIDlg::OnBnClickedOk)
//	ON_BN_CLICKED(IDCANCEL, &CDiplomGUIDlg::OnBnClickedCancel)
ON_CBN_SELCHANGE(IDC_COMBO1, &CDiplomGUIDlg::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()


// Обработчики сообщений CDiplomGUIDlg

BOOL CDiplomGUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();


	SetIcon(m_hIcon, TRUE);			
	SetIcon(m_hIcon, FALSE);		



	return TRUE;  
}


void CDiplomGUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); 

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);


		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;


		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CDiplomGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CDiplomGUIDlg::OnCbnSelchangeCombo1()
{
	Button1.EnableWindow(true);
}

void CDiplomGUIDlg::AlgorithmInit()
{
	if (CropAlgName.GetCurSel() == 0)
		FixAlg = IVT_alg;
	else if (CropAlgName.GetCurSel() == 1)
		FixAlg = IDT_alg;
	
	if (AlgorithmName.GetCurSel() == 0)
	{
		if (cur_select != DNN || cur_crop != Marks)
		{
			delete Obj_Select;
			Obj_Select = new Object_Selection(DNN, Marks);
			cur_crop = Marks;
			cur_select = DNN;
		}
		if (cur_tracker != WithoutTracking)
		{
			delete tracker;
			tracker = new MyTracker(WithoutTracking);
			cur_tracker = WithoutTracking;
		}
	}
	else if (AlgorithmName.GetCurSel() == 1)
	{
		if (cur_select != DNN || cur_crop != Marks)
		{
			delete Obj_Select;
			Obj_Select = new Object_Selection(DNN, Marks);
			cur_crop = Marks;
			cur_select = DNN;
		}
		if (cur_tracker != FaceTrack)
		{
			delete tracker;
			tracker = new MyTracker(FaceTrack);
			cur_tracker = FaceTrack;
		}
	}
	else if (AlgorithmName.GetCurSel() == 2)
	{
		if (cur_select != None || cur_select != None)
		{
			delete Obj_Select;
			Obj_Select = new Object_Selection(None, None);
			cur_select = None;
			cur_crop = None;
		}
		if (cur_tracker != ObjectTrack)
		{
			delete tracker;
			tracker = new MyTracker(ObjectTrack);
			cur_tracker = ObjectTrack;
		}
	}
	IsAlgorithmsChange = true;
}

void CDiplomGUIDlg::TrackerInit()
{
	/*delete tracker;
	if (!TrackingType.GetCheck())
		tracker = new MyTracker(FaceTrack);
	else
		tracker = new MyTracker(FaceTrack);
	NeedsNewInit = true;*/
}

void CDiplomGUIDlg::OnBnClickedButton1()
{
	CString video, data, outfile;
	GetDlgItemText(IDC_EDIT1, video);
	GetDlgItemText(IDC_EDIT3, data);
	string cur_outfile, cur_video = string(CStringA{ video }), cur_data = string(CStringA{ data });
	if (video.GetLength() == 0)
	{
		MessageBox(CString("Please input the filename!"));
		return;
	}
	if (filename != cur_video || IsAlgorithmsChange)
	{
		filename = cur_video;
		if(cur_video.rfind(".txt") == cur_video.length() - 4)
		{
			delete myvideo;
			myvideo = new VideoHandling(cur_video, FixAlg, ivt_velocity, idt_minlenght, idt_dispersion);
		}
		else
		{
			if (IsItFirst && !IsAlgorithmsChange)
			{
				cur_select = DNN;
				cur_crop = Marks;
				cur_tracker = WithoutTracking;
				delete Obj_Select;
				delete tracker;
				Obj_Select = new Object_Selection(cur_select, cur_crop);
				tracker = new MyTracker(cur_tracker);
			}
			if (!viddeo.open(cur_video))
			{
				MessageBox(CString("Wrong format of videofilename!"));
				return;
			}
			MessageBox(CString("Please wait! I handle the videofile!"));
			viddeo.release();
			delete myvideo;
			myvideo = new VideoHandling(cur_video, FixAlg, tracker, Obj_Select, ivt_velocity, idt_minlenght, idt_dispersion);
		}
		IsAlgorithmsChange = false;
	}
	switch (Act.GetCurSel()) {
	case 0:
		GetDlgItemText(IDC_EDIT4, outfile);
		cur_outfile = string(CStringA{ outfile });
		if (cur_outfile.rfind(".txt") != cur_outfile.length() - 4)
		{
			MessageBox(CString("wrong outfile format: please use .txt"));
			return;
		}
		myvideo->SaveContoursInFile(cur_outfile);
		break;
	case 1:
		if (cur_data.rfind(".csv") != cur_data.length() - 4)
		{
			MessageBox(CString("Couldn't open file with data!"));
			return;
		}
		myvideo->NumOfGoodFixations(cur_data, good_fix, all_fix);
		MessageBox(CString(("num of good fixations = " + to_string(good_fix) + "; total: " + to_string(all_fix)).c_str()));
		break;
	case 2:
		GetDlgItemText(IDC_EDIT4, outfile);
		cur_outfile = string(CStringA{ outfile });
		if (cur_outfile.rfind(".avi") != cur_outfile.length() - 4)
		{
			MessageBox(CString("wrong outfile format: please use .avi"));
			return;
		}
		myvideo->DrawFixations(cur_data, cur_outfile);
		break;
	default:
		break;
	}
}





void CDiplomGUIDlg::OnBnClickedButton2()
{
	AlgorithmInit();
}


//void CDiplomGUIDlg::OnBnClickedCheck2()
//{
//	TrackerInit();
//}


//void CDiplomGUIDlg::OnBnClickedButton3()
//{
//	// TODO: добавьте свой код обработчика уведомлений
//
//}


//void CDiplomGUIDlg::OnBnClickedButton4()
//{
//
//}


void CDiplomGUIDlg::OnBnClickedOk()
{

	delete myvideo;
	delete tracker;
	delete Obj_Select;
	CDialogEx::OnOK();
	
}


