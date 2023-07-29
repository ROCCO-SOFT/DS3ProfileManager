#pragma once

#include <list>

#define FACILITY_PROFILE 9103
enum PROFILE_ERROR
{
	PROFILE_E_INVALID_PARAMETER = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_PROFILE, 0x0001),	///< パラメーターが不正。NULLあるいは不正なメモリ(DEBUGビルドのみ)を指している。
	PROFILE_E_NOT_ENOUGH_MEMORY = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_PROFILE, 0x0002),	///< メモリが足りない。
	PROFILE_E_OPEN_FAILED = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_PROFILE, 0x0003),			///< ファイルが開けなかった。
	PROFILE_E_NOT_FOUND_DS3 = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_PROFILE, 0x0004),		///< DS3のプロファイルデータが見つからなかった。
};

class CContext;
class CBackupSet;
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
	virtual HRESULT RemoveBackup(CBackup *pBackup) = 0;
	virtual CString GetProfilePath() = 0;

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

};

class CBackupEnum : public IUnknown
{
public:

	virtual HRESULT Next(CBackup **ppBackup) = 0;
	virtual HRESULT Reset() = 0;

};

typedef std::list<CComPtr<CBackup>> CBackupList;

HRESULT CreateContext(CContext **ppContext);
