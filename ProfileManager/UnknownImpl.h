#pragma once

class CUnknownImpl
{
protected:

#pragma pack(push)
#pragma pack(4)
	ULONG m_nRef;
#pragma pack(pop)

protected:

	CUnknownImpl()
	{
		m_nRef = 0;
	}

	virtual ~CUnknownImpl()
	{
		ASSERT(m_nRef == 0);
	}

public:

	STDMETHOD_(ULONG, AddRef)()
	{
		//ASSERT(IS_ALIGNED(&m_nRef, 4));
		return InterlockedIncrement(&m_nRef);
	}

	STDMETHOD_(ULONG, Release)()
	{
		//ASSERT(IS_ALIGNED(&m_nRef, 4));
		int nRef = InterlockedDecrement(&m_nRef);
		ASSERT(nRef >= 0);
		if (nRef == 0) {
			delete this;
		}
		return nRef;
	}

	STDMETHOD_(HRESULT, QueryInterface)(
                /* [in] */ REFIID riid,
                /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		return E_NOTIMPL;
	}

	STDMETHOD_(ULONG, GetRefCount)() const
	{
		return m_nRef;
	}

};

#define DECLARE_UNKNOWN_IMPL() \
	STDMETHOD_(ULONG, AddRef)() {	\
		return CUnknownImpl::AddRef(); 	\
	}	\
	STDMETHOD_(ULONG, Release)() {	\
		return CUnknownImpl::Release();	\
	}	\
	STDMETHOD_(HRESULT, QueryInterface)(REFIID riid, __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject) {	\
		return CUnknownImpl::QueryInterface(riid, ppvObject);	\
	}	\
	STDMETHOD_(ULONG, GetRefCount)() const {	\
		return CUnknownImpl::GetRefCount();	\
	}
