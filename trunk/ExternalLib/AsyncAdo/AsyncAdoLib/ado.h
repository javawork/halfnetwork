#pragma once

#include <string>
#include <atlcomtime.h>
#include "DBConfig.h"

#import "C:\Program Files\Common Files\System\ADO\msado15.dll" rename("EOF", "EndOfFile") no_namespace

#define ISFAIL(a) if(!(a)) break

using namespace std;

namespace asyncadodblib
{
	class AdoDB
	{
	public:
		AdoDB( DBConfig& );
		~AdoDB();

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
		bool Open( CursorLocationEnum CursorLocation=adUseClient );

		/**
		\remarks	�翬�� �ɼ��� ���� ��� �翬�� �õ�
		*/
		bool RetryOpen();

		/**
		\remarks	���� ����
		*/
		void Close();

		/**
		\remarks	Ŀ�ؼ�Ǯ���� �����ϱ� ���� Ŀ�ǵ� ��ü �����
		*/
		void Release();

		inline void SetQuery( IN wchar_t* pszQuery ) { m_strQuery = pszQuery; }
		void SetConnectionMode( ConnectModeEnum nMode ) { m_pConnection->PutMode(nMode); }

		/**
		\remarks	����� Ʈ����� ���
		\par		CScopedAdo ���� �� �Ҹ��ҽ� Ʈ����� �ɼ��� ���ȴ�. CScopedAdo Ŭ������ ��������.
		*/
		void SetAutoCommit( const bool bAutoCommit ) { m_bAutoCommit = bAutoCommit; }
		bool GetAutoCommit() { return m_bAutoCommit; }

		inline void BeginTransaction()
		{
			try
			{
				m_pConnection->BeginTrans();
			}
			catch (_com_error &e) 
			{
				dump_com_error(e);
				dump_user_error();
				return;
			}
		}

		inline void CommitTransaction()
		{
			try
			{
				m_pConnection->CommitTrans();
			}
			catch(_com_error &e)
			{
				dump_com_error(e);
				dump_user_error();
				return;
			}
		}

		inline void RollbackTransaction()
		{
			try
			{
				m_pConnection->RollbackTrans();
			}
			catch(_com_error &e)
			{
				dump_com_error(e);
				dump_user_error();
				return;
			}
		}

		inline bool IsSuccess()
		{
			if( m_IsSuccess == false )
			{
				dump_user_error();
				m_strQuery.erase();
				m_strCommand.erase();
				m_strColumnName.erase();
				m_strParameterName.erase();
			}

			return m_IsSuccess;
		}

		inline void SetSuccess( bool bIsSuccess ) { m_IsSuccess = bIsSuccess; }
		inline void SetCommit( bool bIsSuccess ) { SetSuccess(bIsSuccess); }
		inline bool GetSuccess() { return m_IsSuccess; }

		void LOG(wchar_t*, ...);
		void dump_com_error(_com_error&);
		void dump_user_error();

		bool GetFieldCount( int& );
		void MoveNext();
		bool GetEndOfFile();
		bool NextRecordSet();

		/**
		\remarks	���ν��� �� SQL Text�� �����Ѵ�.
		\par		�ΰ���� adCmdStoreProc, adCmdTextó�� ����
		\param		CommandTypeEnum, ExecuteOptionEnum
		\return		����(TRUE) ����(FLASE)
		*/
		bool Execute(CommandTypeEnum CommandType = adCmdStoredProc, ExecuteOptionEnum OptionType = adOptionUnspecified);

		/**
		\remarks	����/�Ǽ�/��¥�ð� �ʵ尪�� �д´�.
		\par		���� ���� null�̸� ���и� �����Ѵ�.
		\return		����(TRUE) ����(FLASE)
		*/
		template<typename T> bool GetFieldValue(IN TCHAR* tszName, OUT T& Value)
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
		bool GetFieldValue( IN wchar_t*, OUT wchar_t*, IN unsigned int );

		/**
		\remarks	binary �ʵ尪�� �д´�.
		\par		���� ���� null�̰ų� ���۰� �۴ٸ� ���и� �����Ѵ�.
		\param		���� binary�� ���� ������ ũ��, ���� binary ũ��
		\return		����(TRUE) ����(FLASE)
		*/
		bool GetFieldValue( IN wchar_t*, OUT BYTE*, IN int, OUT int& );
	

		/**
		\remarks	����/�Ǽ�/��¥�ð� Ÿ���� �Ķ���� ����
		\par		null���� �Ķ���� ������ CreateNullParameter�� ���
		*/
		template<typename T> void CreateParameter(IN wchar_t* pszName,IN enum DataTypeEnum Type, IN enum ParameterDirectionEnum Direction, IN T rValue)
		{
			if( !IsSuccess() ) { 
				return;
			}

			m_strCommand = L"CreateParameter(T)";
			m_strParameterName = pszName;

			try	
			{
				_ParameterPtr pParametor = m_pCommand->CreateParameter(pszName,Type,Direction, 0);
				m_pCommand->Parameters->Append(pParametor);
				pParametor->Value = static_cast<_variant_t>(rValue);
			} 
			catch (_com_error &e) 
			{
				dump_com_error(e);
			}

			return;
		}

