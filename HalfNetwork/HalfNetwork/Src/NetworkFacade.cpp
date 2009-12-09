#include <ace/Singleton.h>
#include <string>
#include "HalfNetworkType.h"
#include "HalfNetworkDefine.h"

#include "NetworkFacade.h"
#include "MessageQueueRepository.h"
#include "AbstractAcceptor.h"
#include "AbstractConnector.h"
#include "AbstractEventPool.h"
#include "AbstractServiceAccessor.h"
#include "MemoryObject.h"

namespace HalfNetwork
{
	/////////////////////////////////////////////////////
	// NetworkFacade
	/////////////////////////////////////////////////////
	NetworkFacade::NetworkFacade() : 
		_factory(NULL), 
		_connector(NULL), 
		_eventPool(NULL), 
		_serviceAccessor(NULL), 
		_queueRepository(NULL), 
		_blockPool(NULL),
		_suspend(FALSE)
	{
		SetupDefaultConfig();
	}
	
	NetworkFacade::~NetworkFacade()
	{
	}

	bool NetworkFacade::Create(AbstractFactory* const factory)
	{
		_factory = factory;
		_connector = _factory->CreateConnector();
		_eventPool = _factory->CreateEventPool();
		_serviceAccessor = _factory->CreateServiceAccessor(SystemConfigInst->Send_Mode);
		_queueRepository = new MessageQueueRepository();
		_blockPool = new FlexibleSizePoolT<MessageBlockPool, ACE_Message_Block>();
		return true;
	}

	void NetworkFacade::Destroy()
	{
		SAFE_DELETE(_factory);
		SAFE_DELETE(_connector);
		SAFE_DELETE(_eventPool);
		SAFE_DELETE(_serviceAccessor);
		SAFE_DELETE(_queueRepository);
		SAFE_DELETE(_blockPool);
	}

	bool NetworkFacade::Open(SystemConfig* const config)
	{
		_suspend = FALSE;
		if (NULL != config)
			*SystemConfigInst = *config;

		if (false == _connector->Open())
			return false;

		if (false == _queueRepository->Open())
			return false;

		if (false == _eventPool->Open(SystemConfigInst->Worker_Thread_Count))
			return false;

		SystemConfigInst->Worker_Thread_Count = _eventPool->GetPoolSize();

		if (false == StartListen())
			return false;

		return true;
	}

	void NetworkFacade::Close()
	{
		_queueRepository->SuspendPush();
		ClearMessageQueue();
		_suspend = TRUE;
		CloseService();
		_eventPool->Close();
		_queueRepository->Close();
		ClearAcceptor();
		SetupDefaultConfig();
	}

	bool NetworkFacade::AddAcceptor(const uint16 port, const uint8 queueId)
	{
		return AddAcceptor(NULL, port, queueId, 0, 0);
	}

	bool NetworkFacade::AddAcceptor(const ACE_TCHAR* acceptIp, const uint16 port, const uint8 queueId)
	{
		return AddAcceptor(acceptIp, port, queueId, 0, 0);
	}

	bool NetworkFacade::AddAcceptor
	(
		const ACE_TCHAR* acceptIp, 
		const uint16 port, 
		const uint8 queueId, 
		const uint32 receiveBufferSize
	)
	{
		return AddAcceptor(acceptIp, port, queueId, receiveBufferSize, 0);
	}


	bool NetworkFacade::AddAcceptor
	( 
		const ACE_TCHAR* acceptIp, 
		const uint16 port, 
		const uint8 queueId, 
		const uint32 receiveBufferSize, 
		const uint32 initialAcceptCount 
	)
	{
		if (true == _queueRepository->ExistQueueID(queueId))
			return false;

		if (true == CheckDuplicatedServicePort(port))
			return false;

		if (false == _queueRepository->CreateQueue(queueId))
			return false;

		AbstractAcceptor* acceptor = _factory->CreateAcceptor(port, queueId);
		if (NULL != acceptIp && ACE_OS::strlen(acceptIp) > 0)
			acceptor->SetAcceptIP(acceptIp);
		if (0 != receiveBufferSize)
			acceptor->ReceiveBufferSize(receiveBufferSize);
		if (0 != initialAcceptCount)
			acceptor->InitialAcceptCount(initialAcceptCount);
		_acceptorList.insert_tail(acceptor);
		return true;
	}

