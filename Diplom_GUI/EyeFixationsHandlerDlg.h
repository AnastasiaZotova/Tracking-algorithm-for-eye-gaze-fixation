
// Diplom_GUIDlg.h
//

#pragma once


// CDiplomGUIDlg
class CDiplomGUIDlg : public CDialogEx
{

public:
	CDiplomGUIDlg(CWnd* pParent = nullptr);


#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIPLOM_GUI_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// поддержка DDX/DDV

protected:
	HICON m_hIcon;


	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CButton Button1;
//	afx_msg void OnCbnSelchangeCombo1();
	CEdit FileName;
//	CButton IsItPicture;
	afx_msg void OnBnClickedButton1();
	CComboBox AlgorithmName;
	CComboBox CropAlgName;
	CComboBox Act;
//	afx_msg void OnEnChangeEdit1();
//	afx_msg void OnCbnSelchangeCombo2();
//	afx_msg void OnCbnSelchangeCombo3();
	afx_msg void OnBnClickedButton2();
	CEdit DataFilename;
	CEdit OutFilename;
	void AlgorithmInit();
	void TrackerInit();
//	CButton TrackingType;
//	afx_msg void OnBnClickedCheck2();
//	afx_msg void OnBnClickedButton3();
//	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedOk();
//	afx_msg void OnBnClickedCancel();
	afx_msg void OnCbnSelchangeCombo1();
};
