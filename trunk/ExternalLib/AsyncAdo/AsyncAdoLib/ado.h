﻿#pragma once

#include <string>
#include <atlcomtime.h>
#include "DBConfig.h"

// MS가 제공하는 ado 라이브러리가 있는 위치
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

		/// <summary>
		/// 초기화 - 연결풀에서 재사용하기 위해 이곳에서 초기화 시켜줌 </summary>
		void Init();

		/// <summary>
		/// 연결 설정 - IP 및 DSN 접속 
		/// <param name="CursorLocation"> 배치 작업일 경우 adUseClientBatch 옵션 사용 </param>
		/// <returns> 성공(TRUE) 실패(FLASE) </returns>
		/// </summary>
		bool Open( CursorLocationEnum CursorLocation=adUseClient );

		/// <summary>
		/// 재연결 옵션이 있을 경우 재연결 시도 </summary>
		bool RetryOpen();

		/// <summary>
		/// 연결 종료 </summary>
		void Close();

		/// <summary>
		///  커넥션풀에서 재사용하기 위한 커맨드 객체 재생성 </summary>
		void Release();

		/// <summary>
		/// DB 작업을 요청할 쿼리문 </summary>
		void SetQuery( IN wchar_t* pszQuery ) { m_strQuery = pszQuery; }

		/// <summary>
		///  권한 지정 </summary>
		void SetConnectionMode( ConnectModeEnum nMode ) { m_pConnection->PutMode(nMode); }

		/// <summary>
		/// 명시적 트랜잭션 사용. bAutoCommit이 false인 경우 명시적으로 커밋이나 롤백을 해야한다. </summary>
		void SetAutoCommit( const bool bAutoCommit ) 
		{ 
			m_bAutoCommit = bAutoCommit; 

			if( m_bAutoCommit == false ) { 
				m_bCanCommitTransaction = false;
			}
		}
		
		/// <summary>
		/// 자동 커밋 가능 여부 </summary>
		bool CanAutoCommit() { return m_bAutoCommit; }

		/// <summary>
		/// 트랜잭션을 건다 </summary>
		void BeginTransaction()
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

		/// <summary>
		/// 커밋. 트랜잭션을 걸었을 때 사용 </summary>
		void CommitTransaction()
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

		/// <summary>
		/// 롤백. 트랜잭션을 걸었을 때 사용 </summary>
		void RollbackTransaction()
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

		/// <summary>
		/// 쿼리 작업 성공 여부 </summary>
		bool IsSuccess()
		{
			if( m_bCanGetParamGetFiled == false )
			{
				dump_user_error();
				m_strQuery.erase();
				m_strCommand.erase();
				m_strColumnName.erase();
				m_strParameterName.erase();
			}

			return m_bCanGetParamGetFiled;
		}

		/// <summary>
		/// 
		/// </summary>
		bool CanGetParamGetFiled() { return m_bCanGetParamGetFiled; }
		
		/// <summary>
		/// 
		/// </summary>
		void SetCommitTransaction() { m_bCanCommitTransaction = true; }
		
		/// <summary>
		/// 
		/// </summary
		bool CanCommitTransaction() { return m_bCanCommitTransaction; }

		/// <summary>
		/// 
		/// </summary
		void LOG(wchar_t*, ...);
		
		/// <summary>
		/// 
		/// </summary
		void dump_com_error(_com_error&);
		
		/// <summary>
		/// 
		/// </summary
		void dump_user_error();

		/// <summary>
		/// 
		/// </summary
		bool GetFieldCount( int& );

		/// <summary>
		/// 
		/// </summary
		void MoveNext();

		/// <summary>
		/// 
		/// </summary
		bool GetEndOfFile();

		/// <summary>
		/// 
		/// </summary
		bool NextRecordSet();

		/**
		\remarks	프로시저 및 SQL Text을 실행한다.
		\par		부가기능 adCmdStoreProc, adCmdText처리 가능
		\param		CommandTypeEnum, ExecuteOptionEnum
		\return		성공(TRUE) 실패(FLASE)
		*/
		/// <summary>
		/// 
		/// </summary
		bool Execute(CommandTypeEnum CommandType = adCmdStoredProc, ExecuteOptionEnum OptionType = adOptionUnspecified);

		/**
		\remarks	정수/실수/날짜시간 필드값을 읽는다.
		\par		읽은 값이 null이면 실패를 리턴한다.
		\return		성공(TRUE) 실패(FLASE)
		*/
		/// <summary>
		/// 
		/// </summary
		template<typename T> bool GetFieldValue(IN WCHAR* szName, OUT T& Value)
		{
			m_strCommand = L"GetFieldValue(T)";
			m_strColumnName = szName;

			try	{
				_variant_t vFieldValue = m_pRecordset->GetCollect(szName);

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
					m_strColumnName += _T(" null value");
					dump_user_error();
					return FALSE;
				default:
					WCHAR sz[10]={0,};
					m_strColumnName += L" type error(vt = ";
					m_strColumnName += _itow_s(vFieldValue.vt, sz, 10);
					m_strColumnName += L" ) ";
					m_bCanGetParamGetFiled = false;
					return FALSE;
				}
			} catch (_com_error &e) {
				dump_com_error(e);
				return FALSE;
			}
			return TRUE;
		}


		/**
		\remarks	문자형 필드값을 읽는다.
		\par		읽은 값이 null이거나 버퍼가 작다면 실패를 리턴한다.
		\param		읽은 문자을 담을 버퍼의 크기
		\return		성공(TRUE) 실패(FLASE)
		*/
		/// <summary>
		/// 
		/// </summary
		bool GetFieldValue( IN wchar_t*, OUT wchar_t*, IN unsigned int );

		/**
		\remarks	binary 필드값을 읽는다.
		\par		읽은 값이 null이거나 버퍼가 작다면 실패를 리턴한다.
		\param		읽은 binary을 담을 버퍼의 크기, 읽은 binary 크기
		\return		성공(TRUE) 실패(FLASE)
		*/
		/// <summary>
		/// 
		/// </summary
		bool GetFieldValue( IN wchar_t*, OUT BYTE*, IN int, OUT int& );
	

		/**
		\remarks	정수/실수/날짜시간 타입의 파라메터 생성
		\par		null값의 파라메터 생성은 CreateNullParameter을 사용
		*/
		/// <summary>
		/// 
		/// </summary
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
		\remarks	정수/실수/날짜시간 타입의 null값 파라메터 생성
		*/
		/// <summary>
		/// 
		/// </summary
		void CreateNullParameter(IN wchar_t*, IN enum DataTypeEnum, IN enum ParameterDirectionEnum);

		/**
		\remarks	문자열 타입 파라메터 생성, 길이 변수는 최소 0보다 커야 한다. null값 생성은 wchar_t*에 NULL값을 넘긴다.
		*/
		/// <summary>
		/// 
		/// </summary
		void CreateParameter(IN wchar_t*,IN enum DataTypeEnum, IN enum ParameterDirectionEnum,
								IN wchar_t*, IN int);
		/**
		\remarks	binary 타입 파라메터 생성, 길이 변수는 최소 0보다 커야 한다. null값 생성은 BYTE*에 NULL값을 넘긴다.
		*/
		/// <summary>
		/// 
		/// </summary
		void CreateParameter(IN wchar_t*,IN enum DataTypeEnum, IN enum ParameterDirectionEnum,
								IN BYTE*, IN int);


		/**
		\remarks	정수/실수/날짜시간 타입의 파라메터값 변경
		\par		null값의 파라메터 변경은 UpdateNullParameter을 사용
		*/
		/// <summary>
		/// 
		/// </summary
		template<typename T>
			void UpdateParameter(IN wchar_t* pszName, IN T rValue)
		{
			if( !IsSuccess() ) { 
				return;
			}

			m_strCommand = L"Updatesarameter(T)";
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
		\remarks	정수/실수/날짜시간 타입의 파라메터 값을 null로 변경
		*/
		/// <summary>
		/// 
		/// </summary
		void UpdateNullParameter(IN wchar_t*);

		/**
		\remarks	문자열 타입 파라메터 변경, 길이 변수는 최소 0보다 커야 한다. null값 변경 TCHAR*에 NULL값을 넘긴다.
		*/
		/// <summary>
		/// 
		/// </summary
		void UpdateParameter(IN wchar_t*, IN wchar_t*, IN int);

		/**
		\remarks	binary 타입 파라메터 변경, 길이 변수는 최소 0보다 커야 한다. null값 변경 BYTE*에 NULL값을 넘긴다.
		*/
		/// <summary>
		/// 
		/// </summary
		void UpdateParameter(IN wchar_t*, IN BYTE*, IN int);

		/**
		\remarks	정수/실수/날짜시간 타입의 파라메터 값 읽기
		*/
		/// <summary>
		/// 
		/// </summary
		template<typename T>
			bool GetParameter(wchar_t* pszName, OUT T& Value)
		{
			if( !IsSuccess() ) { 
				return false;
			}

			m_bCanGetParamGetFiled = false;

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
					m_strParameterName += _itow_s( vFieldValue.vt, sz, 10 );
					m_strParameterName += L" ) ";
					m_bCanGetParamGetFiled = false;
					return false;
				}
			} 
			catch (_com_error &e)	
			{
				dump_com_error(e);
				return false;
			}

			m_bCanGetParamGetFiled = true;

			return m_bCanGetParamGetFiled;
		}

		/**
		\remarks	문자형 파라메터값을 읽는다.
		\par		읽은 값이 null이거나 버퍼가 작다면 실패를 리턴한다.
		\param		읽은 문자을 담을 버퍼의 크기
		\return		성공(TRUE) 실패(FLASE)
		*/
		/// <summary>
		/// 
		/// </summary
		bool GetParameter(IN wchar_t*, OUT wchar_t*, IN unsigned int);

		/**
		\remarks	바이너리형 파라메터값을 읽는다.
		\par		읽은 값이 null이거나 버퍼가 작다면 실패를 리턴한다.
		\param		읽은 문자을 담을 버퍼의 크기, 읽은 버퍼의 크기
		\return		성공(TRUE) 실패(FLASE)
		*/
		/// <summary>
		/// 
		/// </summary
		bool GetParameter(IN wchar_t*, OUT BYTE*, IN int, OUT int&);
	
	
	protected:
		_ConnectionPtr m_pConnection;      
		_RecordsetPtr m_pRecordset;
		_CommandPtr m_pCommand;

		bool m_bAutoCommit;
		DBConfig m_Config;

		std::wstring m_strQuery;

		bool m_bCanGetParamGetFiled;
		bool m_bCanCommitTransaction;
		
		std::wstring m_strCommand;
		std::wstring m_strColumnName;
		std::wstring m_strParameterName;

		AdoDB(const AdoDB&);
		AdoDB& operator= (const AdoDB&);
	};
}
