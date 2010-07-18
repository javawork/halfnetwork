#pragma once
#include "ado.h"
#include "Lock.h"

/** 
\brief		Ŀ�ؼ�Ǯ ADO��ü�� �����Ͽ� stack�� �����Ѵ�.
\author		�迵��
*/
class CAdoManager
{
	enum{MAX_ARRAY_SIZE=20};
public:
	explicit CAdoManager(SAdoConfig& adoconfig)
	{
		m_bSuccessConnection = true;
		int MaxConnectionPoolCount = adoconfig.GetMaxConnectionPool();

		_ASSERTE(MaxConnectionPoolCount <= MAX_ARRAY_SIZE);
		for(int i = 0; i < MaxConnectionPoolCount; ++i)
		{
			m_pAdoStack[i] = new CAdo(adoconfig);
			if( FALSE == m_pAdoStack[i]->Open() )
			{
				m_bSuccessConnection = false;
				break;
			}
		}

		m_nTopPos = m_nMaxAdo = MaxConnectionPoolCount - 1;
	}

	// ���� ���� ����
	bool IsSuccessConnection() { return m_bSuccessConnection; 	}

	void PutAdo(CAdo* pAdo)
	{
		ScopedLock lock(m_Lock);
		_ASSERTE(m_nTopPos < m_nMaxAdo);
		m_pAdoStack[++m_nTopPos] = pAdo;
		return;
	}

	CAdo* GetAdo()
	{
		ScopedLock lock(m_Lock);
		_ASSERTE(m_nTopPos >= 0);
		return m_pAdoStack[m_nTopPos--];
	}
private:
	int m_nTopPos;
	int m_nMaxAdo;
	bool m_bSuccessConnection;	 // ���� ���� ����
	
	CAdo* m_pAdoStack[MAX_ARRAY_SIZE];
	CriticalSection m_Lock;
};

/** 
\brief		��ü ������ Ŀ�ؼ�Ǯ�κ��� ADO��ü�� ���� �� �Ҹ�� ADO��ü�� Ŀ�ؼ�Ǯ�� �����ش�.
\par		�ΰ���� ����� Ʈ�����
\author		�迵��
*/
class CScopedAdo
{
public:
	explicit CScopedAdo(CAdo* &pAdo, CAdoManager* pAdoManager, bool bAutoCommit = false)
		:m_pAdoManager(pAdoManager)
	{
		m_pAdo = pAdoManager->GetAdo();
		pAdo = m_pAdo;
		pAdo->SetAutoCommit(bAutoCommit);

		if(true == bAutoCommit)
			pAdo->BeginTransaction();
	}

	~CScopedAdo()
	{
		if(true == m_pAdo->GetAutoCommit())
		{
			if(m_pAdo->GetSuccess())
				m_pAdo->CommitTransaction();
			else
				m_pAdo->RollbackTransaction();
		}

		m_pAdo->Init();
		m_pAdo->Release();
		m_pAdoManager->PutAdo(m_pAdo);
	}

private:
	CAdoManager* m_pAdoManager;
	CAdo* m_pAdo;
};
