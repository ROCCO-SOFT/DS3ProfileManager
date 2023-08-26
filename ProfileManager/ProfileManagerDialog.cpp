#include "pch.h"
#include "framework.h"
#include "ProfileManager.h"
#include "ProfileManagerDialog.h"
#include "afxdialogex.h"
#include "Profile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TID_AUTO_SAVE	0xffff

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	PBYTE GetVerInf(LPCTSTR pszFileName);
	CString GetFileVer(LPCTSTR pszFileName);
};

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

CAboutDlg::CAboutDlg()
	: CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString strFileName;
	GetModuleFileName(NULL, strFileName.GetBuffer(MAX_PATH), MAX_PATH);
	strFileName.ReleaseBuffer();

	CString strVerFormat;
	GetDlgItem(IDC_VER_STATIC)->GetWindowText(strVerFormat);
	CString strVer;
	strVer.Format(strVerFormat, APP_NAME, GetFileVer(strFileName));
	GetDlgItem(IDC_VER_STATIC)->SetWindowText(strVer);

	return TRUE;
}


PBYTE CAboutDlg::GetVerInf(LPCTSTR pszFileName)
{
	int nVerInfoSize = GetFileVersionInfoSize((LPTSTR)pszFileName, 0);
	if (!nVerInfoSize) {
		return NULL;
	}
	LPBYTE pVerInf = new BYTE[nVerInfoSize];
	if (!pVerInf) {
		return NULL;
	}
	if (!GetFileVersionInfo((LPTSTR)pszFileName, NULL, nVerInfoSize, pVerInf)) {
		delete[] pVerInf;
		return NULL;
	}
	return pVerInf;
}

CString CAboutDlg::GetFileVer(LPCTSTR pszFileName)
{
	CString strVer;
	PBYTE pVerInf = GetVerInf(pszFileName);
	if (pVerInf) {
		VS_FIXEDFILEINFO *pInfo;
		UINT len;
		if (VerQueryValue(pVerInf, _T("\\"), (LPVOID *)&pInfo, &len)) {
			strVer.Format(_T("%d, %d, %d, %d"),
				HIWORD(pInfo->dwFileVersionMS),
				LOWORD(pInfo->dwFileVersionMS),
				HIWORD(pInfo->dwFileVersionLS),
				LOWORD(pInfo->dwFileVersionLS));
		}
		delete[] pVerInf;
	}
	return strVer;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CProfileManagerDialog, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_BN_CLICKED(IDC_SAVE_BUTTON, &CProfileManagerDialog::OnBnClickedSaveButton)
	ON_BN_CLICKED(IDC_LOAD_BUTTON, &CProfileManagerDialog::OnBnClickedLoadButton)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_OVERWRITE_BUTTON, &CProfileManagerDialog::OnBnClickedOverwriteButton)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PROFILE_LIST, &CProfileManagerDialog::OnLvnItemchangedProfileList)
	ON_BN_CLICKED(IDC_REMOVE_BUTTON, &CProfileManagerDialog::OnBnClickedRemoveButton)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_PROFILE_LIST, &CProfileManagerDialog::OnLvnEndlabeleditProfileList)
	ON_WM_GETMINMAXINFO()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_NOTIFY(HDN_ITEMCLICK, 0, &CProfileManagerDialog::OnHdnItemclickProfileList)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_PROFILE_LIST, &CProfileManagerDialog::OnLvnBeginDragProfileList)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

CProfileManagerDialog::CProfileManagerDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PROFILE_MANAGER, pParent)
	, m_bEnableAutosave(TRUE)
	, m_nAutosaveMinInterval(1000 * 60)
	, m_bHideAutosaved(FALSE)
	, m_nSort(SORT_USER)
	, m_nMaxAutosaves(300)
{

}

void CProfileManagerDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROFILE_LIST, m_listProfile);
}

BOOL CProfileManagerDialog::InsertListColumn(CListCtrl *pList, int nCol, UINT nCaptionID, int nFormat, int nWidth, int nSubItem)
{
	CString strCaption;
	strCaption.LoadString(nCaptionID);
	return pList->InsertColumn(nCol, strCaption, nFormat, nWidth, nSubItem);
}