		/**
		\remarks	����/�Ǽ�/��¥�ð� Ÿ���� null�� �Ķ���� ����
		*/
		void CreateNullParameter(IN wchar_t*, IN enum DataTypeEnum, IN enum ParameterDirectionEnum);

		/**
		\remarks	���ڿ� Ÿ�� �Ķ���� ����, ���� ������ �ּ� 0���� Ŀ�� �Ѵ�. null�� ������ wchar_t*�� NULL���� �ѱ��.
		*/
		void CreateParameter(IN wchar_t*,IN enum DataTypeEnum, IN enum ParameterDirectionEnum,
								IN wchar_t*, IN int);
		/**
		\remarks	binary Ÿ�� �Ķ���� ����, ���� ������ �ּ� 0���� Ŀ�� �Ѵ�. null�� ������ BYTE*�� NULL���� �ѱ��.
		*/
		void CreateParameter(IN wchar_t*,IN enum DataTypeEnum, IN enum ParameterDirectionEnum,
								IN BYTE*, IN int);


		/**
		\remarks	����/�Ǽ�/��¥�ð� Ÿ���� �Ķ���Ͱ� ����
		\par		null���� �Ķ���� ������ UpdateNullParameter�� ���
		*/
		template<typename T>
			void UpdateParameter(IN wchar_t* pszName, IN T rValue)
		{
			if( !IsSuccess() ) { 
				return;
			}

			m_trCommand = L"UpdateParameter(T)";
			m_strParameterName = pszName;
			
			try	
			{
				m_pCommand->Parameters->GetItem(pszName)->Value = static_cast<_variant_t>(rValue);
			} 
			catch (_com_error &e) 
			{
				dump_com_error(e);
			}

			return;
		}

		/**
		\remarks	����/�Ǽ�/��¥�ð� Ÿ���� �Ķ���� ���� null�� ����
		*/
		void UpdateNullParameter(IN wchar_t*);

		/**
		\remarks	���ڿ� Ÿ�� �Ķ���� ����, ���� ������ �ּ� 0���� Ŀ�� �Ѵ�. null�� ���� TCHAR*�� NULL���� �ѱ��.
		*/
		void UpdateParameter(IN wchar_t*, IN wchar_t*, IN int);

		/**
		\remarks	binary Ÿ�� �Ķ���� ����, ���� ������ �ּ� 0���� Ŀ�� �Ѵ�. null�� ���� BYTE*�� NULL���� �ѱ��.
		*/
		void UpdateParameter(IN wchar_t*, IN BYTE*, IN int);

		/**
		\remarks	����/�Ǽ�/��¥�ð� Ÿ���� �Ķ���� �� �б�
		*/
		template<typename T>
			bool GetParameter(wchar_t* pszName, OUT T& Value)
		{
			if( !IsSuccess() ) { 
				return false;
			}

			m_IsSuccess = false;

			m_strCommand = L"GetParameter(T)";
			m_strParameterName = pszName;

			try	
			{
				_variant_t& vFieldValue = m_pCommand->Parameters->GetItem(pszName)->Value;

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
					m_strColumnName += L" null value";
					dump_user_error();
					return false;
				default:
					wchar_t sz[7]={0,};
					m_strParameterName += L" type error(vt = ";
					m_strParameterName += _itow( vFieldValue.vt, sz, 10 );
					m_strParameterName += L" ) ";
					m_IsSuccess = false;
					return false;
				}
			} 
			catch (_com_error &e)	
			{
				dump_com_error(e);
				return false;
			}

			m_IsSuccess = false;

			return m_IsSuccess;
		}

		/**
		\remarks	������ �Ķ���Ͱ��� �д´�.
		\par		���� ���� null�̰ų� ���۰� �۴ٸ� ���и� �����Ѵ�.
		\param		���� ������ ���� ������ ũ��
		\return		����(TRUE) ����(FLASE)
		*/
		bool GetParameter(IN wchar_t*, OUT wchar_t*, IN unsigned int);

		/**
		\remarks	���̳ʸ��� �Ķ���Ͱ��� �д´�.
		\par		���� ���� null�̰ų� ���۰� �۴ٸ� ���и� �����Ѵ�.
		\param		���� ������ ���� ������ ũ��, ���� ������ ũ��
		\return		����(TRUE) ����(FLASE)
		*/
		bool GetParameter(IN wchar_t*, OUT BYTE*, IN int, OUT int&);
	
	
	protected:
		_ConnectionPtr m_pConnection;      
		_RecordsetPtr m_pRecordset;
		_CommandPtr m_pCommand;

		std::wstring m_strConnectingString;
		std::wstring m_strUserID;
		std::wstring m_strPassword;
		std::wstring m_strInitCatalog;
		std::wstring m_strProvider;
		std::wstring m_strDSN;
		
		int m_nConnectionTimeout;
		int m_nCommandTimeout;
		bool m_bRetryConnection;
		bool m_bAutoCommit;


		std::wstring m_strQuery;

		bool m_IsSuccess;
		
		std::wstring m_strCommand;
		std::wstring m_strColumnName;
		std::wstring m_strParameterName;

		AdoDB(const AdoDB&);
		AdoDB& operator= (const AdoDB&);
	};
}
