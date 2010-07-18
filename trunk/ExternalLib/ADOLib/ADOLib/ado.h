#pragma once
#include "unidef.h"
#include <atlcomtime.h>

#import "C:\Program Files\Common Files\System\ADO\msado15.dll" rename("EOF", "EndOfFile") no_namespace

#define ISFAIL(a) if(!(a)) break

using namespace std;

struct SAdoConfig
{
private:
	tstring m_tstrConnectingString;
	tstring m_tstrInitialCatalog;
	tstring m_tstrDataSource;
	tstring m_tstrUserID;
	tstring m_tstrPassword;
	tstring m_tstrProvider;
	tstring m_tstrDSN;
	int m_nConnectionTimeout;
	int m_nCommandTimeout;
	bool m_bRetryConnection;

	int m_nMaxConnectionPool;

	void SetProvider(TCHAR* pString = _T("SQLOLEDB.1")){m_tstrProvider = pString;};

public:
	void SetInitCatalog(TCHAR* pString) {m_tstrInitialCatalog = pString;}
	void SetUserID(TCHAR* pUserID){m_tstrUserID = pUserID;};
	void SetPassword(TCHAR* pPassword){m_tstrPassword = pPassword;};
	void SetIP(TCHAR* pString){m_tstrDataSource = _T(";Data Source="); m_tstrDataSource += pString; SetProvider();};
	void SetDSN(TCHAR* pString){m_tstrDSN = _T(";DSN="); m_tstrDSN += pString;};
	void SetCommandTimeout(int nCommendTimeout){m_nCommandTimeout = nCommendTimeout;};
	void SetConnectionTimeout(int nConnectionTimeout){m_nConnectionTimeout = nConnectionTimeout;};
	void SetRetryConnection(bool bRetryConnection){m_bRetryConnection = bRetryConnection;};
	void SetMaxConnectionPool(int nMaxConnectionPool){m_nMaxConnectionPool = nMaxConnectionPool;};

	tstring GetConnectionString(){
		if(m_tstrDataSource.empty())
			m_tstrConnectingString = m_tstrDSN;
		else
			m_tstrConnectingString = m_tstrDataSource;

		return m_tstrConnectingString;
	};
	tstring& GetUserID(){return m_tstrUserID;};
	tstring& GetPassword(){return m_tstrPassword;};
	tstring& GetInitCatalog(){return m_tstrInitialCatalog;};
	tstring& GetProvider(){return m_tstrProvider;};
	tstring& GetDSN(){return m_tstrDSN;};
	int GetConnectionTimeout(){return m_nConnectionTimeout;};
	int GetCommandTimeout(){return m_nCommandTimeout;};
	bool GetRetryConnection(){return m_bRetryConnection;};
	int GetMaxConnectionPool(){return m_nMaxConnectionPool;};

	SAdoConfig()
		:m_nConnectionTimeout(0),
		m_nCommandTimeout(0),
		m_bRetryConnection(false)
	{
	}
};



class CAdo
{
public:
	CAdo(SAdoConfig&);
	~CAdo();

	/**
	\remarks	���� �ʱ�ȭ
	\par		����Ǯ���� �����ϱ� ���� �̰����� �ʱ�ȭ ������
	*/
	void Init();

	/**
	\remarks	���� ����
	\par		IP �� DSN ����
	\param		��ġ �۾��� ��� adUseClientBatch �ɼ� ���
	\return		����(TRUE) ����(FLASE)
	*/
	BOOL Open(CursorLocationEnum CursorLocation=adUseClient);

	/**
	\remarks	�翬�� �ɼ��� ���� ��� �翬�� �õ�
	*/
	BOOL RetryOpen();

	/**
	\remarks	���� ����
	*/
	void Close();

	/**
	\remarks	Ŀ�ؼ�Ǯ���� �����ϱ� ���� Ŀ�ǵ� ��ü �����
	*/
	void Release();

	inline void SetQuery(IN TCHAR* tszQuery){m_tstrQuery = tszQuery;};
	void SetConnectionMode(ConnectModeEnum nMode) {m_pConnection->PutMode(nMode);};

	/**
	\remarks	����� Ʈ����� ���
	\par		CScopedAdo ���� �� �Ҹ��ҽ� Ʈ����� �ɼ��� ���ȴ�. CScopedAdo Ŭ������ ��������.
	*/
	void SetAutoCommit(bool bAutoCommit){m_bAutoCommit = bAutoCommit;};
	bool GetAutoCommit(){return m_bAutoCommit;};
	inline void BeginTransaction()
	{
		try{
			m_pConnection->BeginTrans();
		} catch (_com_error &e) {
			dump_com_error(e);
			dump_user_error();
			return;
		}
	};
	inline void CommitTransaction()
	{
		try{
			m_pConnection->CommitTrans();
		}catch(_com_error &e){
			dump_com_error(e);
			dump_user_error();
			return;
		}
	};
	inline void RollbackTransaction()
	{
		try{
			m_pConnection->RollbackTrans();
		}catch(_com_error &e){
			dump_com_error(e);
			dump_user_error();
			return;
		}
	};


