#pragma once

#include <string>

namespace asyncadodblib
{
	class DBConfig
	{
	public:
		DBConfig()
			:m_nConnectionTimeout(0),
			m_nCommandTimeout(0),
			m_bRetryConnection(false)
		{
		}

		void SetInitCatalog( const wchar_t* pszString ) { m_strInitialCatalog = pszString; }
		
		void SetUserID( const wchar_t* pszUserID ) { m_strUserID = pszUserID;}

		void SetPassword(wchar_t* pszPassword)	{ m_strPassword = pszPassword; }

		void SetIP( wchar_t* pszString )
		{ 
			m_strDataSource = L";Data Source="; 
			m_strDataSource += pszString; 
			SetProvider();
		}

		void SetDSN( wchar_t* pszString ) 
		{ 
			m_strDSN = L";DSN="; 
			m_strDSN += pszString;
		}

		void SetCommandTimeout( int nCommendTimeout ) { m_nCommandTimeout = nCommendTimeout; }

		void SetConnectionTimeout( int nConnectionTimeout ) { m_nConnectionTimeout = nConnectionTimeout; }

		void SetRetryConnection( bool bRetryConnection ) { m_bRetryConnection = bRetryConnection; }

		void SetMaxConnectionPool( int nMaxConnectionPool ) { m_nMaxConnectionPool = nMaxConnectionPool; }

		std::wstring GetConnectionString()
		{
			if( m_strDataSource.empty() )
			{
				m_strConnectingString = m_strDSN;
			}
			else
			{
				m_strConnectingString = m_strDataSource;
			}

			return m_strConnectingString;
		}

		std::wstring& GetUserID()		{return m_strUserID; }
		
		std::wstring& GetPassword()		{return m_strPassword; }
		
		std::wstring& GetInitCatalog()	{return m_strInitialCatalog; }
		
		std::wstring& GetProvider()		{return m_strProvider; }
		
		std::wstring& GetDSN()			{ return m_strDSN; }
		
		int GetConnectionTimeout(){return m_nConnectionTimeout;}
		
		int GetCommandTimeout(){return m_nCommandTimeout;};
		
		bool GetRetryConnection(){return m_bRetryConnection;};
		
		int GetMaxConnectionPool(){return m_nMaxConnectionPool;};

	private:
		std::wstring m_strConnectingString;
		std::wstring m_strInitialCatalog;
		std::wstring m_strDataSource;
		std::wstring m_strUserID;
		std::wstring m_strPassword;
		std::wstring m_strProvider;
		std::wstring m_strDSN;
		
		int m_nConnectionTimeout;
		int m_nCommandTimeout;
		bool m_bRetryConnection;

		int m_nMaxConnectionPool;

		void SetProvider( wchar_t* pString = L"SQLOLEDB.1" )
		{
			m_strProvider = pString;
		}	
	};
}