	bool NetworkFacade::Connect(const ACE_TCHAR* ip, const uint16 port, const uint8 queueId)
	{
		_queueRepository->CreateQueue(queueId);
		return _connector->Connect(ip, port, queueId);
	}

	bool NetworkFacade::Connect(const ACE_TCHAR* ip, const uint16 port, const uint8 queueId, const uint32 receiveBufferSize)
	{
		_queueRepository->CreateQueue(queueId);
		return _connector->Connect(ip, port, queueId, receiveBufferSize);
	}

	bool NetworkFacade::PopMessage(const uint8 queueId, ACE_Message_Block** block, const int timeout)
	{
		if (TRUE == _suspend.value())
			return false;

		return _queueRepository->Pop(queueId, block, timeout);
	}

	bool NetworkFacade::PopAllMessage(const uint8 queueId, ACE_Message_Block** block, const int timeout)
	{
		if (TRUE == _suspend.value())
			return false;
		return _queueRepository->PopAll(queueId, block, timeout);
	}

	bool NetworkFacade::PopAllMessage(ACE_Message_Block** block, const int timeout)
	{
		if (TRUE == _suspend.value())
			return false;
		return _queueRepository->PopAll(block, timeout);
	}

	void NetworkFacade::Pulse()
	{
		_queueRepository->PulseAll();
	}

	void NetworkFacade::Pulse(const uint8 queueId)
	{
		_queueRepository->Pulse(queueId);
	}

	bool NetworkFacade::PushCustomMessage(const uint8 queId, ACE_Message_Block* block)
	{
		return PushMessage(queId, eMH_Custom, Invalid_ID, block);
	}

	bool NetworkFacade::PushMessage(  
							const uint8 queId, 
							const char command, 
							const uint32 serial, 
							ACE_Message_Block* block)
	{
		if (TRUE == _suspend.value())
			return false;

		if (false == _queueRepository->Push(queId, (EMessageHeader)command, serial, block))
		{
			//ACE_DEBUG ((LM_ERROR, ACE_TEXT("_queue_repository Push fail")));
			return false;
		}
		return true;
	}

	bool NetworkFacade::SendRequest(const uint32 streamId, ACE_Message_Block* block, bool copy_block) 
	{
		if (TRUE == _suspend.value())
		{
			//OutputDebugString(_T("SendRequest #1\n"));
			return false;
		}

		ACE_Message_Block* send_block = NULL;
		if (copy_block)
		{
			send_block = AllocateBlock(block->length());
			send_block->copy(block->rd_ptr(), block->length());
		}
		else
		{
			send_block = block;
		}

		return _serviceAccessor->SendRequest(streamId, send_block);
	}

	bool NetworkFacade::SendRequest(const uint32 streamId, const char* buffer, const uint32 length) 
	{
		ACE_Message_Block* send_block = AllocateBlock(length);
		ACE_ASSERT(send_block);
		send_block->copy(buffer, length);
		return SendRequest(streamId, send_block, false);
	}

	bool NetworkFacade::SendReserve(
		const uint32 streamId, const char* buffer, const uint32 length, const uint32 delay)
	{
		ACE_Message_Block* send_block = AllocateBlock(length);
		ACE_ASSERT(send_block);
		send_block->copy(buffer, length);
		return _serviceAccessor->SendReserve(streamId, send_block, delay);
	}

	void NetworkFacade::SetSendMode(const ESendMode mode)
	{
		SystemConfigInst->Send_Mode = mode;
	}

	void NetworkFacade::SetWorkerThreadCount(const uint8 count)
	{
		SystemConfigInst->Worker_Thread_Count = count;
	}

	void NetworkFacade::SetReceiveBufferLen(const uint32 length)
	{
		SystemConfigInst->Receive_Buffer_Len = length;
	}

	void NetworkFacade::SetIntervalSendTerm(const uint32 ms)
	{
		SystemConfigInst->Interval_Send_Term = ms;
	}

	void NetworkFacade::GetInformation(NetworkFacadeInfo& info)
	{
		info.worker_thread_count = SystemConfigInst->Worker_Thread_Count;
		info.send_mode = SystemConfigInst->Send_Mode;
		ACE_OS::strcpy(info.event_dispatch_model, _factory->GetFactoryName());
		info.receive_buffer_len = SystemConfigInst->Receive_Buffer_Len;
		info.interval_send_term = SystemConfigInst->Interval_Send_Term;
		info.service_count = _serviceAccessor->ServiceCount();

		AcceptorIter iter(_acceptorList);
		while(false == iter.done())
		{
			AbstractAcceptor* acceptor = iter.next();
			AcceptorInfo acceptor_info;
			acceptor->GetInfo(acceptor_info);
			info.acceptor_que.enqueue_tail(acceptor_info);
			++iter;
		}
	}

