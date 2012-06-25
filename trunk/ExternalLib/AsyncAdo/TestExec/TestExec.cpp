// TestExec.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <iostream>
#include "AdoManager.h"



void SetDBSetting(asyncadodblib::DBConfig& config )
{
	config.Setting( L"gunz2db\\gunz2_db", 
						L"dev", 
						L"dev", 
						L"G2_GAMEDB", 
						3, 
						true, 
						3 
					);
}

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "");
	
	asyncadodblib::DBConfig config;
	SetDBSetting( config );

	asyncadodblib::DBManager* pDBmanager = new asyncadodblib::DBManager( config );
		
	// SQL Query - insert: auto commit
	{
		asyncadodblib::AdoDB* pAdo = nullptr;
		asyncadodblib::CScopedAdo scopedado( pAdo, pDBmanager, true );

		pAdo->SetQuery( L"Insert Into Test_Temp Values( 'jacking8', 1111 )" );
		pAdo->Execute(adCmdText);
		
		if( !pAdo->IsSuccess() ) 
		{
			std::wcout << L"쿼리문 실패" << std::endl;
			return 0;
		}
		else
		{
			std::wcout << L"쿼리문 성공" << std::endl;
		}
	}

	// SQL Query - insert
	{
		asyncadodblib::AdoDB* pAdo = nullptr;
		asyncadodblib::CScopedAdo scopedado( pAdo, pDBmanager, false );

		pAdo->SetQuery( L"Insert Into Test_Temp Values( 'jacking9', 1111 )" );
		pAdo->Execute(adCmdText);
		
		if( !pAdo->IsSuccess() ) 
		{
			std::wcout << L"쿼리문 실패" << std::endl;
			return 0;
		}
		else
		{
			std::wcout << L"쿼리문 성공" << std::endl;
		}

		pAdo->SetCommitTransaction();
	}

	// SQL Query - select
	{
		asyncadodblib::AdoDB* pAdo = nullptr;
		asyncadodblib::CScopedAdo scopedado( pAdo, pDBmanager, true );

		pAdo->SetQuery(_T("SELECT ID, Code FROM Test_Temp WHERE ID='jacking3'"));
		pAdo->Execute(adCmdText);

		if( !pAdo->IsSuccess() )
		{
			std::wcout << L"select 쿼리문 실패" << std::endl;
			return 0;
		}

		WCHAR szID[16] = {0,};
		int nCode = 0;

		if( !pAdo->GetEndOfFile() )
		{
			pAdo->GetFieldValue(_T("Code"), nCode);
			pAdo->GetFieldValue(_T("ID"), szID, 16);
		}
		else
		{
			std::wcout << L"jacking3는 없습니다" << std::endl;
			return 0;
		}

		std::wcout << L"ID : " << szID << std::endl;
		std::wcout << L"Code : " << nCode << std::endl;
	}

	delete pDBmanager;
	
	getchar();

	return 0;
}