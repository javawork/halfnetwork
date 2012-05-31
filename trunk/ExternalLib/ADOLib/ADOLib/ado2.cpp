#include "stdafx.h"
#include "ado - New.h"

namespace adodblib
{
	AdoDB::AdoDB(DBConfig& adoconfig)
		:m_strConnectingString(adoconfig.GetConnectionString()),
		m_strUserID(adoconfig.GetUserID()),
		m_strPassword(adoconfig.GetPassword()),
		m_strInitCatalog(adoconfig.GetInitCatalog()),
		m_strProvider(adoconfig.GetProvider()),
		m_strDSN(adoconfig.GetDSN()),
		m_nConnectionTimeout(adoconfig.GetConnectionTimeout()),
		m_nCommandTimeout(adoconfig.GetCommandTimeout()),
		m_bRetryConnection(adoconfig.GetRetryConnection()),
		m_bAutoCommit(false),

		m_pConnection(nullptr), 
		m_pCommand(nullptr),
		m_pRecordset(nullptr),

		m_IsSuccess(TRUE),
		m_strParameterName(),
		m_strColumnName(),
		m_strQuery(),
		m_strCommand()
	{
		if( FAILED(::CoInitialize(nullptr)) ) 
		{
			LOG(L"::CoInitialize Fail!!");
			return;
		}

		m_pConnection.CreateInstance(__uuidof(Connection));
		m_pCommand.CreateInstance(__uuidof(Command)); 
	}

	AdoDB::~AdoDB()
	{
		Close();
	}

	void AdoDB::Init()
	{
		m_bAutoCommit = false;
		m_IsSuccess = TRUE;
		m_strParameterName.clear();
		m_strColumnName.clear();
		m_strQuery.clear();
		m_strCommand.clear();
	}

	void AdoDB::dump_com_error(_com_error &e)
	{
		m_IsSuccess = FALSE;
		
		if( e.Error() == 0X80004005 ) { 
			Close();
		}

		LOG(L"Code = %08lX   Code meaning = %s", e.Error(), e.ErrorMessage());
		LOG(L"Source = %s", (LPCTSTR)e.Source());
		LOG(L"Desc = %s", (LPCTSTR)e.Description());

	}

	void AdoDB::dump_user_error()
	{
		m_IsSuccess = FALSE;

		if( !m_strQuery.empty() ) { 
			LOG( L"SQLQuery[%s]", m_strQuery.c_str() );
		}

		if( !m_strCommand.empty() ) { 
			LOG( L"Command[%s]", m_strCommand.c_str() );
		}

		if( !m_strColumnName.empty() ) { 
			LOG( L"Column[%s]", m_strColumnName.c_str() );
		}

		if( !m_strParameterName.empty() ) { 
			LOG( L"Paramter[%s]", m_strParameterName.c_str() );
		}
	}

	bool AdoDB::GetFieldCount( int& nValue )
	{
		m_strCommand = L"GetFieldCount()";
		
		try
		{
			nValue =  m_pRecordset->GetFields()->GetCount();;
		}
		catch(_com_error &e) 
		{
			dump_com_error(e);
			return false;	
		}
		
		return true;
	}

	bool AdoDB::RetryOpen()
	{
		if( m_pConnection->GetState() != adStateClosed ) { 
			return false;
		}

		return Open();
	}

	bool AdoDB::Execute(CommandTypeEnum CommandType /*= adCmdStoredProc*/, ExecuteOptionEnum OptionType /*= adOptionUnspecified*/)
	{
		if( !m_IsSuccess ) { 
			return false;
		}

		try	
		{
			if( m_pConnection->GetState() == adStateClosed && m_bRetryConnection )
			{	
				m_strCommand = L"RetryOpen()";  	//재연결 시도
				
				if( RetryOpen() == false ) 
				{
					return false;
				} 
				else 
				{ 
					m_IsSuccess = true;
				}
			}

			m_strCommand = L"Execute()";

			m_pCommand->CommandType = CommandType;
			m_pCommand->CommandText = m_strQuery.c_str();

			if( m_nConnectionTimeout != 0 ) {
				m_pCommand->CommandTimeout = m_nConnectionTimeout;
			}

			m_pRecordset = m_pCommand->Execute( NULL,NULL,OptionType ); 	
		} 
		catch(_com_error &e) 
		{
			dump_com_error(e);
			return false;	
		}
		
		return true;
	}

	void AdoDB::MoveNext()
	{
		m_strCommand = L"MoveNext()";
		
		try	
		{
			m_pRecordset->MoveNext();
		} 
		catch (_com_error &e)	
		{
			dump_com_error(e);
			return;
		}
		
		return;
	}

