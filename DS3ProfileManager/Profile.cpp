#include "pch.h"
#include "Profile.h"
#include "UnknownImpl.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

#define PROFILE_FNAME_FORMAT	_T("%016I64x_%016I64x_")
#define DS3_PROFILE_FOLDER		_T("DarkSoulsIII\\")
#define DS3_SL2_FILE_NAME		_T("DS30000.sl2")
#define APP_PROFILE_FOLDER		_T("DS3ProfileManager\\")

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
	static HRESULT GetFolderPath(int id, CString &strRet);

};

class CBackupSetImpl : public CBackupSet, public CUnknownImpl
{
public:

	DECLARE_UNKNOWN_IMPL();

	virtual HRESULT EnumBackup(CBackupEnum **ppEnum);
	virtual HRESULT SaveCurrent(CBackup **ppBackup, PCTSTR pszName);
	virtual HRESULT RemoveBackup(CBackup *pBackup);
	virtual CString GetProfilePath();
	static HRESULT Open(CBackupSet **ppList, UINT64 id, PCTSTR pszProfilePath, CContext *pContext);

protected:

	UINT64 m_id;					// カレントID。
	CBackupList m_listBackup;
	CContext *m_pContext;
	CString m_strProfilePath;

	CBackupSetImpl();
	virtual ~CBackupSetImpl();

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

	static HRESULT Open(CBackup **ppBackup, PCTSTR pszFileName, UINT64 idDefault, CContext *pContext, CBackupSet *pSet);
	static HRESULT FormatFname(PTSTR szFname, int cbFname, UINT64 id, SYSTEMTIME sTime, PCTSTR szName);
	static HRESULT ScanFname(PCTSTR szFname, UINT64 &id, SYSTEMTIME &sTime, CString &strName);

protected:

	CTime m_dateCreate;
	CTime m_dateSL2;
	CString m_strName;
	CString m_strPath;
	BOOL m_bReadOnly;
	UINT64 m_id;
	CContext *m_pContext;
	CBackupSet *m_pSet;

	CBackupImpl();
	virtual ~CBackupImpl();

};

class CBackupEnumImpl : public CBackupEnum, public CUnknownImpl
{
public:

	DECLARE_UNKNOWN_IMPL();

	virtual HRESULT Next(CBackup **ppBackup);
	virtual HRESULT Reset();
	static HRESULT Open(CBackupEnum **ppEnum, CBackupList *pList);

protected:

	CBackupList::iterator m_it;
	CBackupList *m_pList;

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

		UINT64 id = -1;
		if (_stscanf_s(find.GetFileTitle(), _T("%016I64x"), &id) != 1) {
			continue;	// 数値化できなかった。
		}

		CString path = find.GetFilePath();
		path.TrimRight(_T("\\"));
		path += _T("\\");