	inline BOOL IsSuccess(){
		if(FALSE == m_IsSuccess)
		{
			dump_user_error();
			m_tstrQuery.erase();
			m_tstrCommand.erase();
			m_tstrColumnName.erase();
			m_tstrParameterName.erase();
		}
		return m_IsSuccess;
	};
	inline void SetSuccess(BOOL bIsSuccess){m_IsSuccess = bIsSuccess;};
	inline void SetCommit(BOOL bIsSuccess){SetSuccess(bIsSuccess);};
	inline BOOL GetSuccess(){return m_IsSuccess;};

	void LOG(TCHAR*, ...);
	void dump_com_error(_com_error&);
	void dump_user_error();

	BOOL GetFieldCount(int&);
	void MoveNext();
	BOOL GetEndOfFile();
	BOOL NextRecordSet();

	/**
	\remarks	���ν��� �� SQL Text�� �����Ѵ�.
	\par		�ΰ���� adCmdStoreProc, adCmdTextó�� ����
	\param		CommandTypeEnum, ExecuteOptionEnum
	\return		����(TRUE) ����(FLASE)
	*/
	BOOL Execute(CommandTypeEnum CommandType = adCmdStoredProc, ExecuteOptionEnum OptionType = adOptionUnspecified);

	/**
	\remarks	����/�Ǽ�/��¥�ð� �ʵ尪�� �д´�.
	\par		���� ���� null�̸� ���и� �����Ѵ�.
	\return		����(TRUE) ����(FLASE)
	*/
	template<typename T> BOOL GetFieldValue(IN TCHAR* tszName, OUT T& Value)
	{
		m_tstrCommand = _T("GetFieldValue(T)");
		m_tstrColumnName = tszName;

		try	{
			_variant_t vFieldValue = m_pRecordset->GetCollect(tszName);

			switch(vFieldValue.vt)
			{
			case VT_BOOL:		//bool
			case VT_I1:			//BYTE WORD
			case VT_I2:
			case VT_UI1:
			case VT_I4:			//DWORD
			case VT_DECIMAL:	//INT64
			case VT_R8:			//float double
			case VT_DATE:
				Value = vFieldValue;
				break;
			case VT_NULL:
			case VT_EMPTY:
				m_tstrColumnName += _T(" null value");
				dump_user_error();
				return FALSE;
			default:
				TCHAR tsz[7]={0,};
				m_tstrColumnName += _T(" type error(vt = ");
				m_tstrColumnName += _itot(vFieldValue.vt, tsz, 10);
				m_tstrColumnName += _T(" ) ");
				m_IsSuccess = FALSE;
				return FALSE;
			}
		} catch (_com_error &e) {
			dump_com_error(e);
			return FALSE;
		}
		return TRUE;
	}


	/**
	\remarks	������ �ʵ尪�� �д´�.
	\par		���� ���� null�̰ų� ���۰� �۴ٸ� ���и� �����Ѵ�.
	\param		���� ������ ���� ������ ũ��
	\return		����(TRUE) ����(FLASE)
	*/
	BOOL GetFieldValue(IN TCHAR*, OUT TCHAR*, IN unsigned int);

	/**
	\remarks	binary �ʵ尪�� �д´�.
	\par		���� ���� null�̰ų� ���۰� �۴ٸ� ���и� �����Ѵ�.
	\param		���� binary�� ���� ������ ũ��, ���� binary ũ��
	\return		����(TRUE) ����(FLASE)
	*/
	BOOL GetFieldValue(IN TCHAR*, OUT BYTE*, IN int, OUT int&);
	

	/**
	\remarks	����/�Ǽ�/��¥�ð� Ÿ���� �Ķ���� ����
	\par		null���� �Ķ���� ������ CreateNullParameter�� ���
	*/
	template<typename T> void CreateParameter(IN TCHAR* tszName,IN enum DataTypeEnum Type, IN enum ParameterDirectionEnum Direction, IN T rValue)
	{
		if(!IsSuccess()) return;

		m_tstrCommand = _T("CreateParameter(T)");
		m_tstrParameterName = tszName;
		try	{
			_ParameterPtr pParametor = m_pCommand->CreateParameter(tszName,Type,Direction, 0);
			m_pCommand->Parameters->Append(pParametor);
			pParametor->Value = static_cast<_variant_t>(rValue);
		} catch (_com_error &e) {
			dump_com_error(e);
		}

		return;
	}

	/**
	\remarks	����/�Ǽ�/��¥�ð� Ÿ���� null�� �Ķ���� ����
	*/
	void CreateNullParameter(IN TCHAR*, IN enum DataTypeEnum, IN enum ParameterDirectionEnum);

