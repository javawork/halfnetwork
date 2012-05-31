// ADOLib.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include <iostream>
#include "AdoManager.h"
/*

void SetDBSetting(SAdoConfig& adoconfig )
{
	adoconfig.SetIP(_T("localhost\\TEST"));   //����ּ�
	adoconfig.SetUserID(_T("sa"));			//���̵�
	adoconfig.SetPassword(_T("dev"));			//�н�����
	adoconfig.SetInitCatalog(_T("TEST"));     //����
	adoconfig.SetConnectionTimeout(3);
	adoconfig.SetRetryConnection(true);
	adoconfig.SetMaxConnectionPool(3);
}

// �Ϲ� ������ - insert - 2
int _tmain(int argc, _TCHAR* argv[])
{  
	setlocale(LC_ALL, "");
	SAdoConfig adoconfig;
	SetDBSetting( adoconfig );

	CAdoManager* pAdomanager = new CAdoManager(adoconfig);
	CAdo* pAdo = NULL;
	{
		CScopedAdo scopedado(pAdo, pAdomanager, true);

		pAdo->SetQuery( _T("Insert Into Users Values( 'jacking3', '1111' )") );
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
	delete pAdomanager;
	
	getchar();
	return 0;
}

// �Ϲ� ������ - insert - 1
//int _tmain(int argc, _TCHAR* argv[])
//{  
//	setlocale(LC_ALL, "");
//	SAdoConfig adoconfig;
//	SetDBSetting( adoconfig );
//
//	CAdoManager* pAdomanager = new CAdoManager(adoconfig);
//	CAdo* pAdo = NULL;
//	{
//		CScopedAdo scopedado(pAdo, pAdomanager, false);
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
//	delete pAdomanager;
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
//	CAdoManager* pAdomanager = new CAdoManager(adoconfig);
//	CAdo* pAdo = NULL;
//	{
//		CScopedAdo scopedado(pAdo, pAdomanager, false);
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
//	delete pAdomanager;
//	
//	getchar();
//	return 0;
//}
*/