BOOL CProfileManagerDialog::InitList()
{
	HRESULT hr = S_OK;

	POSITION pos = m_listProfile.GetFirstSelectedItemPosition();
	int nItem = -1;
	if (pos) {
		nItem = m_listProfile.GetNextSelectedItem(pos);
	}

	m_listProfile.DeleteAllItems();

	CComPtr <CBackupEnum> pEnum;
	hr = m_pContext->GetCurrentBackupSet()->EnumBackup(&pEnum);
	if (FAILED(hr)) {
		return FALSE;
	}

	CComPtr <CBackup> pBackup;
	while (pEnum->Next(&pBackup) == S_OK) {
		if (!m_bHideAutosaved || !pBackup->IsAutosaved()) {
			InsertItem(-1, pBackup);
		}
		pBackup.Release();
	}

	m_listProfile.EnsureVisible(nItem, FALSE);

	return TRUE;
}

BOOL CProfileManagerDialog::OnInitDialog()
{
	HRESULT hr = S_OK;

	CDialogEx::OnInitDialog();

	// 初期のサイズが最小サイズ。
	GetWindowRect(&m_rcClientMin);

	//
	// load settings
	//

	CRect rc;
	GetWindowRect(&rc);
	rc.left = AfxGetApp()->GetProfileInt(_T("Settings"), _T("MainWindow.Left"), rc.left);
	rc.top = AfxGetApp()->GetProfileInt(_T("Settings"), _T("MainWindow.Top"), rc.top);
	rc.right = AfxGetApp()->GetProfileInt(_T("Settings"), _T("MainWindow.Right"), rc.right);
	rc.bottom = AfxGetApp()->GetProfileInt(_T("Settings"), _T("MainWindow.Bottom"), rc.bottom);
	SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);

	int nNameWidth = 120;
	int nDateWidth = 140;
	int nStatusWidth = 80;
	int nAutosaveWidth = 80;

	nNameWidth = AfxGetApp()->GetProfileInt(_T("Settings"), _T("HeaderColumn.Name.Width"), nNameWidth);
	nDateWidth = AfxGetApp()->GetProfileInt(_T("Settings"), _T("HeaderColumn.Date.Width"), nDateWidth);
	nStatusWidth = AfxGetApp()->GetProfileInt(_T("Settings"), _T("HeaderColumn.Status.Width"), nStatusWidth);

	m_bEnableAutosave = AfxGetApp()->GetProfileInt(_T("Settings"), _T("Autosave.Enable"), m_bEnableAutosave);
	m_nAutosaveMinInterval = AfxGetApp()->GetProfileInt(_T("Settings"), _T("Autosave.MinInterval"), m_nAutosaveMinInterval);
	m_bHideAutosaved = AfxGetApp()->GetProfileInt(_T("Settings"), _T("Autosave.HideAutosaved"), m_bHideAutosaved);
	m_nMaxAutosaves = AfxGetApp()->GetProfileInt(_T("Settings"), _T("Autosave.MaxAutosaves"), m_nMaxAutosaves);

	m_nSort = AfxGetApp()->GetProfileInt(_T("Settings"), _T("ProfileList.Sort"), m_nSort);

	GetClientRect(&m_rcClient);

	CString strTitleFormat;
	GetWindowText(strTitleFormat);
	CString strTitle;
	strTitle.Format(strTitleFormat, APP_NAME);
	SetWindowText(strTitle);

	// "バージョン情報..." メニューをシステム メニューに追加します。
	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		/////////////////////////////////////////////////////////////////////////////
		pSysMenu->AppendMenu(MF_SEPARATOR);
		/////////////////////////////////////////////////////////////////////////////

		BOOL bNameValid;

		CString strAutosaveMenu;
		bNameValid = strAutosaveMenu.LoadString(IDS_AUTOSAVE);
		ASSERT(bNameValid);
		if (!strAutosaveMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_STRING, IDM_AUTOSAVE, strAutosaveMenu);
			pSysMenu->CheckMenuItem(IDM_AUTOSAVE, m_bEnableAutosave ? MF_CHECKED : MF_UNCHECKED);
		}

		CString strHideAutosavedMenu;
		bNameValid = strHideAutosavedMenu.LoadString(IDS_HIDE_AUTOSAVED);
		ASSERT(bNameValid);
		if (!strAutosaveMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_STRING, IDM_HIDE_AUTOSAVED, strHideAutosavedMenu);
			pSysMenu->CheckMenuItem(IDM_HIDE_AUTOSAVED, m_bHideAutosaved ? MF_CHECKED : MF_UNCHECKED);
		}

		/////////////////////////////////////////////////////////////////////////////
		pSysMenu->AppendMenu(MF_SEPARATOR);
		/////////////////////////////////////////////////////////////////////////////

		CString strReadmeMenu;
		bNameValid = strReadmeMenu.LoadString(IDS_README);
		ASSERT(bNameValid);
		if (!strReadmeMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_STRING, IDM_README, strReadmeMenu);
		}

		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	hr = CreateContext(&m_pContext);
	if (FAILED(hr)) {
		return FALSE;
	}

	m_listProfile.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	InsertListColumn(&m_listProfile, 0, IDS_LIST_HEADER_NAME, 0, nNameWidth);
	InsertListColumn(&m_listProfile, 1, IDS_LIST_HEADER_DATE, 0, nDateWidth);
	InsertListColumn(&m_listProfile, 2, IDS_LIST_HEADER_STATUS, 0, nStatusWidth);

	// init sort icon
	m_listSortIcon.Create(16, 16, ILC_MASK, 2, 2);
	CBitmap bmp;
	bmp.LoadBitmap(IDB_HEADER_SORT_ASC);
	m_listSortIcon.Add(&bmp, RGB(255, 0, 255));
	bmp.DeleteObject();
	bmp.LoadBitmap(IDB_HEADER_SORT_DEC);
	m_listSortIcon.Add(&bmp, RGB(255, 0, 255));
	bmp.DeleteObject();
	CHeaderCtrl* pHeader = m_listProfile.GetHeaderCtrl();
	pHeader->SetImageList(&m_listSortIcon);

	InitList();

	UpdateData(FALSE);

	EnableAutoSave(m_bEnableAutosave);
	UpdateSort(m_nSort);

	return TRUE;
}

