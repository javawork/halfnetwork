#pragma once
#include "ado.h"
#include "Lock.h"

/** 
\brief		Ŀ�ؼ�Ǯ ADO��ü�� �����Ͽ� stack�� �����Ѵ�.
\author		�迵��
*/

namespace asyncadodblib
{
	class DBManager
	{
		enum{MAX_ARRAY_SIZE=20};
	public:
		explicit DBManager( DBConfig& dboconfig )
		{
			m_bSuccessConnection = true;
			int MaxConnectionPoolCount = dboconfig.GetMaxConnectionPool();

			_ASSERTE( MaxConnectionPoolCount <= MAX_ARRAY_SIZE );

			for( int i = 0; i < MaxConnectionPoolCount; ++i )
			{
				m_pAdoStack[i] = new AdoDB(dboconfig);

				if( m_pAdoStack[i]->Open() == false )
				{
					m_bSuccessConnection = false;
					break;
				}
			}

			m_nTopPos = m_nMaxAdo = MaxConnectionPoolCount - 1;
		}

		// ���� ���� ����
		bool IsSuccessConnection() { return m_bSuccessConnection; }
	
		void PutDB( AdoDB* pAdo )
		{
			ScopedLock lock(m_Lock);
			
			_ASSERTE( m_nTopPos < m_nMaxAdo );

			m_pAdoStack[ ++m_nTopPos ] = pAdo;
			return;
		}

		AdoDB* GetDB()
		{
			ScopedLock lock(m_Lock);

			_ASSERTE( m_nTopPos >= 0 );
			
			return m_pAdoStack[ m_nTopPos-- ];
		}

	private:
		int m_nTopPos;
		int m_nMaxAdo;
		bool m_bSuccessConnection;	 // ���� ���� ����
	
		AdoDB* m_pAdoStack[MAX_ARRAY_SIZE];
		CSSpinLockWin32 m_Lock;
	};

	/** 
	\brief		��ü ������ Ŀ�ؼ�Ǯ�κ��� ADO��ü�� ���� �� �Ҹ�� ADO��ü�� Ŀ�ؼ�Ǯ�� �����ش�.
	\par		�ΰ���� ����� Ʈ�����
	\author		�迵��
	*/
	class CScopedAdo
	{
	public:
		explicit CScopedAdo( AdoDB* &pAdo, DBManager* pAdoManager, bool bAutoCommit = false )
			:m_pAdoManager(pAdoManager)
		{
			m_pAdo = pAdoManager->GetDB();
			pAdo = m_pAdo;
			pAdo->SetAutoCommit( bAutoCommit );

			if( bAutoCommit ) { 
				pAdo->BeginTransaction(); 
			}
		}

		~CScopedAdo()
		{
			if( m_pAdo->GetAutoCommit() )
			{
				if( m_pAdo->GetSuccess() )
				{
					m_pAdo->CommitTransaction();
				}
				else
				{
					m_pAdo->RollbackTransaction();
				}
			}

			m_pAdo->Init();
			m_pAdo->Release();
			m_pAdoManager->PutDB( m_pAdo );
		}

	private:
		DBManager* m_pAdoManager;
		AdoDB* m_pAdo;
	};
}