	/**
	\remarks	���ڿ� Ÿ�� �Ķ���� ����, ���� ������ �ּ� 0���� Ŀ�� �Ѵ�. null�� ������ TCHAR*�� NULL���� �ѱ��.
	*/
	void CreateParameter(IN TCHAR*,IN enum DataTypeEnum, IN enum ParameterDirectionEnum,
		IN TCHAR*, IN int);
	/**
	\remarks	binary Ÿ�� �Ķ���� ����, ���� ������ �ּ� 0���� Ŀ�� �Ѵ�. null�� ������ BYTE*�� NULL���� �ѱ��.
	*/
	void CreateParameter(IN TCHAR*,IN enum DataTypeEnum, IN enum ParameterDirectionEnum,
		IN BYTE*, IN int);


	/**
	\remarks	����/�Ǽ�/��¥�ð� Ÿ���� �Ķ���Ͱ� ����
	\par		null���� �Ķ���� ������ UpdateNullParameter�� ���
	*/
	template<typename T>
		void UpdateParameter(IN TCHAR* tszName, IN T rValue)
	{
		if(!IsSuccess()) return;

		m_tstrCommand = _T("UpdateParameter(T)");
		m_tstrParameterName = tszName;
		try	{
			m_pCommand->Parameters->GetItem(tszName)->Value = static_cast<_variant_t>(rValue);
		} catch (_com_error &e) {
			dump_com_error(e);
		}

		return;
	}

	/**
	\remarks	����/�Ǽ�/��¥�ð� Ÿ���� �Ķ���� ���� null�� ����
	*/
	void UpdateNullParameter(IN TCHAR*);

	/**
	\remarks	���ڿ� Ÿ�� �Ķ���� ����, ���� ������ �ּ� 0���� Ŀ�� �Ѵ�. null�� ���� TCHAR*�� NULL���� �ѱ��.
	*/
	void UpdateParameter(IN TCHAR*, IN TCHAR*, IN int);

	/**
	\remarks	binary Ÿ�� �Ķ���� ����, ���� ������ �ּ� 0���� Ŀ�� �Ѵ�. null�� ���� BYTE*�� NULL���� �ѱ��.
	*/
	void UpdateParameter(IN TCHAR*, IN BYTE*, IN int);

	/**
	\remarks	����/�Ǽ�/��¥�ð� Ÿ���� �Ķ���� �� �б�
	*/
	template<typename T>
		BOOL GetParameter(TCHAR* tszName, OUT T& Value)
	{
		if(!IsSuccess()) return FALSE;
		m_IsSuccess = FALSE;

		m_tstrCommand = _T("GetParameter(T)");
		m_tstrParameterName = tszName;
		try	{
			_variant_t& vFieldValue = m_pCommand->Parameters->GetItem(tszName)->Value;

			switch(vFieldValue.vt)
			{
			case VT_BOOL:	//bool
			case VT_I1:
			case VT_I2:		//BYTE WORD
			case VT_UI1:
			case VT_I4:		//DWORD
			case VT_DECIMAL: //INT64
			case VT_R8:	//float double
			case VT_DATE:
				Value = vFieldValue;
				break;
			case VT_NULL:
			case VT_EMPTY:
				m_tstrColumnName += _T(" null value");
				dump_user_error();
				return FALSE;
			default:
				TCHAR tsz[7]={0,};
				m_tstrParameterName += _T(" type error(vt = ");
				m_tstrParameterName += _itot(vFieldValue.vt, tsz, 10);
				m_tstrParameterName += _T(" ) ");
				m_IsSuccess = FALSE;
				return FALSE;
			}
		} catch (_com_error &e)	{
			dump_com_error(e);
			return FALSE;
		}
		return m_IsSuccess = TRUE;
	}

	/**
	\remarks	������ �Ķ���Ͱ��� �д´�.
	\par		���� ���� null�̰ų� ���۰� �۴ٸ� ���и� �����Ѵ�.
	\param		���� ������ ���� ������ ũ��
	\return		����(TRUE) ����(FLASE)
	*/
	BOOL GetParameter(IN TCHAR*, OUT TCHAR*, IN unsigned int);

	/**
	\remarks	���̳ʸ��� �Ķ���Ͱ��� �д´�.
	\par		���� ���� null�̰ų� ���۰� �۴ٸ� ���и� �����Ѵ�.
	\param		���� ������ ���� ������ ũ��, ���� ������ ũ��
	\return		����(TRUE) ����(FLASE)
	*/
	BOOL GetParameter(IN TCHAR*, OUT BYTE*, IN int, OUT int&);
private:
	_ConnectionPtr m_pConnection;      
	_RecordsetPtr m_pRecordset;
	_CommandPtr m_pCommand;

	tstring m_tstrConnectingString;
	tstring m_tstrUserID;
	tstring m_tstrPassword;
	tstring m_tstrInitCatalog;
	tstring m_tstrProvider;
	tstring m_tstrDSN;
	int m_nConnectionTimeout;
	int m_nCommandTimeout;
	bool m_bRetryConnection;
	bool m_bAutoCommit;


	tstring m_tstrQuery;

	BOOL m_IsSuccess;
	tstring m_tstrCommand;
	tstring m_tstrColumnName;
	tstring m_tstrParameterName;

	CAdo(const CAdo&);
	CAdo& operator= (const CAdo&);
};
