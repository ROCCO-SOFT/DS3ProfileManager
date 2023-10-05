#pragma once

#include "Profile.h"

#define AC_STRETCH_MASK 0x0000000f
#define AC_MOVE_MASK    0x000000f0
#define AC_STRETCH_HV 0x00000001
#define AC_STRETCH_H  0x00000002
#define AC_STRETCH_V  0x00000003
#define AC_MOVE_HV    0x00000010
#define AC_MOVE_H     0x00000020
#define AC_MOVE_V     0x00000030

enum SORT
{
	SORT_NAME,
	SORT_NAME_DEC_UNUSE,
	SORT_DATE,
	SORT_DATE_DEC_UNUSE,
	SORT_USER,
};

class CProfileManagerDialog : public CDialogEx, public CBackupSetCallback
{
public:
	CProfileManagerDialog(CWnd* pParent = nullptr);
	int InsertItem(int nInsert, CBackup *pBackup);
	int UpdateList(int nItem);
	void UpdateLastSave(int nItem);
	void UpdateLastLoad(int nItem);
	static BOOL AlignControl(HWND hParent, int cx, int cy, int arID[], int nCount, int nFunc);
	BOOL InsertListColumn(CListCtrl *pList, int nCol, UINT nCaptionID,
		int nFormat = LVCFMT_LEFT, int nWidth = -1, int nSubItem = -1);
	void EnableAutoSave(BOOL bEnable);
	void HideAutosaved(BOOL bHide);
	virtual HRESULT BackupSetAutoSaveSaved(CBackupSet *pBackupSet, CBackup *pBackup);
	virtual HRESULT BackupSetAutoSaveParged(CBackupSet *pBackupSet, CBackup *pBackup);
	BOOL InitList();
	void UpdateSortIcon(int nSort, BOOL bDescending);
	HRESULT SaveList();
	void OnDrag(HWND hDlg, int x, int y);
	void OnEndDrag(HWND hDlg, int x, int y);
	static int CompareListItem_Name(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CompareListItem_Date(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CompareListItem_User(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	void UpdateSort(int nSort, BOOL bDescending);
	int FindItem(CBackup *pBackup);
	void SetTopMost(BOOL bTopMost);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROFILE_MANAGER };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	HICON m_hIcon;
	CComPtr <CContext> m_pContext;
	CRect m_rcClient;
	CRect m_rcClientMin;
	CComPtr <CBackup> m_pLastStatus;
	CListCtrl m_listProfile;
	BOOL m_bEnableAutosave;
	int m_nAutosaveMinInterval;
	BOOL m_bHideAutosaved;
	int m_nMaxAutosaves;
	int m_nSort;
	BOOL m_bSortDescending;
	CImageList m_listSortIcon;
	BOOL m_bSaveBeforeLoading;

	HIMAGELIST m_hDragImage;
	int m_lDragTopItem;
	HWND m_CaptureWnd;
	CMenu *m_pMenu;
	BOOL m_bTopMost;

	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSaveButton();
	afx_msg void OnBnClickedLoadButton();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedOverwriteButton();
	afx_msg void OnLvnItemchangedProfileList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedRemoveButton();
	afx_msg void OnLvnEndlabeleditProfileList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnHdnItemclickProfileList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnBeginDragProfileList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnUpdateAppAbout(CCmdUI *pCmdUI);
	afx_msg void OnAppAbout();
	afx_msg void OnUpdateAppReadme(CCmdUI *pCmdUI);
	afx_msg void OnAppReadme();
	afx_msg void OnUpdateSort(CCmdUI *pCmdUI);
	afx_msg void OnSort(UINT nID);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnUpdateBackupSave(CCmdUI *pCmdUI);
	afx_msg void OnBackupSave();
	afx_msg void OnUpdateBackupOverwrite(CCmdUI *pCmdUI);
	afx_msg void OnBackupOverwrite();
	afx_msg void OnUpdateBackupLoad(CCmdUI *pCmdUI);
	afx_msg void OnBackupLoad();
	afx_msg void OnUpdateBackupRemove(CCmdUI *pCmdUI);
	afx_msg void OnBackupRemove();
	afx_msg void OnUpdateBackupRename(CCmdUI *pCmdUI);
	afx_msg void OnBackupRename();
	afx_msg void OnUpdateViewMenuVisible(CCmdUI *pCmdUI);
	afx_msg void OnViewMenuVisible();
	afx_msg void OnUpdateViewHeaderVisible(CCmdUI *pCmdUI);
	afx_msg void OnViewHeaderVisible();
	afx_msg void OnUpdateViewAutosaveVisible(CCmdUI *pCmdUI);
	afx_msg void OnViewAutosaveVisible();
	afx_msg void OnUpdateSettings(CCmdUI *pCmdUI);
	afx_msg void OnSettings(UINT nID);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnUpdateViewTopmost(CCmdUI *pCmdUI);
	afx_msg void OnViewTopmost();
};
