// TestExec.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
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
	asyncadodblib::AdoDB* pAdo = nullptr;
	{
		asyncadodblib::CScopedAdo scopedado( pAdo, pDBmanager, true );

		pAdo->SetQuery( L"Insert Into Test_Temp Values( 'jacking5', 1111 )" );
		pAdo->Execute(adCmdText);
		
		if( !pAdo->IsSuccess() ) 
		{
			std::wcout << L"������ ����" << std::endl;
			return 0;
		}
		else
		{
			std::wcout << L"������ ����" << std::endl;
		}

		pAdo->SetCommit(true);
	}

	delete pDBmanager;
	
	getchar();

	return 0;
}

/*
// �Ϲ� ������ - insert - 1
//int _tmain(int argc, _TCHAR* argv[])
//{  
//	setlocale(LC_ALL, "");
//	SAdoConfig adoconfig;
//	SetDBSetting( adoconfig );
//
//	CAdoManager* pDBmanager = new CAdoManager(adoconfig);
//	CAdo* pAdo = NULL;
//	{
//		CScopedAdo scopedado(pAdo, pDBmanager, false);
//
//		pAdo->SetQuery( _T("Insert Into Users Values( 'jacking2', '1111' )") );
//		pAdo->Execute(adCmdText);
//		if( !pAdo->IsSuccess() ) 
//		{
//			std::wcout << L"������ ����" << std::endl;
//			return 0;
//		}
//		else
//		{
//			std::wcout << L"������ ����" << std::endl;
//		}
//	}
//	delete pDBmanager;
//	
//	getchar();
//	return 0;
//}

// �Ϲ� ������ - select
//int _tmain(int argc, _TCHAR* argv[])
//{  
//	setlocale(LC_ALL, "");
//	SAdoConfig adoconfig;
//	SetDBSetting( adoconfig );
//
//	CAdoManager* pDBmanager = new CAdoManager(adoconfig);
//	CAdo* pAdo = NULL;
//	{
//		CScopedAdo scopedado(pAdo, pDBmanager, false);
//
//		pAdo->SetQuery(_T("SELECT UID, PWD FROM Users WHERE ID='jacking'"));
//		pAdo->Execute(adCmdText);
//		if(!pAdo->IsSuccess())
//		{
//			std::wcout << L"������ ����" << std::endl;
//			return 0;
//		}
//
//		int nUID = 0;
//		WCHAR szPWD[16];
//
//		if(!pAdo->GetEndOfFile() )
//		{
//			pAdo->GetFieldValue(_T("UID"), nUID);
//			pAdo->GetFieldValue(_T("PWD"), szPWD, 16);
//		}
//		else
//		{
//			std::wcout << L"jacking�� �����ϴ�" << std::endl;
//			return 0;
//		}
//
//		std::wcout << L"UID : " << nUID << std::endl;
//		std::wcout << L"PWD : " << szPWD << std::endl;
//	}
//	delete pDBmanager;
//	
//	getchar();
//	return 0;
//}
*/