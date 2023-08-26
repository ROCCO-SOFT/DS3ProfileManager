#include "pch.h"
#include "Profile.h"
#include "UnknownImpl.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void Replace(LPTSTR psz, TCHAR c1, TCHAR c2)
{
	LPTSTR p = psz;
	while (*p) {
		LPTSTR t = _tcschr(p + 1, c1);
		if (!t) {
			return;
		}
		p = t + 1;
		*t = c2;
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class CContextImpl : public CContext, public CUnknownImpl
{
public:

	DECLARE_UNKNOWN_IMPL();

	virtual CBackupSet *GetCurrentBackupSet();
	virtual CString GetProfilePath();
	virtual CString GetBackupPath();
	static HRESULT Open(CContext **ppContext);

protected:

	CString m_strProfilePath;
	CString m_strBackupPath;
	std::list<CComPtr<CBackupSet>> m_listBackupSet;

	CContextImpl();
	virtual ~CContextImpl();
	HRESULT OpenBackupSet();
	static HRESULT GetSpecialFolderPath(int id, CString &strRet);

};

class CBackupSetImpl : public CBackupSet, public CUnknownImpl
{
public:

	DECLARE_UNKNOWN_IMPL();

	virtual HRESULT EnumBackup(CBackupEnum **ppEnum);
	virtual HRESULT SaveCurrent(CBackup **ppBackup, PCTSTR pszName);
	virtual HRESULT AddBackup(CBackup *pBackup);
	virtual HRESULT RemoveBackup(CBackup *pBackup);
	virtual CString GetProfilePath();
	virtual CString GetProfileId();
	virtual HRESULT DoAutoSave(int nMaxAutosaves);
	virtual HRESULT RegisterCallback(CBackupSetCallback *pCallback);
	virtual HRESULT UnregisterCallback(CBackupSetCallback *pCallback);
	virtual HRESULT LoadIndexList();
	virtual HRESULT SaveIndexList();
	static HRESULT Open(CBackupSet **ppList, PCTSTR pszProfileId, PCTSTR pszProfilePath, CContext *pContext);

protected:

	CBackupList m_listBackup;
	CContext *m_pContext;
	CString m_strProfilePath;
	CString m_strProfileId;					// �J�����gID�B
	CTime m_timeLatestSave;
	std::map <CBackupSetCallback *, CBackupSetCallback *> m_listCallback;

	CBackupSetImpl();
	virtual ~CBackupSetImpl();
	HRESULT UpdateBackupIndex(CBackupList::iterator it);
	static bool CompareBackupDate(CBackup *p1, CBackup *p2);
	static bool CompareBackupIndex(CBackup *p1, CBackup *p2);

};

class CBackupImpl : public CBackup, public CUnknownImpl
{
public:

	DECLARE_UNKNOWN_IMPL();

	virtual SYSTEMTIME GetCreateTime();
	virtual CString GetName();
	virtual HRESULT SetName(PCTSTR pszName);
	virtual CString GetPath();
	virtual HRESULT Load();
	virtual HRESULT Save();
	virtual HRESULT Delete();
	virtual HRESULT IsAutosaved();
	virtual int GetIndex();
	virtual HRESULT SetIndex(int nIndex);

	static HRESULT Open(CBackup **ppBackup, PCTSTR pszFileName, CContext *pContext, CBackupSet *pSet);
	static HRESULT FormatFname(PTSTR szFname, int cbFname, PCTSTR pszId, SYSTEMTIME sTime, PCTSTR szName);
	static HRESULT ScanFname(PCTSTR szFname, CString &strId, SYSTEMTIME &sTime, CString &strName);

protected:

	CTime m_dateCreate;
	CTime m_dateSL2;
	CString m_strName;
	CString m_strPath;
	BOOL m_bReadOnly;
	CString m_strId;
	CContext *m_pContext;
	CBackupSet *m_pSet;
	int m_nIndex;

	CBackupImpl();
	virtual ~CBackupImpl();

};

class CBackupEnumImpl : public CBackupEnum, public CUnknownImpl
{
public:

	DECLARE_UNKNOWN_IMPL();

	virtual HRESULT Next(CBackup **ppBackup);
	virtual HRESULT Reset();
	static HRESULT Open(CBackupEnum **ppEnum, CBackupList *pList, CBackupSetImpl *pSet);

protected:

	CBackupList::iterator m_it;
	CBackupList m_list;
	CComPtr <CBackupSetImpl> m_pSet;

	CBackupEnumImpl();
	virtual ~CBackupEnumImpl();

};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CContextImpl::CContextImpl()
{

}

CContextImpl::~CContextImpl()
{

}

HRESULT CContextImpl::OpenBackupSet()
{
	HRESULT hr = S_OK;

	CFileFind find;
	BOOL bFound = find.FindFile(GetProfilePath() + _T("*.*"));
	while (bFound) {
		bFound = find.FindNextFile();
		if (find.IsDots()) {
			continue;
		}
		else if (!find.IsDirectory()) {
			continue;
		}

		CString strId = find.GetFileTitle();
		CString strPath = find.GetFilePath();
		strPath.TrimRight(_T("\\"));
		strPath += _T("\\");
		if (!PathFileExists(strPath + SL2_FILE)) {
			continue;	// SL2�t�@�C����������Ȃ��B
		}

		CComPtr <CBackupSet> pSet;
		hr = CBackupSetImpl::Open(&pSet, strId, strPath, this);
		if (FAILED(hr)) {
			continue;
		}

		m_listBackupSet.push_back(pSet);
	}

	return S_OK;
}

CBackupSet *CContextImpl::GetCurrentBackupSet()
{
	return m_listBackupSet.front();
}

CString CContextImpl::GetProfilePath()
{
	return m_strProfilePath;
}

CString CContextImpl::GetBackupPath()
{
	return m_strBackupPath;
}

HRESULT CContextImpl::GetSpecialFolderPath(int id, CString &strRet)
{
	HRESULT hr = S_OK;
	LPITEMIDLIST pidl = NULL;
	if ((SHGetSpecialFolderLocation(NULL, id, &pidl) != NOERROR)
		|| !pidl) {
		return E_FAIL;
	}
	CString strPath;
	do {
		if (!SHGetPathFromIDList(pidl, strPath.GetBuffer(MAX_PATH))) {
			hr = E_FAIL;
			break;
		}
		strPath.ReleaseBuffer();
	} while (0);
	CoTaskMemFree(pidl);
	pidl = NULL;
	if (FAILED(hr)) {
		return hr;
	}
	strPath.TrimRight(_T("\\"));
	strPath += _T("\\");
	if (!PathFileExists(strPath)) {
		return E_FAIL;
	}
	strRet = strPath;
	return S_OK;
}

HRESULT CContextImpl::Open(CContext **ppContext)
{
	HRESULT hr = S_OK;

	CString strProfile;
	hr = GetSpecialFolderPath(PROFILE_FOLDER_ID, strProfile);
	if (FAILED(hr)) {
		return hr;	// AppData�̏ꏊ�����Ȃ��B
	}
	strProfile += PROFILE_FOLDER _T("\\");
	if (!PathFileExists(strProfile)) {
		return E_FAIL;	// �v���t�@�C����������Ȃ��B
	}

	CString strBackup;
	hr = GetSpecialFolderPath(CSIDL_PERSONAL, strBackup);
	if (FAILED(hr)) {
		return hr;	// Personal�̏ꏊ�����Ȃ��B
	}

	TCHAR path[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	if (GetModuleFileName(NULL, path, numof(path)) == 0) {
		return E_FAIL;	// ���W���[���̃t�@�C���������Ȃ��B
	}
	_tsplitpath_s(path, drive, dir, fname, ext);
	strBackup += fname;
	strBackup +=_T("\\");

	if (!PathFileExists(strBackup)) {
		if (!CreateDirectory(strBackup, NULL)) {
			return E_FAIL;	// �o�b�N�A�b�v�t�H���_�����Ȃ��B
		}
	}

	CComPtr <CContextImpl> pThis = new CContextImpl;
	if (!pThis) {
		return E_FAIL;	// �������s���B
	}

	pThis->m_strProfilePath = strProfile;
	pThis->m_strBackupPath = strBackup;

	hr = pThis->OpenBackupSet();
	if (FAILED(hr)) {
		return hr;
	}

	if (pThis->m_listBackupSet.empty()) {
		return E_FAIL;	// �Z�[�u�f�[�^���Ȃ��\��������B
	}

	*ppContext = pThis;
	(*ppContext)->AddRef();

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CBackupSetImpl::CBackupSetImpl()
{

}

CBackupSetImpl::~CBackupSetImpl()
{

}

HRESULT CBackupSetImpl::EnumBackup(CBackupEnum **ppEnum)
{
	HRESULT hr = S_OK;

	CComPtr <CBackupEnum> pEnum;
	hr = CBackupEnumImpl::Open(&pEnum, &m_listBackup, this);
	if (FAILED(hr)) {
		return hr;
	}

	*ppEnum = pEnum;
	(*ppEnum)->AddRef();

	return S_OK;
}

CString CBackupSetImpl::GetProfilePath()
{
	return m_strProfilePath;
}

CString CBackupSetImpl::GetProfileId()
{
	return m_strProfileId;
}

HRESULT CBackupSetImpl::DoAutoSave(int nMaxAutosaves)
{
	HRESULT hr = S_OK;

#if 1
	CString strSL2 = m_strProfilePath + SL2_FILE;
	if (!PathFileExists(strSL2)) {
		return E_FAIL;
	}

	WIN32_FILE_ATTRIBUTE_DATA attr = { NULL };
	GetFileAttributesEx(strSL2, GetFileExInfoStandard, &attr);
	FILETIME ftLocal;
	FileTimeToLocalFileTime(&attr.ftLastWriteTime, &ftLocal);
	SYSTEMTIME sTimeSL2;
	FileTimeToSystemTime(&ftLocal, &sTimeSL2);
#endif

	CTime time(sTimeSL2);
	if (time > m_timeLatestSave) {
		CComPtr <CBackup> pBackup;
		hr = SaveCurrent(&pBackup, AUTOSAVE_NAME);
		if (FAILED(hr)) {
			return hr;
		}

#if 1
		CBackupList listAutosaved;
		auto it = m_listBackup.begin();
		while (it != m_listBackup.end()) {
			if ((*it)->IsAutosaved()) {
				listAutosaved.push_back(*it);
			}
			it++;
		}
		if (listAutosaved.size() > nMaxAutosaves) {
			listAutosaved.sort(CompareBackupDate);
			auto it = listAutosaved.begin();
			while ((listAutosaved.size() > nMaxAutosaves)
				&& (it != listAutosaved.end())) {
				CComPtr <CBackup> pErase;
				pErase = *(it++);
				auto itFound = std::find(m_listBackup.begin(), m_listBackup.end(), pErase);
				if (itFound != m_listBackup.end()) {
					for (auto it = m_listCallback.begin(); it != m_listCallback.end(); it++) {
						CBackupSetCallback *pCallback = it->second;
						pCallback->BackupSetAutoSaveParged(this, pErase);
					}
					m_listBackup.erase(itFound);
				}
				hr = pErase->Delete();
				if (FAILED(hr)) {
					return hr;
				}
				it = listAutosaved.erase(it);
			}
		}
#endif

#if 1
		{
			WIN32_FILE_ATTRIBUTE_DATA attr = { NULL };
			GetFileAttributesEx(strSL2, GetFileExInfoStandard, &attr);
			FILETIME ftLocal;
			FileTimeToLocalFileTime(&attr.ftLastWriteTime, &ftLocal);
			SYSTEMTIME sTimeSL2;
			FileTimeToSystemTime(&ftLocal, &sTimeSL2);

			CTime time2(sTimeSL2);
			if (time2 > time) {
				// It seems that it was updated while backing up.
				pBackup->Delete();
				return E_FAIL;
			}
		}
#endif

		for (auto it = m_listCallback.begin(); it != m_listCallback.end(); it++) {
			CBackupSetCallback *pCallback = it->second;
			pCallback->BackupSetAutoSaveSaved(this, pBackup);
		}
	}

	return S_OK;
}

HRESULT CBackupSetImpl::RegisterCallback(CBackupSetCallback *pCallback)
{
	auto it = m_listCallback.find(pCallback);
	if (it != m_listCallback.end()) {
		return E_FAIL;
	}

	m_listCallback[pCallback] = pCallback;

	return S_OK;
}

HRESULT CBackupSetImpl::UnregisterCallback(CBackupSetCallback *pCallback)
{
	auto it = m_listCallback.find(pCallback);
	if (it == m_listCallback.end()) {
		return E_FAIL;
	}

	it = m_listCallback.erase(it);

	return S_OK;
}

HRESULT CBackupSetImpl::UpdateBackupIndex(CBackupList::iterator it)
{
	int nIndex = 0;
	while (it != m_listBackup.end()) {
		(*it)->SetIndex(nIndex++);
		it++;
	}
	return S_OK;
}

HRESULT CBackupSetImpl::LoadIndexList()
{
	CString strFileName = m_pContext->GetBackupPath() + m_strProfileId + ORDER_INDEX_FILE;
	if (!PathFileExists(strFileName)) {
		return S_OK;
	}
	CStdioFile file;
	if (!file.Open(strFileName, CFile::modeRead | CFile::typeText | CFile::typeUnicode)) {
		return E_FAIL;
	}

	CBackupList list = m_listBackup;
	CBackupList listNew;
	CString strPath;
	while (file.ReadString(strPath)) {
		auto it = list.begin();
		while (it != list.end()) {
			if (_tcsicmp(strPath, (*it)->GetPath()) == 0) {
				listNew.push_back(*it);
				it = list.erase(it);
				break;
			}
			it++;
		}
	}
	if (!list.empty()) {
		listNew.insert(listNew.end(), list.begin(), list.end());
	}
	if (!listNew.empty()) {
		m_listBackup = listNew;
	}

	UpdateBackupIndex(m_listBackup.begin());

	return S_OK;
}

HRESULT CBackupSetImpl::SaveIndexList()
{
	CString strFileName = m_pContext->GetBackupPath() + m_strProfileId + ORDER_INDEX_FILE;
	CStdioFile file;
	if (!file.Open(strFileName, CFile::modeReadWrite | CFile::typeText | CFile::modeCreate | CFile::typeUnicode)) {
		return E_FAIL;
	}

	CBackupList list = m_listBackup;
	list.sort(CompareBackupIndex);
	auto it = list.begin();
	while (it != list.end()) {
		file.WriteString((*it)->GetPath() + _T("\n"));
		it++;
	}

	return S_OK;
}


HRESULT CBackupSetImpl::SaveCurrent(CBackup **ppBackup, PCTSTR pszName)
{
	HRESULT hr = S_OK;

	SHFILEOPSTRUCT fos;
	ZeroMemory(&fos, sizeof(fos));

	// From
	CString strFrom = m_strProfilePath + _T("*.*");

	// To
	SYSTEMTIME sTime;
	CTime::GetCurrentTime().GetAsSystemTime(sTime);
	TCHAR fname[_MAX_FNAME];
	hr = CBackupImpl::FormatFname(fname, _MAX_FNAME, m_strProfileId, sTime, pszName);
	if (FAILED(hr)) {
		return hr;
	}
	CString strTo = m_pContext->GetBackupPath();
	strTo += fname;

	strFrom += _T("\t");
	strTo += _T("\t");

	fos.hwnd = NULL;
	fos.wFunc = FO_COPY;
	fos.fFlags = FOF_NOCONFIRMMKDIR;
	fos.pFrom = strFrom;
	fos.pTo = strTo;

	// ����������Ƃ��Ȃ��Ɠ��삵�Ȃ��B
	Replace((LPTSTR)fos.pFrom, _T('\t'), _T('\0'));
	Replace((LPTSTR)fos.pTo, _T('\t'), _T('\0'));

	SHFileOperation(&fos);

	CComPtr <CBackup> pBackup;
	hr = CBackupImpl::Open(&pBackup, strTo, m_pContext, this);
	if (FAILED(hr)) {
		return hr;
	}

	hr = AddBackup(pBackup);
	if (FAILED(hr)) {
		return hr;
	}

	if (m_timeLatestSave < pBackup->GetCreateTime()) {
		m_timeLatestSave = pBackup->GetCreateTime();
	}

	*ppBackup = pBackup;
	(*ppBackup)->AddRef();

	return S_OK;
}

HRESULT CBackupSetImpl::AddBackup(CBackup *pBackup)
{
	pBackup->SetIndex((int)m_listBackup.size());
	m_listBackup.push_back(pBackup);
	if (m_timeLatestSave < pBackup->GetCreateTime()) {
		m_timeLatestSave = pBackup->GetCreateTime();
	}
	return S_OK;
}

HRESULT CBackupSetImpl::RemoveBackup(CBackup *pBackup)
{
	auto it = m_listBackup.begin();
	while (it != m_listBackup.end()) {
		if (*it == pBackup) {
			it = m_listBackup.erase(it);
			break;
		}
		else {
			it++;
		}
	}

	UpdateBackupIndex(it);

	return S_OK;
}

bool CBackupSetImpl::CompareBackupDate(CBackup *p1, CBackup *p2)
{
	FILETIME ft1, ft2;
	SystemTimeToFileTime(&p1->GetCreateTime(), &ft1);
	SystemTimeToFileTime(&p2->GetCreateTime(), &ft2);
	return CompareFileTime(&ft1, &ft2) < 0;
}

bool CBackupSetImpl::CompareBackupIndex(CBackup *p1, CBackup *p2)
{
	return p1->GetIndex() < p2->GetIndex();
}

HRESULT CBackupSetImpl::Open(CBackupSet **ppList, PCTSTR pszProfileId, PCTSTR pszProfilePath, CContext *pContext)
{
	HRESULT hr = S_OK;

	CComPtr <CBackupSetImpl> pThis = new CBackupSetImpl;
	if (!pThis) {
		return E_FAIL;
	}

	pThis->m_strProfileId = pszProfileId;
	pThis->m_strProfilePath = pszProfilePath;
	pThis->m_pContext = pContext;

	CFileFind find;
	BOOL bFound = find.FindFile(pContext->GetBackupPath() + _T("*.*"));
	while (bFound) {
		bFound = find.FindNextFile();
		if (find.IsDots()) {
			continue;
		}
		else if (!find.IsDirectory()) {
			continue;
		}

		CString szPath = find.GetFilePath();
		szPath.TrimRight(_T("\\"));
		szPath += _T("\\");

		CComPtr <CBackup> pBackup;
		hr = CBackupImpl::Open(&pBackup, szPath, pContext, pThis);
		if (FAILED(hr)) {
			continue;
		}

		if (pThis->m_timeLatestSave < pBackup->GetCreateTime()) {
			pThis->m_timeLatestSave = pBackup->GetCreateTime();
		}

		pThis->m_listBackup.push_back(pBackup);
	}

	pThis->LoadIndexList();

	*ppList = pThis;
	(*ppList)->AddRef();

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CBackupImpl::CBackupImpl()
	: m_bReadOnly(FALSE)
	, m_nIndex(-1)
{

}

CBackupImpl::~CBackupImpl()
{

}

HRESULT CBackupImpl::FormatFname(PTSTR szFname, int cbFname, PCTSTR pszId, SYSTEMTIME sTime, PCTSTR szName)
{
	try {
		FILETIME fTime;
		SystemTimeToFileTime(&sTime, &fTime);
		UINT64 time = ((UINT64)fTime.dwHighDateTime << 32) | fTime.dwLowDateTime;
		_stprintf_s(szFname, cbFname, PROFILE_FNAME_FORMAT, pszId, time);
		_tcscat_s(szFname, cbFname, szName);
	}
	catch (...) {
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CBackupImpl::ScanFname(PCTSTR szFname, CString &strId, SYSTEMTIME &sTime, CString &strName)
{
	CString strFname = szFname;

	int nSep1 = strFname.Find(_T("_"));	// first separator
	if (nSep1 == -1) {
		return E_FAIL;
	}
	strId = strFname.Mid(0, nSep1);

	int nSep2 = strFname.Find(_T("_"), nSep1 + 1);
	if (nSep2 == -1) {
		return E_FAIL;
	}
	CString strTime = strFname.Mid(nSep1 + 1, nSep2 - (nSep1 + 1));
	UINT64 time = -1;
	_stscanf_s(strTime, _T("%016I64x"), &time);
	if (time == -1) {
		return E_FAIL;
	}
	FILETIME fTime;
	fTime.dwHighDateTime = (UINT32)(time >> 32);
	fTime.dwLowDateTime = (UINT32)time;
	FileTimeToSystemTime(&fTime, &sTime);

	strName = strFname.Mid(nSep2 + 1);

	return S_OK;
}

SYSTEMTIME CBackupImpl::GetCreateTime()
{
	SYSTEMTIME st;
	m_dateSL2.GetAsSystemTime(st);
	return st;
}

CString CBackupImpl::GetName()
{
	return m_strName;
}

HRESULT CBackupImpl::SetName(PCTSTR pszName)
{
	HRESULT hr = S_OK;

	SHFILEOPSTRUCT fos;
	ZeroMemory(&fos, sizeof(fos));

	// From
	CString strFrom = m_strPath;

	// To
	TCHAR fname[_MAX_FNAME];
	SYSTEMTIME sTime;
	m_dateCreate.GetAsSystemTime(sTime);
	hr = CBackupImpl::FormatFname(fname, _MAX_FNAME, m_pSet->GetProfileId(), sTime, pszName);
	if (FAILED(hr)) {
		return hr;
	}

	CString strTo = m_pContext->GetBackupPath() + fname + _T("\\");

	strFrom += _T("\t");
	strTo += _T("\t");

	fos.hwnd = NULL;
	fos.wFunc = FO_RENAME;
	fos.fFlags = FOF_NOCONFIRMMKDIR | FOF_SILENT | FOF_NOCONFIRMATION;
	fos.pFrom = strFrom;
	fos.pTo = strTo;

	// ����������Ƃ��Ȃ��Ɠ��삵�Ȃ��B
	Replace((LPTSTR)fos.pFrom, _T('\t'), _T('\0'));
	Replace((LPTSTR)fos.pTo, _T('\t'), _T('\0'));

	SHFileOperation(&fos);

	m_strName = pszName;
	m_strPath = m_pContext->GetBackupPath() + fname + _T("\\");

	return S_OK;
}

CString CBackupImpl::GetPath()
{
	return m_strPath;
}

HRESULT CBackupImpl::Load()
{
	HRESULT hr = S_OK;

	SHFILEOPSTRUCT fos;
	ZeroMemory(&fos, sizeof(fos));

	// From
	CString strFrom = m_strPath + _T("*.*");

	// To
	CString strTo = m_pSet->GetProfilePath();

	strFrom += _T("\t");
	strTo += _T("\t");

	fos.hwnd = NULL;
	fos.wFunc = FO_COPY;
	fos.fFlags = FOF_NOCONFIRMMKDIR | FOF_SILENT | FOF_NOCONFIRMATION;
	fos.pFrom = strFrom;
	fos.pTo = strTo;

	// ����������Ƃ��Ȃ��Ɠ��삵�Ȃ��B
	Replace((LPTSTR)fos.pFrom, _T('\t'), _T('\0'));
	Replace((LPTSTR)fos.pTo, _T('\t'), _T('\0'));

	SHFileOperation(&fos);

	return S_OK;
}

HRESULT CBackupImpl::Save()
{
	HRESULT hr = S_OK;

	SHFILEOPSTRUCT fos;
	ZeroMemory(&fos, sizeof(fos));

	// From
	CString strFrom = m_pSet->GetProfilePath() + _T("*.*");

	// To
	CString strTo = m_strPath;

	strFrom += _T("\t");
	strTo += _T("\t");

	fos.hwnd = NULL;
	fos.wFunc = FO_COPY;
	fos.fFlags = FOF_NOCONFIRMMKDIR | FOF_SILENT | FOF_NOCONFIRMATION;
	fos.pFrom = strFrom;
	fos.pTo = strTo;

	// ����������Ƃ��Ȃ��Ɠ��삵�Ȃ��B
	Replace((LPTSTR)fos.pFrom, _T('\t'), _T('\0'));
	Replace((LPTSTR)fos.pTo, _T('\t'), _T('\0'));

	SHFileOperation(&fos);

	CString strSL2 = m_strPath + SL2_FILE;
	if (!PathFileExists(strSL2)) {
		return E_FAIL;
	}

	WIN32_FILE_ATTRIBUTE_DATA attr = { NULL };
	GetFileAttributesEx(strSL2, GetFileExInfoStandard, &attr);
	FILETIME ftLocal;
	FileTimeToLocalFileTime(&attr.ftLastWriteTime, &ftLocal);
	SYSTEMTIME sTimeSL2;
	FileTimeToSystemTime(&ftLocal, &sTimeSL2);
	m_dateSL2 = sTimeSL2;

	return S_OK;
}

HRESULT CBackupImpl::Delete()
{
	HRESULT hr = S_OK;

	SHFILEOPSTRUCT fos;
	ZeroMemory(&fos, sizeof(fos));

	// From
	CString strFrom = m_strPath;
	strFrom += _T("\t");

	fos.hwnd = NULL;
	fos.wFunc = FO_DELETE;
	fos.fFlags = FOF_NOCONFIRMMKDIR | FOF_SILENT | FOF_NOCONFIRMATION;
	fos.pFrom = strFrom;

	// ����������Ƃ��Ȃ��Ɠ��삵�Ȃ��B
	Replace((LPTSTR)fos.pFrom, _T('\t'), _T('\0'));

	SHFileOperation(&fos);

	hr = m_pSet->RemoveBackup(this);
	if (FAILED(hr)) {
		return hr;
	}

	return S_OK;
}

HRESULT CBackupImpl::IsAutosaved()
{
	return _tcscmp(m_strName, AUTOSAVE_NAME) == 0;
}

int CBackupImpl::GetIndex()
{
	return m_nIndex;
}

HRESULT CBackupImpl::SetIndex(int nIndex)
{
	m_nIndex = nIndex;
	return S_OK;
}

HRESULT CBackupImpl::Open(CBackup **ppBackup, PCTSTR pszPath, CContext *pContext, CBackupSet *pSet)
{
	HRESULT hr = S_OK;

	if (!PathFileExists(pszPath)) {
		return E_FAIL;
	}

	CString strPath = pszPath;
	strPath.TrimRight(_T("\\"));	// �킴�ƉE��\����菜���Ă���B

	TCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_tsplitpath_s(strPath, drive, dir, fname, ext);

	strPath += _T("\\");	// \��߂��B

	CString strId;
	SYSTEMTIME sTime;
	CString strName;
	hr = ScanFname(fname, strId, sTime, strName);
	if (FAILED(hr)) {
		return hr;
	}

	if (strId != pSet->GetProfileId()) {
		return E_FAIL;
	}

#if 1
	CString strSL2 = strPath + SL2_FILE;
	if (!PathFileExists(strSL2)) {
		return E_FAIL;
	}

	WIN32_FILE_ATTRIBUTE_DATA attr = { NULL };
	GetFileAttributesEx(strSL2, GetFileExInfoStandard, &attr);
	FILETIME ftLocal;
	FileTimeToLocalFileTime(&attr.ftLastWriteTime, &ftLocal);
	SYSTEMTIME sTimeSL2;
	FileTimeToSystemTime(&ftLocal, &sTimeSL2);
#endif

	CComPtr <CBackupImpl> pThis = new CBackupImpl;
	if (!pThis) {
		return E_FAIL;
	}

	pThis->m_dateCreate = sTime;
	pThis->m_dateSL2 = sTimeSL2;
	pThis->m_strPath = strPath;
	pThis->m_strName = strName;
	pThis->m_strId = strId;
	pThis->m_bReadOnly = FALSE;
	pThis->m_pContext = pContext;
	pThis->m_pSet = pSet;

	*ppBackup = pThis;
	(*ppBackup)->AddRef();

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CBackupEnumImpl::CBackupEnumImpl()
{

}

CBackupEnumImpl::~CBackupEnumImpl()
{

}

HRESULT CBackupEnumImpl::Next(CBackup **ppBackup)
{
	if (m_it == m_list.end()) {
		return S_FALSE;
	}
	*ppBackup = *(m_it++);
	(*ppBackup)->AddRef();
	return S_OK;
}

HRESULT CBackupEnumImpl::Reset()
{
	m_it = m_list.begin();
	return S_OK;
}

HRESULT CBackupEnumImpl::Open(CBackupEnum **ppEnum, CBackupList *pList, CBackupSetImpl *pSet)
{
	CComPtr <CBackupEnumImpl> pThis;
	pThis = new CBackupEnumImpl;
	if (!pThis) {
		return E_FAIL;
	}
	pThis->m_list = *pList;
	pThis->m_it = pThis->m_list.begin();
	pThis->m_pSet = pSet;
	*ppEnum = pThis;
	(*ppEnum)->AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

HRESULT CreateContext(CContext **ppContext)
{
	HRESULT hr = S_OK;
	CComPtr <CContext> pContext;
	hr = CContextImpl::Open(&pContext);
	if (FAILED(hr)) {
		return hr;
	}
	*ppContext = pContext;
	(*ppContext)->AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
