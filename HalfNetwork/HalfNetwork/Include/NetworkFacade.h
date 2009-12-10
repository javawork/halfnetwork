#ifndef __networkfacade_h__
#define __networkfacade_h__

#pragma once

#include "AbstractFactory.h"
#include <ace/Containers.h>
#include <ace/Message_Block.h>
#include <ace/Task.h>
#include "PoolStrategy.h"
#include <ace/Singleton.h>
#include "HalfNetworkDefine.h"

class ACE_Handler;

namespace HalfNetwork
{
	typedef ACE_Unbounded_Queue<AcceptorInfo> AcceptorQueue;
	struct NetworkFacadeInfo 
	{
		uint8			worker_thread_count;
		ACE_TCHAR			event_dispatch_model[FACTORY_NAME_SIZE];
		AcceptorQueue	acceptor_que;
		ESendMode		send_mode;
		uint32			receive_buffer_len;
		uint32			interval_send_term;
		uint32			service_count;
	};

	class MessageQueueRepository;

	class NetworkFacade
	{
	public:
		typedef ACE_DLList<AbstractAcceptor>			AcceptorList;
		typedef ACE_DLList_Iterator<AbstractAcceptor>	AcceptorIter;

	public:
		NetworkFacade();
		virtual ~NetworkFacade();

	public:
		///////////////////////////////////////////////////////
		// Description:
		//   Start listen, start worker thread and so on
		//   Before Open you must Create
		// See Also: Create
		// Arguments:
		//   config - all about config values
		// Return:
		//   success or not
		///////////////////////////////////////////////////////
		bool	Open(SystemConfig* const config = NULL);
		void	Close();

	public:
		///////////////////////////////////////////////////////
		// Description:
		//   Configure Event model
		// Arguments:
		//   factory - EventModel factory
		// Return:
		//   success or not
		///////////////////////////////////////////////////////
		template <class EventModel>
		bool	Create()
		{
			return Create(new EventModel);
		}
		void	Destroy();

	public:
		/////////////////////////////////////////////////////////////////////////
		// Arguments:
		//   acceptIp - allowed ip. if you don't know much, just put NULL or ""
		//   port - accept port
		//   queueId - Related queue id
		// Return:
		//   success or not
		/////////////////////////////////////////////////////////////////////////
		bool	AddAcceptor(const uint16 port, const uint8 queueId);
		bool	AddAcceptor(const ACE_TCHAR* acceptIp, const uint16 port, const uint8 queueId);
		bool	AddAcceptor(const ACE_TCHAR* acceptIp, const uint16 port, const uint8 queueId, const uint32 receiveBufferSize);
		bool	AddAcceptor(
			const ACE_TCHAR* acceptIp, 
			const uint16 port, 
			const uint8 queueId, 
			const uint32 receiveBufferSize, 
			const uint32 initialAcceptCount);

		/////////////////////////////////////////////////////////////////////////
		// Description:
		//   Connect right now. Result will be informed Message through queue 
		// Arguments:
		//   queueId - Related queue id
		/////////////////////////////////////////////////////////////////////////
		bool	Connect(const ACE_TCHAR* ip, const uint16 port, const uint8 queueId);
		bool	Connect(const ACE_TCHAR* ip, const uint16 port, const uint8 queueId, const uint32 receiveBufferSize);

	public:
		////////////////////////////////////////////////////////////
		// Description:
		//   Get Message from specified MessageQueue(queue_id)
		// Arguments:
		//   timeout_ms : wait time(millisecond) / -1 means infinite
		///////////////////////////////////////////////////////////
		bool	PopMessage(const uint8 queueId, ACE_Message_Block** block, const int timeout);
		bool	PopAllMessage(const uint8 queueId, ACE_Message_Block** block, const int timeout);

		/////////////////////////////////////////////////////
		// Description:
		//   Get Message from whole MessageQueue(make chain)
		/////////////////////////////////////////////////////
		bool	PopAllMessage(ACE_Message_Block** block, const int timeout);