	bool AdoDB::GetEndOfFile()
	{
		m_strCommand = _T("GetEndOfFile()");
		bool bEndOfFile = true;

		try 
		{
			VARIANT_BOOL vbEnd = m_pRecordset->GetEndOfFile();
			
			if( vbEnd == 0 ) {
				bEndOfFile = false;
			}
		}
		catch (_com_error &e) 
		{
			dump_com_error(e);
		}
	
		return bEndOfFile;
	}

	bool AdoDB::NextRecordSet()
	{
		m_strCommand = L"NextRecordSet()";

		_variant_t variantRec;
		variantRec.intVal = 0;
		
		try	
		{
			m_pRecordset = m_pRecordset->NextRecordset((_variant_t*)&variantRec);
		} 
		catch (_com_error &e) 
		{
			dump_com_error(e);
			return false;
		}
		
		return true;
	}

	bool AdoDB::GetFieldValue( IN wchar_t* tszName, OUT wchar_t* pszValue, IN unsigned int nSize )
	{
		m_strCommand = L"GetFieldValue(string)";

		m_strColumnName = tszName;
		
		try	
		{
			_variant_t vFieldValue = m_pRecordset->GetCollect(tszName);


			if( vFieldValue.vt == VT_NULL || vFieldValue.vt == VT_EMPTY ) 
			{
				m_strColumnName += L" null value";
				return false;
			} 
			else if( vFieldValue.vt != VT_BSTR ) 
			{
				m_strColumnName += L" nonbstr type";
				return false;
			} 

			if( nSize < wcslen((wchar_t*)(_bstr_t(vFieldValue.bstrVal))) ) 
			{
				m_strColumnName += L" string size overflow";
				return false;
			}

			wcscpy_s( pszValue, nSize, (wchar_t*)static_cast<_bstr_t>(vFieldValue.bstrVal) );

		} catch (_com_error &e) 
		{
			dump_com_error(e);
			return false;
		}

		return true;
	}

	bool AdoDB::GetFieldValue( IN wchar_t* pszName, OUT BYTE* pbyBuffer, IN int inSize, OUT int& outSize )
	{
		m_strCommand = L"GetFieldValue(binary)";
		m_strColumnName = pszName;
		
		try
		{
			_variant_t vFieldValue = m_pRecordset->GetCollect(pszName);

			if( vFieldValue.vt == VT_NULL )
			{
				m_strColumnName += L" null value";
				return false;
			}
			else if( vFieldValue.vt != (VT_ARRAY|VT_UI1) ) 
			{
				m_strColumnName += L" nonbinary type";
				return false;
			}

			FieldPtr pField = m_pRecordset->Fields->GetItem(pszName);

			if( inSize < pField->ActualSize || inSize > 8060 )
			{
				m_strColumnName += L" binary size overflow";
				dump_user_error();
				return false;
			}

			outSize = static_cast<int>(pField->ActualSize);

			BYTE * pData = nullptr;
			SafeArrayAccessData( vFieldValue.parray, (void HUGEP* FAR*)&pData );
			CopyMemory( pbyBuffer, pData, static_cast<size_t>(pField->ActualSize) );
			SafeArrayUnaccessData( vFieldValue.parray );
		}
		catch (_com_error &e)
		{
			dump_com_error(e);
			return false;
		}

		return true;
	}

	void AdoDB::CreateNullParameter(IN wchar_t* pszName, IN enum DataTypeEnum Type, IN enum ParameterDirectionEnum Direction)
	{
		if( !m_IsSuccess ) { 
			return;
		}

		m_strCommand = L"CreateParameter(null)";
		m_strParameterName = pszName;

		try	
		{
			_ParameterPtr pParametor( m_pCommand->CreateParameter(pszName, Type, Direction, 0) );
			m_pCommand->Parameters->Append(pParametor);
			
			_variant_t vNull;
			vNull.ChangeType(VT_NULL);
			pParametor->Value = vNull;
		} 
		catch (_com_error &e) 
		{
			dump_com_error(e);
		}

		return;
	}

	void AdoDB::CreateParameter(IN wchar_t* pszName, IN enum DataTypeEnum Type, IN enum ParameterDirectionEnum Direction,
								  IN wchar_t* pValue, IN int nSize)
	{
		if( !m_IsSuccess ) { 
			return;
		}

		m_strCommand = L"CreateParameter(TCHAR)";
		m_strParameterName = pszName;

		_ASSERTE( nSize > 0 && "not allow 0 size!!" );

		try	
		{
			_ParameterPtr pParametor( m_pCommand->CreateParameter(pszName, Type, Direction, nSize) );
			m_pCommand->Parameters->Append(pParametor);

			if( pValue == nullptr )
			{
				_variant_t vValue;
				vValue.vt = VT_NULL;
				pParametor->Value = vValue;
			}
			else
			{
				_variant_t vValue(pValue);
				pParametor->Value = vValue;
			}
		} 
		catch (_com_error &e) 
		{
			dump_com_error(e);
		}

		return;
	}