int CProfileManagerDialog::InsertItem(int nInsert, CBackup *pBackup)
{
	if (nInsert == -1) {
		nInsert = m_listProfile.GetItemCount();
	}
	int nItem = m_listProfile.InsertItem(nInsert, pBackup->GetName());
	SYSTEMTIME stCreate = pBackup->GetCreateTime();
	CTime tmCreate(stCreate);
	CString strTime;
	strTime = tmCreate.Format(_T("%Y/%m/%d %H:%M:%S"));
	m_listProfile.SetItemText(nItem, 1, strTime);
	m_listProfile.SetItemData(nItem, (DWORD_PTR)pBackup);

//	UpdateSort(m_nSort);
//	nItem = FindItem(pBackup);

	return nItem;
}

int CProfileManagerDialog::UpdateList(int nItem)
{
	CComPtr <CBackup> pBackup;
	pBackup = (CBackup *)m_listProfile.GetItemData(nItem);

	SYSTEMTIME stCreate = pBackup->GetCreateTime();
	CTime tmCreate(stCreate);
	CString strTime;
	strTime = tmCreate.Format(_T("%Y/%m/%d %H:%M:%S"));
	m_listProfile.SetItemText(nItem, 1, strTime);

	return nItem;
}

void CProfileManagerDialog::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_AUTOSAVE)
	{
		EnableAutoSave(!m_bEnableAutosave);
		CMenu* pSysMenu = GetSystemMenu(FALSE);
		pSysMenu->CheckMenuItem(IDM_AUTOSAVE, m_bEnableAutosave ? MF_CHECKED : MF_UNCHECKED);
	}
	else if ((nID & 0xFFF0) == IDM_HIDE_AUTOSAVED)
	{
		HideAutosaved(!m_bHideAutosaved);
		CMenu* pSysMenu = GetSystemMenu(FALSE);
		pSysMenu->CheckMenuItem(IDM_HIDE_AUTOSAVED, m_bHideAutosaved ? MF_CHECKED : MF_UNCHECKED);
	}
	else if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if ((nID & 0xFFF0) == IDM_README)
	{
		TCHAR path[_MAX_PATH];
		GetModuleFileName(NULL, path, _MAX_PATH);
		TCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
		_tsplitpath_s(path, drive, dir, fname, ext);
		CString strReadmeFileName;
		strReadmeFileName.LoadString(IDS_README_FILE_NAME);
		CString strReadmePathName;
		strReadmePathName += drive;
		strReadmePathName += dir;
		strReadmePathName += strReadmeFileName;
		ShellExecute(GetSafeHwnd(), _T("open"), strReadmePathName, NULL, NULL, SW_SHOWNORMAL);
	}
	else 
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CProfileManagerDialog::UpdateLastSave(int nItem)
{
	if (m_pLastStatus) {
		int nItem = FindItem(m_pLastStatus);
		m_listProfile.SetItemText(nItem, 2, _T(""));
	}
	m_listProfile.SetItemText(nItem, 2, _T("Saved"));
	m_pLastStatus = (CBackup *)m_listProfile.GetItemData(nItem);
}

