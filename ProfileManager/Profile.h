#pragma once

#include <list>
#include <map>

#define FACILITY_PROFILE 9103
enum PROFILE_ERROR
{
	PROFILE_E_INVALID_PARAMETER = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_PROFILE, 0x0001),	///< パラメーターが不正。NULLあるいは不正なメモリ(DEBUGビルドのみ)を指している。
	PROFILE_E_NOT_ENOUGH_MEMORY = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_PROFILE, 0x0002),	///< メモリが足りない。
	PROFILE_E_OPEN_FAILED = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_PROFILE, 0x0003),			///< ファイルが開けなかった。
	PROFILE_E_NOT_FOUND_PROFILE = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_PROFILE, 0x0004),		///< DS3のプロファイルデータが見つからなかった。
};

#define numof(ar)		(sizeof(ar)/sizeof((ar)[0]))

#define PROFILE_FNAME_FORMAT	_T("%s_%016I64x_")
#define ORDER_INDEX_FILE		_T("_IndexList.dat")
#define AUTOSAVE_NAME			_T("Autosaved")
#define NEWSAVE_NAME			_T("New Save")

#ifdef _DS3
#define APP_NAME				_T("DS3")
#define PROFILE_FOLDER_ID		CSIDL_APPDATA
#define PROFILE_FOLDER			_T("DarkSoulsIII")
#define SL2_FILE				_T("DS30000.sl2")
#elif _ER
#define APP_NAME				_T("ER")
#define PROFILE_FOLDER_ID		CSIDL_APPDATA
#define PROFILE_FOLDER			_T("EldenRing")
#define SL2_FILE				_T("ER0000.sl2")
#elif _SKR
#define APP_NAME				_T("SKR")
#define PROFILE_FOLDER_ID		CSIDL_APPDATA
#define PROFILE_FOLDER			_T("Sekiro")
#define SL2_FILE				_T("S0000.sl2")
#elif _DSR
#define APP_NAME				_T("DSR")
#define PROFILE_FOLDER_ID		CSIDL_PERSONAL
#define PROFILE_FOLDER			_T("FromSoftware\\DARK SOULS REMASTERED")
#define SL2_FILE				_T("DRAKS0005.sl2")
#elif _AC6
#define APP_NAME				_T("AC6")
#define PROFILE_FOLDER_ID		CSIDL_APPDATA
#define PROFILE_FOLDER			_T("ArmoredCoreVI")
#define SL2_FILE				_T("AC60000.sl2")
#endif

class CContext;
class CBackupSet;
class CBackupSetCallback;
class CBackup;
class CBackupEnum;

class CContext : public IUnknown
{
public:

	virtual CBackupSet *GetCurrentBackupSet() = 0;
	virtual CString GetProfilePath() = 0;
	virtual CString GetBackupPath() = 0;

};

class CBackupSet : public IUnknown
{
public:

	virtual HRESULT EnumBackup(CBackupEnum **ppEnum) = 0;
	virtual HRESULT SaveCurrent(CBackup **ppBackup, PCTSTR pszName) = 0;
	virtual HRESULT AddBackup(CBackup *pBackup) = 0;
	virtual HRESULT RemoveBackup(CBackup *pBackup) = 0;
	virtual CString GetProfilePath() = 0;
	virtual CString GetProfileId() = 0;
	virtual HRESULT DoAutoSave(int nMaxAutosaves) = 0;
	virtual HRESULT RegisterCallback(CBackupSetCallback *pCallback) = 0;
	virtual HRESULT UnregisterCallback(CBackupSetCallback *pCallback) = 0;
	virtual HRESULT LoadIndexList() = 0;
	virtual HRESULT SaveIndexList() = 0;

};

class CBackupSetCallback
{
public:

	virtual HRESULT BackupSetAutoSaveSaved(CBackupSet *pBackupSet, CBackup *pBackup) { return E_NOTIMPL; }
	virtual HRESULT BackupSetAutoSaveParged(CBackupSet *pBackupSet, CBackup *pBackup) { return E_NOTIMPL; }

};

class CBackup : public IUnknown
{
public:

	virtual SYSTEMTIME GetCreateTime() = 0;
	virtual CString GetName() = 0;
	virtual HRESULT SetName(PCTSTR pszName) = 0;
	virtual CString GetPath() = 0;
	virtual HRESULT Load() = 0;
	virtual HRESULT Save() = 0;
	virtual HRESULT Delete() = 0;
	virtual HRESULT IsAutosaved() = 0;
	virtual int GetIndex() = 0;
	virtual HRESULT SetIndex(int nIndex) = 0;
 
};

class CBackupEnum : public IUnknown
{
public:

	virtual HRESULT Next(CBackup **ppBackup) = 0;
	virtual HRESULT Reset() = 0;

};

typedef std::list<CComPtr<CBackup>> CBackupList;

HRESULT CreateContext(CContext **ppContext);
