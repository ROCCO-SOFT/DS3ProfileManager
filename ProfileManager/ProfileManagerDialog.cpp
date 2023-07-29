#include "pch.h"
#include "framework.h"
#include "ProfileManager.h"
#include "ProfileManagerDialog.h"
#include "afxdialogex.h"
#include "Profile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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
END_MESSAGE_MAP()

CProfileManagerDialog::CProfileManagerDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PROFILE_MANAGER, pParent)
	, m_nLastStatus(-1)
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

BOOL CProfileManagerDialog::OnInitDialog()
{
	HRESULT hr = S_OK;

	CDialogEx::OnInitDialog();

	// 初期のサイズが最小サイズ。
	GetClientRect(&m_rcClientMin);

	CRect rc;
	GetWindowRect(&rc);
	rc.left = AfxGetApp()->GetProfileInt(_T("Settings"), _T("MainWindow.Left"), rc.left);
	rc.top = AfxGetApp()->GetProfileInt(_T("Settings"), _T("MainWindow.Top"), rc.top);
	rc.right = AfxGetApp()->GetProfileInt(_T("Settings"), _T("MainWindow.Right"), rc.right);
	rc.bottom = AfxGetApp()->GetProfileInt(_T("Settings"), _T("MainWindow.Bottom"), rc.bottom);
	SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);

	GetClientRect(&m_rcClient);

	// "バージョン情報..." メニューをシステム メニューに追加します。
	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	hr = CreateContext(&m_pContext);
	if (FAILED(hr)) {
		return FALSE;
	}

	int nNameWidth = 120;
	int nDateWidth = 140;
	int nStatusWidth = 80;

	nNameWidth = AfxGetApp()->GetProfileInt(_T("Settings"), _T("HeaderColumn.Name.Width"), nNameWidth);
	nDateWidth = AfxGetApp()->GetProfileInt(_T("Settings"), _T("HeaderColumn.Date.Width"), nDateWidth);
	nStatusWidth = AfxGetApp()->GetProfileInt(_T("Settings"), _T("HeaderColumn.Status.Width"), nStatusWidth);

	m_listProfile.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	InsertListColumn(&m_listProfile, 0, IDS_LIST_HEADER_NAME, 0, nNameWidth);
	InsertListColumn(&m_listProfile, 1, IDS_LIST_HEADER_DATE, 0, nDateWidth);
	InsertListColumn(&m_listProfile, 2, IDS_LIST_HEADER_STATUS, 0, nStatusWidth);

	CComPtr <CBackupEnum> pEnum;
	hr = m_pContext->GetCurrentBackupSet()->EnumBackup(&pEnum);
	if (FAILED(hr)) {
		return FALSE;
	}

	CComPtr <CBackup> pBackup;
	while (pEnum->Next(&pBackup) == S_OK) {
		AddList(pBackup);
		pBackup.Release();
	}

	UpdateData(FALSE);

	return TRUE;
}

int CProfileManagerDialog::AddList(CBackup *pBackup)
{
	int nItem = m_listProfile.InsertItem(m_listProfile.GetItemCount(), pBackup->GetName());
	SYSTEMTIME stCreate = pBackup->GetCreateTime();
	CTime tmCreate(stCreate);
	CString strTime;
	strTime = tmCreate.Format(_T("%Y/%m/%d %H:%M:%S"));
	m_listProfile.SetItemText(nItem, 1, strTime);
	m_listProfile.SetItemData(nItem, (DWORD_PTR)pBackup);

	return nItem;
}

int CProfileManagerDialog::UpdateList(int nItem)
{
	CBackup *pBackup = (CBackup *)m_listProfile.GetItemData(nItem);

	SYSTEMTIME stCreate = pBackup->GetCreateTime();
	CTime tmCreate(stCreate);
	CString strTime;
	strTime = tmCreate.Format(_T("%Y/%m/%d %H:%M:%S"));
	m_listProfile.SetItemText(nItem, 1, strTime);

	return nItem;
}

void CProfileManagerDialog::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CProfileManagerDialog::UpdateLastSave(int nItem)
{
	if (m_nLastStatus != -1) {
		m_listProfile.SetItemText(m_nLastStatus, 2, _T(""));
	}
	m_nLastStatus = nItem;
	m_listProfile.SetItemText(m_nLastStatus, 2, _T("Saved"));
}

void CProfileManagerDialog::UpdateLastLoad(int nItem)
{
	if (m_nLastStatus != -1) {
		m_listProfile.SetItemText(m_nLastStatus, 2, _T(""));
	}
	m_nLastStatus = nItem;
	m_listProfile.SetItemText(m_nLastStatus, 2, _T("Loaded"));
}

void CProfileManagerDialog::OnBnClickedSaveButton()
{
	HRESULT hr = S_OK;

	UpdateData();

	CBackup *pBackup;
	hr = m_pContext->GetCurrentBackupSet()->SaveCurrent(&pBackup, _T("new Backup"));
	if (FAILED(hr)) {
		return;
	}

	int nItem = AddList(pBackup);
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

	UINT nPrompt = AfxMessageBox(IDS_OVERWRITE_CONFIRM, MB_OKCANCEL | MB_ICONQUESTION);
	if (nPrompt == IDCANCEL) {
		return;
	}

	//OnBnClickedSaveButton();

	int nItem = m_listProfile.GetNextSelectedItem(pos);
	CBackup *pBackup = (CBackup *)m_listProfile.GetItemData(nItem);

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

	UINT nPrompt = AfxMessageBox(IDS_LOAD_CONFIRM, MB_OKCANCEL | MB_ICONQUESTION);
	if (nPrompt == IDCANCEL) {
		return;
	}

	//OnBnClickedSaveButton();

	int nItem = m_listProfile.GetNextSelectedItem(pos);
	CBackup *pBackup = (CBackup *)m_listProfile.GetItemData(nItem);

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

	UINT nPrompt = AfxMessageBox(IDS_DELETE_CONFIRM, MB_OKCANCEL | MB_ICONQUESTION);
	if (nPrompt == IDCANCEL) {
		return;
	}

	//OnBnClickedSaveButton();

	int nItem = m_listProfile.GetNextSelectedItem(pos);
	CBackup *pBackup = (CBackup *)m_listProfile.GetItemData(nItem);

	hr = pBackup->Delete();
	if (FAILED(hr)) {
		return;
	}

	m_listProfile.DeleteItem(nItem);
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

	CBackup *pBackup = (CBackup *)m_listProfile.GetItemData(nItem);
	hr = pBackup->SetName(pDispInfo->item.pszText);
	m_listProfile.SetItemText(nItem, 0, pDispInfo->item.pszText);

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

	CDialogEx::OnClose();
}