void CProfileManagerDialog::UpdateLastLoad(int nItem)
{
	if (m_pLastStatus) {
		int nItem = FindItem(m_pLastStatus);
		m_listProfile.SetItemText(nItem, 2, _T(""));
	}
	m_listProfile.SetItemText(nItem, 2, _T("Loaded"));
	m_pLastStatus = (CBackup *)m_listProfile.GetItemData(nItem);
}

void CProfileManagerDialog::OnBnClickedSaveButton()
{
	HRESULT hr = S_OK;

	UpdateData();

	CComPtr <CBackup> pBackup;
	hr = m_pContext->GetCurrentBackupSet()->SaveCurrent(&pBackup, NEWSAVE_NAME);
	if (FAILED(hr)) {
		return;
	}

	int nItem = InsertItem(-1, pBackup);
	UpdateLastSave(nItem);

	m_listProfile.SetItemState(nItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
	m_listProfile.EnsureVisible(nItem, FALSE);
	m_listProfile.SetFocus();
	m_listProfile.EditLabel(nItem);
	Sleep(300);
	m_listProfile.GetEditControl()->SetFocus();
}

void CProfileManagerDialog::OnBnClickedOverwriteButton()
{
	HRESULT hr = S_OK;

	POSITION pos = m_listProfile.GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	UINT nPrompt = AfxMessageBox(IDS_CONFIRM_OVERWRITE, MB_OKCANCEL | MB_ICONQUESTION);
	if (nPrompt == IDCANCEL) {
		return;
	}

	//OnBnClickedSaveButton();

	int nItem = m_listProfile.GetNextSelectedItem(pos);
	CComPtr <CBackup> pBackup;
	pBackup = (CBackup *)m_listProfile.GetItemData(nItem);

	hr = pBackup->Save();

	UpdateList(nItem);
	UpdateLastSave(nItem);
}

void CProfileManagerDialog::OnBnClickedLoadButton()
{
	HRESULT hr = S_OK;

	POSITION pos = m_listProfile.GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	UINT nPrompt = AfxMessageBox(IDS_CONFIRM_LOAD, MB_OKCANCEL | MB_ICONQUESTION);
	if (nPrompt == IDCANCEL) {
		return;
	}

	//OnBnClickedSaveButton();

	int nItem = m_listProfile.GetNextSelectedItem(pos);
	CComPtr <CBackup> pBackup;
	pBackup = (CBackup *)m_listProfile.GetItemData(nItem);

	hr = pBackup->Load();

	UpdateLastLoad(nItem);
}

void CProfileManagerDialog::OnBnClickedRemoveButton()
{
	HRESULT hr = S_OK;

	POSITION pos = m_listProfile.GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	UINT nPrompt = AfxMessageBox(IDS_CONFIRM_DELETE, MB_OKCANCEL | MB_ICONQUESTION);
	if (nPrompt == IDCANCEL) {
		return;
	}

	//OnBnClickedSaveButton();

	CWaitCursor wait;
	CBackupList listMove;
	for (int nItem = m_listProfile.GetItemCount() - 1; nItem >= 0; nItem--) {
		if (m_listProfile.GetItemState(nItem, LVIS_SELECTED) == LVIS_SELECTED) {
			CComPtr <CBackup> pBackup;
			pBackup = (CBackup *)m_listProfile.GetItemData(nItem);
			hr = pBackup->Delete();
			if (FAILED(hr)) {
				return;
			}
			m_listProfile.DeleteItem(nItem);
			m_listProfile.RedrawWindow(NULL, NULL, RDW_UPDATENOW);
		}
	}
}

BOOL CProfileManagerDialog::AlignControl(HWND hParent, int cx, int cy, int arID[], int nCount, int nFunc)
{
	for (int n = 0; n < nCount; n++) {
		HWND hWnd = ::GetDlgItem(hParent, arID[n]);
		if (hWnd) {

			CRect rc;
			::GetWindowRect(hWnd, &rc);
			::ScreenToClient(hParent, &rc.TopLeft());
			::ScreenToClient(hParent, &rc.BottomRight());

			switch (nFunc & AC_MOVE_MASK) {
			case AC_MOVE_HV:
				::SetWindowPos(hWnd, NULL, rc.left + cx, rc.top + cy, NULL, NULL, SWP_NOZORDER | SWP_NOSIZE);
				break;
			case AC_MOVE_H:
				::SetWindowPos(hWnd, NULL, rc.left + cx, rc.top, NULL, NULL, SWP_NOZORDER | SWP_NOSIZE);
				break;
			case AC_MOVE_V:
				::SetWindowPos(hWnd, NULL, rc.left, rc.top + cy, NULL, NULL, SWP_NOZORDER | SWP_NOSIZE);
				break;
			}

			switch (nFunc & AC_STRETCH_MASK) {
			case AC_STRETCH_HV:
				::SetWindowPos(hWnd, NULL, NULL, NULL, rc.Width() + cx, rc.Height() + cy, SWP_NOZORDER | SWP_NOMOVE);
				break;
			case AC_STRETCH_H:
				::SetWindowPos(hWnd, NULL, NULL, NULL, rc.Width() + cx, rc.Height(), SWP_NOZORDER | SWP_NOMOVE);
				break;
			case AC_STRETCH_V:
				::SetWindowPos(hWnd, NULL, NULL, NULL, rc.Width(), rc.Height() + cy, SWP_NOZORDER | SWP_NOMOVE);
				break;
			}
		}
	}
	return TRUE;
}

void CProfileManagerDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	CRect rcClient;
	GetClientRect(&rcClient);
	CSize sizeDelta = rcClient.Size() - m_rcClient.Size();
	m_rcClient = rcClient;

	int cxDelta = sizeDelta.cx;
	int cyDelta = sizeDelta.cy;

#if 0	// 横ストレッチ。
	int arStretch_H[] = {
	};

	AlignControl(GetSafeHwnd(), cxDelta, cyDelta, arStretch_H, numof(arStretch_H), AC_STRETCH_H);
#endif

#if 1	// 縦横ストレッチ。
	int arStretch_HV[] = {
		IDC_PROFILE_LIST,
	};

	AlignControl(GetSafeHwnd(), cxDelta, cyDelta, arStretch_HV, numof(arStretch_HV), AC_STRETCH_HV);
#endif

#if 1	// 縦移動。
	int arMove_V[] = {
		IDC_SAVE_BUTTON,
		IDC_OVERWRITE_BUTTON,
		IDC_REMOVE_BUTTON,
	};

	AlignControl(GetSafeHwnd(), cxDelta, cyDelta, arMove_V, numof(arMove_V), AC_MOVE_V);
#endif

#if 1	// 縦横移動。
	int arMove_HV[] = {
		IDC_LOAD_BUTTON,
	};

	AlignControl(GetSafeHwnd(), cxDelta, cyDelta, arMove_HV, numof(arMove_HV), AC_MOVE_HV);
#endif

}