		CComPtr <CBackupSet> pSet;
		hr = CBackupSetImpl::Open(&pSet, id, path, this);
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

HRESULT CContextImpl::GetFolderPath(int id, CString &strRet)
{
	LPITEMIDLIST pidl;
	if (SHGetSpecialFolderLocation(NULL, id, &pidl) != NOERROR) {
		return E_FAIL;
	}
	SHGetPathFromIDList(pidl, strRet.GetBuffer(MAX_PATH));
	strRet.ReleaseBuffer();
	CoTaskMemFree(pidl);
	strRet.TrimRight(_T("\\"));
	strRet += _T("\\");
	if (!PathFileExists(strRet)) {
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CContextImpl::Open(CContext **ppContext)
{
	HRESULT hr = S_OK;

	CString strProfile;
	hr = GetFolderPath(CSIDL_APPDATA, strProfile);
	if (FAILED(hr)) {
		return hr;	// AppDataの場所が取れない。
	}

	strProfile += DS3_PROFILE_FOLDER;
	if (!PathFileExists(strProfile)) {
		return E_FAIL;	// DS3のプロファイルが見つからない。
	}

	CString strBackup;
	hr = GetFolderPath(CSIDL_PERSONAL, strBackup);
	if (FAILED(hr)) {
		return hr;	// Personalの場所が取れない。
	}

	strBackup += APP_PROFILE_FOLDER;
	if (!PathFileExists(strBackup)) {
		if (!CreateDirectory(strBackup, NULL)) {
			return E_FAIL;	// バックアップフォルダが作れない。
		}
	}

	CComPtr <CContextImpl> pThis = new CContextImpl;
	if (!pThis) {
		return E_FAIL;	// メモリ不足。
	}

	pThis->m_strProfilePath = strProfile;
	pThis->m_strBackupPath = strBackup;

	hr = pThis->OpenBackupSet();
	if (FAILED(hr)) {
		return hr;
	}

	if (pThis->m_listBackupSet.empty()) {
		return E_FAIL;	// DS3のセーブデータがない可能性がある。
	}

	*ppContext = pThis;
	(*ppContext)->AddRef();

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CBackupSetImpl::CBackupSetImpl()
	: m_id(NULL)
{

}

CBackupSetImpl::~CBackupSetImpl()
{

}

HRESULT CBackupSetImpl::EnumBackup(CBackupEnum **ppEnum)
{
	HRESULT hr = S_OK;

	CComPtr <CBackupEnum> pEnum;
	hr = CBackupEnumImpl::Open(&pEnum, &m_listBackup);
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
	hr = CBackupImpl::FormatFname(fname, _MAX_FNAME, m_id, sTime, pszName);
	if (FAILED(hr)) {
		return hr;
	}
	CString strTo = m_pContext->GetBackupPath() + fname;

	strFrom += _T("\t");
	strTo += _T("\t");

	fos.hwnd = NULL;
	fos.wFunc = FO_COPY;
	fos.fFlags = FOF_NOCONFIRMMKDIR;
	fos.pFrom = strFrom;
	fos.pTo = strTo;

	// これをちゃんとやらないと動作しない。
	Replace((LPTSTR)fos.pFrom, _T('\t'), _T('\0'));
	Replace((LPTSTR)fos.pTo, _T('\t'), _T('\0'));

	SHFileOperation(&fos);

	CComPtr <CBackup> pBackup;
	hr = CBackupImpl::Open(&pBackup, strTo, m_id, m_pContext, this);
	if (FAILED(hr)) {
		return hr;
	}

	*ppBackup = pBackup;
	(*ppBackup)->AddRef();

	return S_OK;
}

HRESULT CBackupSetImpl::RemoveBackup(CBackup *pBackup)
{
	auto it = m_listBackup.begin();
	while (it != m_listBackup.end()) {
		if (*it == pBackup) {
			it = m_listBackup.erase(it);
		}
		else {
			it++;
		}
	}
	return S_OK;
}

HRESULT CBackupSetImpl::Open(CBackupSet **ppList, UINT64 id, PCTSTR pszProfilePath, CContext *pContext)
{
	HRESULT hr = S_OK;

	CComPtr <CBackupSetImpl> pThis = new CBackupSetImpl;
	if (!pThis) {
		return E_FAIL;
	}

	pThis->m_id = id;
	pThis->m_pContext = pContext;
	pThis->m_strProfilePath = pszProfilePath;

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

		CString path = find.GetFilePath();
		path.TrimRight(_T("\\"));
		path += _T("\\");

		CComPtr <CBackup> pBackup;
		hr = CBackupImpl::Open(&pBackup, path, pThis->m_id, pContext, pThis);
		if (FAILED(hr)) {
			continue;
		}

		pThis->m_listBackup.push_back(pBackup);
	}

	*ppList = pThis;
	(*ppList)->AddRef();

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CBackupImpl::CBackupImpl()
{

}

CBackupImpl::~CBackupImpl()
{

}

HRESULT CBackupImpl::FormatFname(PTSTR szFname, int cbFname, UINT64 id, SYSTEMTIME sTime, PCTSTR szName)
{
	try {
		FILETIME fTime;
		SystemTimeToFileTime(&sTime, &fTime);
		UINT64 time = ((UINT64)fTime.dwHighDateTime << 32) | fTime.dwLowDateTime;
		_stprintf_s(szFname, cbFname, PROFILE_FNAME_FORMAT, id, time);
		_tcscat_s(szFname, cbFname, szName);
	}
	catch (...) {
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CBackupImpl::ScanFname(PCTSTR szFname, UINT64 &id, SYSTEMTIME &sTime, CString &strName)
{
	TCHAR name[_MAX_FNAME] = { NULL };
	UINT64 _id = -1;
	UINT64 time = -1;
	FILETIME fTime;

	try {
		_stscanf_s(szFname, PROFILE_FNAME_FORMAT, &_id, &time);
	}
	catch (...) {
		return E_FAIL;
	}

	if (time == -1) {
		return E_FAIL;
	}

	try {
		TCHAR temp[_MAX_FNAME];
		_stprintf_s(temp, PROFILE_FNAME_FORMAT, _id, time);
		_tcscpy_s(name, szFname + _tcslen(temp));
	}
	catch (...) {
		return E_FAIL;
	}

	id = _id;
	fTime.dwHighDateTime = (UINT32)(time >> 32);
	fTime.dwLowDateTime = (UINT32)time;
	FileTimeToSystemTime(&fTime, &sTime);
	strName = name;

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
	hr = CBackupImpl::FormatFname(fname, _MAX_FNAME, m_id, sTime, pszName);
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

	// これをちゃんとやらないと動作しない。
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

	// これをちゃんとやらないと動作しない。
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

	// これをちゃんとやらないと動作しない。
	Replace((LPTSTR)fos.pFrom, _T('\t'), _T('\0'));
	Replace((LPTSTR)fos.pTo, _T('\t'), _T('\0'));

	SHFileOperation(&fos);

	CString strSL2 = m_strPath + DS3_SL2_FILE_NAME;
	if (!PathFileExists(strSL2)) {
		return E_FAIL;
	}

	WIN32_FILE_ATTRIBUTE_DATA attr = { NULL };
	GetFileAttributesEx(strSL2, GetFileExInfoStandard, &attr);
	SYSTEMTIME sTimeSL2;
	FileTimeToSystemTime(&attr.ftLastWriteTime, &sTimeSL2);
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

	// これをちゃんとやらないと動作しない。
	Replace((LPTSTR)fos.pFrom, _T('\t'), _T('\0'));

	SHFileOperation(&fos);

	hr = m_pSet->RemoveBackup(this);
	if (FAILED(hr)) {
		return hr;
	}

	return S_OK;
}

HRESULT CBackupImpl::Open(CBackup **ppBackup, PCTSTR pszPath, UINT64 idDefault, CContext *pContext, CBackupSet *pSet)
{
	HRESULT hr = S_OK;

	if (!PathFileExists(pszPath)) {
		return E_FAIL;
	}

	CString strPath = pszPath;
	strPath.TrimRight(_T("\\"));	// わざと右の\を取り除いている。

	TCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_tsplitpath_s(strPath, drive, dir, fname, ext);

	strPath += _T("\\");	// \を戻す。

	UINT64 id;
	SYSTEMTIME sTime;
	CString strName;
	hr = ScanFname(fname, id, sTime, strName);
	if (FAILED(hr)) {
		return hr;
	}

	if (idDefault != -1) {
		if (id != idDefault) {
			return E_FAIL;
		}
	}

#if 1
	CString strSL2 = strPath + DS3_SL2_FILE_NAME;
	if (!PathFileExists(strSL2)) {
		return E_FAIL;
	}

	WIN32_FILE_ATTRIBUTE_DATA attr = { NULL };
	GetFileAttributesEx(strSL2, GetFileExInfoStandard, &attr);
	SYSTEMTIME sTimeSL2;
	FileTimeToSystemTime(&attr.ftLastWriteTime, &sTimeSL2);
#endif

	CComPtr <CBackupImpl> pThis = new CBackupImpl;
	if (!pThis) {
		return E_FAIL;
	}

	pThis->m_dateCreate = sTime;
	pThis->m_dateSL2 = sTimeSL2;
	pThis->m_strPath = strPath;
	pThis->m_strName = strName;
	pThis->m_id = id;
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
	if (m_it == m_pList->end()) {
		return S_FALSE;
	}
	*ppBackup = *(m_it++);
	(*ppBackup)->AddRef();
	return S_OK;
}

HRESULT CBackupEnumImpl::Reset()
{
	m_it = m_pList->begin();
	return S_OK;
}

HRESULT CBackupEnumImpl::Open(CBackupEnum **ppEnum, CBackupList *pList)
{
	CComPtr <CBackupEnumImpl> pThis = new CBackupEnumImpl;
	if (!pThis) {
		return E_FAIL;
	}
	pThis->m_pList = pList;
	pThis->m_it = pList->begin();
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