		/////////////////////////////////////////////////////
		// Description:
		//   Push user custom message block
		/////////////////////////////////////////////////////
		bool	PushCustomMessage(const uint8 queId, ACE_Message_Block* block);

		/////////////////////////////////////////////////////
		// Description:
		//   Wake up thread which wait PopMessage
		/////////////////////////////////////////////////////
		void	Pulse();
		void	Pulse(const uint8 queueId);

	public:
		///////////////////////////////////////////////////////////////////////////////////
		// Description:
		//   Send block or buffer. 
		//   Directly(eSM_Direct) or after a while(eSM_Interval) depend on mode(ESendMode)
		///////////////////////////////////////////////////////////////////////////////////
		bool	SendRequest(const uint32 streamId, ACE_Message_Block* block, bool copy_block = false);
		bool	SendRequest(const uint32 streamId, const char* buffer, const uint32 length);

		///////////////////////////////////////////////////////////////////////////////////
		// Description:
		//   Send buffer delayed time
		// Arguments:
		//   delay : delayed time(millisecond)
		///////////////////////////////////////////////////////////////////////////////////
		bool	SendReserve(const uint32 streamId, const char* buffer, const uint32 length, const uint32 delay);

		/////////////////////////////////////////////////
		// Description:
		//   Close connection
		/////////////////////////////////////////////////
		void	CloseStream(const uint32 streamID);
		void	CloseReceiveStream(const uint32 streamID);

	public:
		/////////////////////////////////////////////////////
		// Description:
		//   Setup options
		/////////////////////////////////////////////////////
		void	SetWorkerThreadCount(const uint8 count);
		void	SetSendMode(const ESendMode mode);
		void	SetReceiveBufferLen(const uint32 length);
		void	SetIntervalSendTerm(const uint32 ms);

	public:
		void	GetInformation(NetworkFacadeInfo& info);

		/////////////////////////////////////////////////////
		// Description:
		//   Show current status
		/////////////////////////////////////////////////////
		void	Dump();
		ACE_Message_Block*	AllocateBlock(const size_t bufferSize);
		void	PrepareMessageBlock(const size_t bufferSize, const uint32 count);
		void	PrepareMemoryBlock(const size_t bufferSize, const uint32 count);

	public:
		/////////////////////////////////////////////////////
		// Description:
		//   Stop accept
		/////////////////////////////////////////////////////
		bool	SuspendAcceptor(const uint16 port);
		bool	ResumeAcceptor(const uint16 port);

	protected:
		//////////////////////////////////////////////////////////////////////////////////
		// Description:
		//   Push event message. For example
		//   Accept(Connect) new connection / Receive packets / Close connection
		//////////////////////////////////////////////////////////////////////////////////
		bool	PushMessage(const uint8 queId, 
							const char command, 
							const uint32 serial, 
							ACE_Message_Block* block);

	protected:
		bool	Create(AbstractFactory* const factory);
		bool	StartListen();
		void	ClearAcceptor();
		bool	CheckDuplicatedServicePort(const uint16 port);
		void	CloseService();
		void	ClearMessageQueue();

		/////////////////////////////////////////////////////
		// Description:
		//   Setup default value of config
		/////////////////////////////////////////////////////
		void	SetupDefaultConfig();

	private:
		AbstractFactory*			_factory;
		AbstractConnector*			_connector;
		AbstractEventPool*			_eventPool;
		AbstractServiceAccessor*	_serviceAccessor;

	private:
		MessageQueueRepository*		_queueRepository;
		FlexibleSizePoolT<MessageBlockPool, ACE_Message_Block>*	_blockPool;

	private:
		AcceptorList				_acceptorList;		// acceptor repository

	private:
		ACE_Atomic_Op<ACE_Thread_Mutex, int32>	_suspend;

	friend class ServiceImpl;

	};
	typedef ACE_Singleton<NetworkFacade, ACE_Thread_Mutex> NetworkFacadeSingleton;
} // namespace HalfNetwork

#define NetworkInstance HalfNetwork::NetworkFacadeSingleton::instance()

#endif // __networkfacade_h__