void CProfileManagerDialog::OnLvnItemchangedProfileList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	POSITION pos = m_listProfile.GetFirstSelectedItemPosition();
	GetDlgItem(IDC_OVERWRITE_BUTTON)->EnableWindow(pos != NULL);
	GetDlgItem(IDC_LOAD_BUTTON)->EnableWindow(pos != NULL);
	GetDlgItem(IDC_REMOVE_BUTTON)->EnableWindow(pos != NULL);
		
	*pResult = 0;
}



void CProfileManagerDialog::OnLvnEndlabeleditProfileList(NMHDR *pNMHDR, LRESULT *pResult)
{
	HRESULT hr = S_OK;

	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	if (!pDispInfo->item.pszText) {
		return;
	}

	int nItem = pDispInfo->item.iItem;
	CString strText = pDispInfo->item.pszText;
	strText.Trim(_T(" "));
	const CString strNg = _T("\\/:*?\"<>|");
	if (strText.FindOneOf(strNg) != -1) {
		CString strErr;
		strErr.LoadString(IDS_ERROR_INVALID_CHARACTER);
		strErr += _T("\n") + strNg;
		AfxMessageBox(strErr, MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	CComPtr <CBackup> pBackup;
	pBackup = (CBackup *)m_listProfile.GetItemData(nItem);
	hr = pBackup->SetName(strText);
	m_listProfile.SetItemText(nItem, 0, strText);

	*pResult = 0;
}


void CProfileManagerDialog::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	CDialogEx::OnGetMinMaxInfo(lpMMI);

	lpMMI->ptMinTrackSize.x = m_rcClientMin.Width();
	lpMMI->ptMinTrackSize.y = m_rcClientMin.Height();
}


void CProfileManagerDialog::OnClose()
{
	CRect rc;
	GetWindowRect(&rc);
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("MainWindow.Left"), rc.left);
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("MainWindow.Top"), rc.top);
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("MainWindow.Right"), rc.right);
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("MainWindow.Bottom"), rc.bottom);

	int nNameWidth = m_listProfile.GetColumnWidth(0);
	int nDateWidth = m_listProfile.GetColumnWidth(1);
	int nStatusWidth = m_listProfile.GetColumnWidth(2);

	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("HeaderColumn.Name.Width"), nNameWidth);
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("HeaderColumn.Date.Width"), nDateWidth);
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("HeaderColumn.Status.Width"), nStatusWidth);

	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("Autosave.Enable"), m_bEnableAutosave);
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("Autosave.MinInterval"), m_nAutosaveMinInterval);
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("Autosave.HideAutosaved"), m_bHideAutosaved);
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("Autosave.MaxAutosaves"), m_nMaxAutosaves);

	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("ProfileList.Sort"), m_nSort);

	// cleanup.
	EnableAutoSave(FALSE);

	CDialogEx::OnClose();
}