	void NetworkFacade::Dump()
	{
		// worker thread count
		HALF_DEBUG("[%t] Service Count(%d).\n", _serviceAccessor->ServiceCount());
		_queueRepository->Dump();
		_blockPool->Dump();
	}

	ACE_Message_Block* NetworkFacade::AllocateBlock(const size_t bufferSize)
	{
		return _blockPool->Allocate(bufferSize);
	}

	void NetworkFacade::PrepareMessageBlock( const size_t bufferSize, const uint32 count )
	{
		ACE_Array_Base<ACE_Message_Block*> blockArray(count);
		for(uint32 i=0;i<count;++i)
		{
			ACE_Message_Block* block = AllocateBlock(bufferSize);
			blockArray[i] = block;
		}
		for(uint32 i=0;i<blockArray.size(); ++i)
		{
			ACE_Message_Block* block = blockArray[i];
			block->release();
		}
	}

	void NetworkFacade::PrepareMemoryBlock( const size_t bufferSize, const uint32 count )
	{
		ACE_Array_Base<void*> pointerArray(count);
		for(uint32 i=0;i<count;++i)
		{
			void* p = MemoryPoolInstance->Allocate(bufferSize);
			pointerArray[i] = p;
		}
		for(uint32 i=0;i<pointerArray.size(); ++i)
		{
			void* p = pointerArray[i];
			MemoryPoolInstance->Deallocate(p, bufferSize);
			p = NULL;
		}
	}

	bool NetworkFacade::SuspendAcceptor(const uint16 port)
	{
		AcceptorIter iter(_acceptorList);
		while(false == iter.done())
		{
			AbstractAcceptor* acceptor = iter.next();
			if ( port == acceptor->ServicePort())
			{
				acceptor->Suspend();
				return true;
			}
			
			++iter;
		}
		return false;
	}

	bool NetworkFacade::ResumeAcceptor(const uint16 port)
	{
		AcceptorIter iter(_acceptorList);
		while(false == iter.done())
		{
			AbstractAcceptor* acceptor = iter.next();
			if ( port == acceptor->ServicePort())
			{
				acceptor->Resume();
				return true;
			}

			++iter;
		}
		return false;
	}

	bool NetworkFacade::StartListen()
	{
		AcceptorIter iter(_acceptorList);
		while(false == iter.done())
		{
			AbstractAcceptor* acceptor = iter.next();
			if (false == acceptor->Open())
				return false;
			++iter;
		}
		return true;
	}

	void NetworkFacade::ClearAcceptor()
	{
		AcceptorIter iter(_acceptorList);
		while(false == iter.done())
		{
			AbstractAcceptor* acceptor = iter.next();
			acceptor->Close();
			SAFE_DELETE(acceptor);
			AcceptorIter remove_iter = iter;
			++iter;
			remove_iter.remove();
		}
	}

	bool NetworkFacade::CheckDuplicatedServicePort(const uint16 port)
	{
		AcceptorIter iter(_acceptorList);
		while(false == iter.done())
		{
			AbstractAcceptor* acceptor = iter.next();
			if (port == acceptor->ServicePort())
				return true;
			++iter;
		}
		return false;
	}

	void NetworkFacade::CloseService()
	{
		_serviceAccessor->CloseService();
	}

	void NetworkFacade::ClearMessageQueue()
	{
		ACE_Message_Block* commandBlock = NULL;
		bool result = NetworkInstance->PopAllMessage(&commandBlock, 100);
		if (false == result)
			return;

		commandBlock->release();
	}

	void NetworkFacade::SetupDefaultConfig()
	{
		SystemConfigInst->Receive_Buffer_Len = 1024*4;
		SystemConfigInst->Worker_Thread_Count = 0;
		SystemConfigInst->Interval_Send_Term = 20;
		SystemConfigInst->Send_Mode = eSM_Interval;
	}

	void NetworkFacade::CloseStream(const uint32 streamID)
	{
		_serviceAccessor->DisableService(streamID);
	}

	void NetworkFacade::CloseReceiveStream(const uint32 streamID)
	{
		_serviceAccessor->CloseReceiveStream(streamID);
	}

} // namespace HalfNetwork