	void AdoDB::CreateParameter(IN wchar_t* pszName,IN enum DataTypeEnum Type, IN enum ParameterDirectionEnum Direction,
								  IN BYTE* pValue, IN int nSize)
	{
		if( !m_IsSuccess ) { 
			return;
		}

		_ASSERTE( nSize > 0 && "not allow 0 size!!" );

		m_strCommand = L"CreateParameter(binary)";
		m_strParameterName = pszName;

		try	
		{
			_ParameterPtr pParametor( m_pCommand->CreateParameter( pszName, Type, Direction, nSize) );
			m_pCommand->Parameters->Append(pParametor);

			_variant_t vBinary;
			SAFEARRAY FAR *pArray = nullptr;
			SAFEARRAYBOUND rarrayBound[1];

			if(pValue == nullptr )		//명시적 null이거나 값이 null이라면
			{
				vBinary.vt = VT_NULL;
				pParametor->Value = vBinary;
			}
			else
			{
				vBinary.vt = VT_ARRAY|VT_UI1;
				rarrayBound[0].lLbound = 0;
				rarrayBound[0].cElements = nSize;
				pArray = SafeArrayCreate(VT_UI1, 1, rarrayBound);

				for (long n = 0; n < nSize; ++n )
				{
					SafeArrayPutElement(pArray, &n, &(pValue[n]) );
				}
				vBinary.parray = pArray;
				pParametor->AppendChunk(vBinary);
			}
		} 
		catch (_com_error &e) 
		{
			dump_com_error(e);
		}
		
		return;
	}

	void AdoDB::UpdateNullParameter(IN wchar_t* pszName)
	{
		if(!m_IsSuccess) return;

		m_strCommand = L"UpdateNullParameter(null)";
		m_strParameterName = pszName;
		
		try	
		{
			m_pCommand->Parameters->GetItem(pszName)->Value.ChangeType(VT_NULL);
		} 
		catch (_com_error &e) 
		{
			dump_com_error(e);
		}

		return;
	}

	void AdoDB::UpdateParameter(IN wchar_t* pszName, IN wchar_t* pValue, IN int nSize)
	{
		if( !m_IsSuccess ) {
			return;
		}

		_ASSERTE( nSize > 0 && "not allow 0 size!!" );

		m_strCommand = L"UpdateParameter(WCHAR)";
		m_strParameterName = pszName;
		
		try	
		{
			_variant_t vValue(pValue);

			if( pValue == nullptr )
			{	
				vValue.vt = VT_NULL;
			}
		
			m_pCommand->Parameters->GetItem(pszName)->Size = nSize;
			m_pCommand->Parameters->GetItem(pszName)->Value = vValue;
		} 
		catch (_com_error &e) 
		{
			dump_com_error(e);
		}

		return;
	}

	void AdoDB::UpdateParameter(IN wchar_t* pszName, IN BYTE* pValue, IN int nSize)
	{
		if( !m_IsSuccess ) { 
			return;
		}

		_ASSERTE( nSize > 0 && "not allow 0 size!!" );

		m_strCommand = L"UpdateParameter(binary)";
		m_strParameterName = pszName;
		
		try	
		{
			_ParameterPtr pParametor( m_pCommand->Parameters->GetItem(pszName) );
			pParametor->Size = nSize;

			_variant_t vBinary;
			SAFEARRAY FAR *pArray = nullptr;
			SAFEARRAYBOUND rarrayBound[1];

			if( pValue == nullptr )
			{
				vBinary.vt = VT_NULL;
				rarrayBound[0].lLbound = 0;
				rarrayBound[0].cElements = 0;
				pParametor->Value = vBinary;
			}
			else
			{
				vBinary.vt = VT_ARRAY|VT_UI1;
				rarrayBound[0].lLbound = 0;
				rarrayBound[0].cElements = nSize;
				pArray = SafeArrayCreate(VT_UI1, 1, rarrayBound);

				for (long n = 0; n < nSize; ++n )
				{
					SafeArrayPutElement(pArray, &n, &(pValue[n]) );
				}

				vBinary.parray = pArray;
				pParametor->AppendChunk(vBinary);
			}
		} 
		catch (_com_error &e) 
		{
			dump_com_error(e);
		}
		
		return;
	}