void CProfileManagerDialog::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TID_AUTO_SAVE) {
		m_pContext->GetCurrentBackupSet()->DoAutoSave(m_nMaxAutosaves);
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CProfileManagerDialog::EnableAutoSave(BOOL bEnable)
{
	if (bEnable) {
		m_pContext->GetCurrentBackupSet()->RegisterCallback(this);
		//if (!m_bEnableAutosave) {
			m_pContext->GetCurrentBackupSet()->DoAutoSave(m_nMaxAutosaves);
		//}
		SetTimer(TID_AUTO_SAVE, m_nAutosaveMinInterval, NULL);
	}
	else {
		KillTimer(TID_AUTO_SAVE);
		if (m_bEnableAutosave) {
			m_pContext->GetCurrentBackupSet()->DoAutoSave(m_nMaxAutosaves);
		}
		m_pContext->GetCurrentBackupSet()->UnregisterCallback(this);
	}
	m_bEnableAutosave = bEnable;
}

void CProfileManagerDialog::HideAutosaved(BOOL bHide)
{
	m_bHideAutosaved = bHide;

	InitList();
}

HRESULT CProfileManagerDialog::BackupSetAutoSaveSaved(CBackupSet *pBackupSet, CBackup *pBackup)
{
	if (m_bHideAutosaved) {
		return S_OK;
	}

	int nItem = InsertItem(-1, pBackup);

	UpdateLastSave(nItem);

	// Automatically scroll if nothing is selected.
	POSITION pos = m_listProfile.GetFirstSelectedItemPosition();
	if (!pos) {
		m_listProfile.EnsureVisible(nItem, FALSE);
	}

	return S_OK;
}

HRESULT CProfileManagerDialog::BackupSetAutoSaveParged(CBackupSet *pBackupSet, CBackup *pBackup)
{
	if (m_bHideAutosaved) {
		return S_OK;
	}

	int nItem = FindItem(pBackup);
	if (nItem == -1) {
		return S_OK;
	}

	m_listProfile.DeleteItem(nItem);

	return S_OK;
}

void CProfileManagerDialog::UpdateSortIcon(int nSort)
{
	CHeaderCtrl* pHeader = m_listProfile.GetHeaderCtrl();

	HDITEM hdi;
	ZeroMemory(&hdi, sizeof(hdi));

	hdi.mask = HDI_FORMAT;
	for (int n = 0; n < pHeader->GetItemCount(); n++) {
		pHeader->GetItem(n, &hdi);
		hdi.fmt &= ~(HDF_IMAGE | HDF_BITMAP_ON_RIGHT);
		pHeader->SetItem(n, &hdi);
	}

	int nSubItem = 0;
	switch (nSort) {
	case SORT_NAME:
	case SORT_NAME_DEC:
		nSubItem = 0;
		break;
	case SORT_DATE:
	case SORT_DATE_DEC:
		nSubItem = 1;
		break;
	case SORT_USER:
		return;
	}

	ZeroMemory(&hdi, sizeof(hdi));
	pHeader->GetItem(nSubItem, &hdi);

	hdi.mask = HDI_IMAGE | HDI_FORMAT;
	switch (nSort) {
	case SORT_NAME:
	case SORT_DATE:
		hdi.iImage = 0;
		break;
	case SORT_NAME_DEC:
	case SORT_DATE_DEC:
		hdi.iImage = 1;
		break;
	case SORT_USER:
		return;
	}

	hdi.fmt = HDF_STRING | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
	pHeader->SetItem(nSubItem, &hdi);
}

void CProfileManagerDialog::UpdateSort(int nSort)
{
	m_nSort = nSort;
	m_listProfile.SortItems(CompareListItem, m_nSort);
	UpdateSortIcon(m_nSort);
}

int CProfileManagerDialog::FindItem(CBackup *pBackup)
{
	int nItem = -1;
	for (int n = 0; n < m_listProfile.GetItemCount(); n++) {
		CComPtr <CBackup> p;
		p = (CBackup *)m_listProfile.GetItemData(n);
		if (p == pBackup) {
			nItem = n;
			break;
		}
	}
	return nItem;
}

void CProfileManagerDialog::OnHdnItemclickProfileList(NMHDR *pNMHDR, LRESULT *pResult)
{
	HRESULT hr = S_OK;

	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

	CComPtr <CBackupEnum> pEnum;
	hr = m_pContext->GetCurrentBackupSet()->EnumBackup(&pEnum);
	if (FAILED(hr)) {
		return;
	}

	int nSort = SORT_USER;
	switch (phdr->iItem) {
	case 0:	// name
		if (m_nSort == SORT_NAME) {
			nSort = SORT_NAME_DEC;
		}
		else if (m_nSort == SORT_NAME_DEC) {
			nSort = SORT_USER;
		}
		else {
			nSort = SORT_NAME;
		}
		break;
	case 1:	// date
		if (m_nSort == SORT_DATE) {
			nSort = SORT_DATE_DEC;
		}
		else if (m_nSort == SORT_DATE_DEC) {
			nSort = SORT_USER;
		}
		else {
			nSort = SORT_DATE;
		}
		break;
	case 2:
		return;
		break;	//　no effect
	}

	UpdateSort(nSort);

	*pResult = 0;
}

int CProfileManagerDialog::CompareListItem(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CComPtr <CBackup> pb1 = (CBackup *)lParam1;
	CComPtr <CBackup> pb2 = (CBackup *)lParam2;

	switch (lParamSort) {

	case SORT_NAME:
	case SORT_NAME_DEC:
		return pb1->GetName().CompareNoCase(pb2->GetName()) * ((lParamSort == SORT_NAME) ? 1 : -1);
		break;

	case SORT_DATE:
	case SORT_DATE_DEC:
	{
		FILETIME ft1, ft2;
SystemTimeToFileTime(&pb1->GetCreateTime(), &ft1);
SystemTimeToFileTime(&pb2->GetCreateTime(), &ft2);
return CompareFileTime(&ft1, &ft2)  * ((lParamSort == SORT_DATE) ? 1 : -1);
	}
	break;

	default:
	case SORT_USER:
		return (pb1->GetIndex() - pb2->GetIndex());
		break;
	}
}

HRESULT CProfileManagerDialog::SaveList()
{
	HRESULT hr = S_OK;
	for (int nItem = 0; nItem < m_listProfile.GetItemCount(); nItem++) {
		CComPtr <CBackup> pBackup;
		pBackup = (CBackup *)m_listProfile.GetItemData(nItem);
		hr = pBackup->SetIndex(nItem);
		if (FAILED(hr)) {
			return hr;
		}
	}
	hr = m_pContext->GetCurrentBackupSet()->SaveIndexList();
	if (FAILED(hr)) {
		return hr;
	}
	return S_OK;
}

void CProfileManagerDialog::OnLvnBeginDragProfileList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	m_lDragTopItem = pNMLV->iItem;	// 移動するアイテムの保存

	HWND hList = m_listProfile.GetSafeHwnd();	// 自作クラス
	POINT ptPos;
	m_hDragImage = ListView_CreateDragImage(hList, pNMLV->iItem, &ptPos);
	// ドラッグ開始
	POINT ptCursor;
	GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);

	long lX = ptCursor.x - ptPos.x;
	long lY = ptCursor.y - ptPos.y;

	//ImageList_BeginDrag(m_hDragImage, 0, lX, lY);
	//ImageList_DragEnter(hList, 0, 0);
	HCURSOR hCursor = LoadCursor(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_MOVE));
	SetCursor(hCursor);
	SetCapture();

	m_CaptureWnd = hList;
}

void CProfileManagerDialog::OnDrag(HWND hDlg, int x, int y)
{
	POINT Point = { x,y };
	ClientToScreen(&Point);
	RECT Rect;
	GetWindowRect(&Rect);
	//ImageList_DragMove(Point.x - Rect.left, Point.y - Rect.top);

	CRect rcList;
	m_listProfile.GetClientRect(&rcList);
	CRect rcHeader;
	m_listProfile.GetHeaderCtrl()->GetClientRect(&rcHeader);
	rcList.top += rcHeader.Height();
	m_listProfile.ClientToScreen(&rcList);
	ScreenToClient(&rcList);
	CRect rcListScroll = rcList;
	rcListScroll.DeflateRect(16, 16);
	CPoint ptCur(x, y);
	CSize sizeScroll(0, 0);
	if (ptCur.x < rcListScroll.left) {
		sizeScroll.cx = ptCur.x - rcListScroll.left;
	}
	else if (ptCur.x > rcListScroll.right) {
		sizeScroll.cx = ptCur.x - rcListScroll.right;
	}
	if (ptCur.y < rcListScroll.top) {
		sizeScroll.cy = ptCur.y - rcListScroll.top;
	}
	else if (ptCur.y > rcListScroll.bottom) {
		sizeScroll.cy = ptCur.y - rcListScroll.bottom;
	}
	if (sizeScroll.cx != 0 || sizeScroll.cy != 0) {
		m_listProfile.Scroll(sizeScroll);
	}
}