	bool AdoDB::GetParameter( IN wchar_t* pszName, OUT wchar_t* pValue, IN unsigned int nSize )
	{
		if( !m_IsSuccess ) { 
			return false;
		}

		m_IsSuccess = false;

		m_strCommand = L"GetParameter(wchar_t*)";
		m_strParameterName = pszName;

		try	
		{
			_variant_t vFieldValue = m_pCommand->Parameters->GetItem(pszName)->Value;

			if( vFieldValue.vt == VT_NULL || vFieldValue.vt == VT_EMPTY ) 
			{
				m_strParameterName += L" null value";
				return false;
			} 
			else if( vFieldValue.vt != VT_BSTR ) 
			{
				m_strParameterName += L" nonString Type";
				return false;
			} 
			else if( nSize < wcslen((wchar_t*)(_bstr_t(vFieldValue.bstrVal))) ) 
			{
				m_strParameterName += L" string size overflow";
				return false;
			}

			wcscpy_s( pValue, nSize, (wchar_t*)(_bstr_t)vFieldValue );
		} 
		catch ( _com_error &e )	
		{
			dump_com_error(e);
			return false;
		}
		
		m_IsSuccess = true;
		return m_IsSuccess;
	}

	bool AdoDB::GetParameter( IN wchar_t* pszName, OUT BYTE* pBuffer, IN int inSize, OUT int& outSize )
	{
		if( !m_IsSuccess ) { 
			return false;
		}

		m_IsSuccess = false;

		m_strCommand = L"GetParameter(binary)";
		m_strParameterName = pszName;
		
		try	
		{
			_variant_t vFieldValue = m_pCommand->Parameters->GetItem(pszName)->Value;

			if(VT_NULL == vFieldValue.vt) 
			{
				m_strParameterName += L" null value";
				return false;
			} 
			else if( (VT_ARRAY|VT_UI1) != vFieldValue.vt ) 
			{
				m_strParameterName += L" nonbinary type";
				return false;
			}

			int ElementSize = vFieldValue.parray->rgsabound[0].cElements;

			if(ElementSize > inSize || inSize > 8060)
			{
				m_strParameterName += L" size overflow";
				return false;
			}

			BYTE * pData = nullptr;
			SafeArrayAccessData( vFieldValue.parray, (void HUGEP* FAR*)&pData );
			CopyMemory( pBuffer, pData, ElementSize );
			SafeArrayUnaccessData( vFieldValue.parray );
			outSize = vFieldValue.parray->rgsabound[0].cElements;
		} 
		catch (_com_error &e)	
		{
			dump_com_error(e);
			return false;
		}
		
		m_IsSuccess = true;
		return m_IsSuccess;
	}


	void AdoDB::Close()
	{
		if( m_pConnection == nullptr ) { 
			return;
		}

		try
		{
			if( m_pConnection->GetState() != adStateClosed ) { 
				m_pConnection->Close();
			}
		}
		catch(...)
		{

		}
	}

	bool AdoDB::Open(CursorLocationEnum CursorLocation)
	{
		m_strCommand = L"Open()";

		try
		{
			if( m_nConnectionTimeout != 0 ) { 
				m_pConnection->PutConnectionTimeout( m_nConnectionTimeout );
			}

			m_pConnection->CursorLocation = CursorLocation;

			if( !m_strProvider.empty() ) { //ip접속일 경우 Provider 사용
				m_pConnection->put_Provider((_bstr_t)m_strProvider.c_str());
			}

			m_pConnection->Open( (_bstr_t)m_strConnectingString.c_str(), (_bstr_t)m_strUserID.c_str(),
				(_bstr_t)m_strPassword.c_str(), NULL );

			if( m_pConnection->GetState() == adStateOpen ) { 
				m_pConnection->DefaultDatabase = m_strInitCatalog.c_str();
			}

			m_pCommand->ActiveConnection = m_pConnection;
			return true;
		}
		catch(_com_error &e)
		{
			dump_com_error(e);
			return false;
		}
	}

	void AdoDB::Release()
	{
		m_pCommand.Release();
		m_pCommand.CreateInstance(__uuidof(Command));
		
		if( m_pConnection->GetState() != adStateClosed ) { 
			m_pCommand->ActiveConnection = m_pConnection;
		}
	}

	void AdoDB::LOG(wchar_t* format, ...)
	{
		wchar_t szBuffer[1024] = { 0, };
		
		va_list ap;
		va_start(ap, format);
		vswprintf_s( szBuffer, 1024, format, ap );
		va_end(ap);
		
		wprintf( L"%s\n", szBuffer );
	}
}