void CProfileManagerDialog::OnEndDrag(HWND hDlg, int x, int y)
{
	HRESULT hr = S_OK;

	//ImageList_DragLeave(hDlg);
	//ImageList_EndDrag();
	//ImageList_Destroy(m_hDragImage);
	m_hDragImage = NULL;

	SetCursor(NULL);
	//ShowCursor(TRUE);

	int nInsItem = m_listProfile.HitTest(CPoint(x, y));
	if (nInsItem == -1) {
		nInsItem = m_listProfile.GetItemCount();
	}
	else {
		CRect rcItem;
		m_listProfile.GetItemRect(nInsItem, &rcItem, LVIR_BOUNDS);
		if (y > rcItem.CenterPoint().y) {
			nInsItem++;
		}
		while (nInsItem < m_listProfile.GetItemCount()) {
			if (m_listProfile.GetItemState(nInsItem, LVIS_SELECTED) != LVIS_SELECTED) {
				break;
			}
			nInsItem++;
		}
	}

	CBackupList listMove;
	for (int nItem = m_listProfile.GetItemCount() - 1; nItem >= 0; nItem --) {
		if (m_listProfile.GetItemState(nItem, LVIS_SELECTED) == LVIS_SELECTED) {
			listMove.push_front((CBackup *)m_listProfile.GetItemData(nItem));
			m_listProfile.DeleteItem(nItem);
			if (nInsItem > nItem) {
				nInsItem--;
			}
		}
	}

	auto it = listMove.rbegin();
	while (it != listMove.rend()) {
		int nItem = InsertItem(nInsItem, *it);
		m_listProfile.SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
		m_listProfile.EnsureVisible(nItem, FALSE);
		it++;
	}

	//UpdateSort(SORT_USER);
	m_nSort = SORT_USER;
	UpdateSortIcon(m_nSort);
	hr = SaveList();
}

void CProfileManagerDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if (GetCapture() == this) {
		OnDrag(GetSafeHwnd(), point.x, point.y);
	}

	__super::OnMouseMove(nFlags, point);
}

void CProfileManagerDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (GetCapture() == this) {
		OnEndDrag(GetSafeHwnd(), point.x, point.y);
		// キャプチャ解除
		ReleaseCapture();
		m_CaptureWnd = NULL;
	}

	__super::OnLButtonUp(nFlags, point);
}


