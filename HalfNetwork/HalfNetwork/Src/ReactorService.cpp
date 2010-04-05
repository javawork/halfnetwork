
#include <string>
#include <ace/Singleton.h>
#include "HalfNetworkType.h"
#include "HalfNetworkDefine.h"

#include "ReactorService.h"
#include <ace/Reactor.h>
#include "MessageHeader.h"
#include "TimerUtil.h"
#include "ServiceImpl.h"
#include "InterlockedValue.h"
#include "MessageBlockUtil.h"

namespace HalfNetwork
{
	//////////////////////////////////////////////////////////////////////////
	ReactorService::ReactorService() : 
		_closeLock(new InterlockedValue()),
		_serial(Invalid_ID), 
		_queue_id(0),
		_receive_buffer_size(SystemConfigInst->Receive_Buffer_Len),
		_serviceImpl(new ServiceImpl())
	{
		_peer_ip = new char[IP_ADDR_LEN];
		_serviceImpl->SetCloseFlag(eCF_None);
		_serviceImpl->ReleaseTimerLock();
		_serviceImpl->ReleaseSendLock();
		_closeLock->Release();
	}

	ReactorService::~ReactorService()
	{
		delete [] _peer_ip;
		_peer_ip = NULL;
		//ACE_DEBUG ((LM_DEBUG, "[%t] ReactorService Destroy.\n"));
		_CleanUp();
		SAFE_DELETE(_serviceImpl);
		SAFE_DELETE(_closeLock);
	}

	int ReactorService::open()
	{
		this->reactor()->dump();
		ACE_INET_Addr peer_addr;
		if (0 == this->_sock.get_remote_addr(peer_addr))
		{
			peer_addr.get_host_addr(_peer_ip, IP_ADDR_LEN);
		}

		_Register();
		_OnEstablish();
		_RegisterTimer();
		return this->reactor()->register_handler(this, ACE_Event_Handler::READ_MASK);
	}

	int	ReactorService::open(void* p)
	{
		ACE_UNUSED_ARG(p);
		return open();
	}

	int ReactorService::handle_input(ACE_HANDLE fd)
	{
		ACE_Message_Block* block = 
			_serviceImpl->AllocateBlock(_receive_buffer_size);
		ACE_ASSERT(NULL != block);

		ssize_t recv_length = 0;
		if ((recv_length = this->_sock.recv(block->wr_ptr(), block->space())) <= 0)
		{
			//_NotifyClose();
			//ACE_DEBUG ((LM_DEBUG,
			//	ACE_TEXT ("(%P|%t) Connection closed\n")));
			block->release();
			return -1;
		}
		block->wr_ptr(recv_length);
		_OnReceive(block);
		return 0;
	}

#if 0
	int ReactorService::handle_output(ACE_HANDLE fd)
	{
		ACE_UNUSED_ARG(fd);
		//printf("handle_output\n");
		ACE_Message_Block *mb = 0;
		ACE_Time_Value nowait (ACE_OS::gettimeofday ());
		while (-1 != this->getq (mb, &nowait))
		{
			ACE_DEBUG((LM_DEBUG, "handle_output send_length(%d)\n", mb->length ()));
			ssize_t send_cnt = _sock.send (mb->rd_ptr(), mb->length());
			
			ACE_DEBUG((LM_DEBUG, "handle_output sent_cnt(%d)\n", send_cnt));
			if (send_cnt == -1)
				ACE_ERROR ((LM_ERROR,
				ACE_TEXT ("(%P|%t) %p\n"),
				ACE_TEXT ("send")));
			else
				mb->rd_ptr (static_cast<size_t> (send_cnt));
			if (mb->length () > 0)
			{
				this->ungetq (mb);
				break;
			}
			mb->release ();
		}
		return (this->msg_queue()->is_empty()) ? -1 : 0;
	}
#endif

	int ReactorService::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask)
	{
		ACE_UNUSED_ARG(handle);
		if (mask == ACE_Event_Handler::WRITE_MASK)
			return 0;

		if (false == _closeLock->Acquire())
			return 0;

		_NotifyClose();
		//this->_wait_queue.flush();
		delete this;
		return 0;
	}

	int ReactorService::handle_timeout(const ACE_Time_Value &current_time, const void* act)
	{
		ACE_UNUSED_ARG(act);
		if ( false == _serviceImpl->AcquireTimerLock())
			return 0;

		if (false == _IsCloseFlagActivate())
			_SendQueuedBlock();

		_serviceImpl->ReleaseTimerLock();
		return 0;
	}

	ACE_HANDLE	ReactorService::get_handle() const 
	{ 
		return this->_sock.get_handle(); 
	}

	void ReactorService::set_handle(ACE_HANDLE handle) 
	{
		return this->_sock.set_handle(handle);
	}

	ACE_SOCK_Stream& ReactorService::peer() 
	{ 
		return this->_sock; 
	}

	void ReactorService::_Register()
	{
		_serial = ReactorServiceMap->Register(this);
		ACE_ASSERT(_serial != Invalid_ID);
	}

	void ReactorService::_UnRegister()
	{
		if (Invalid_ID == _serial)
			return;

		ReactorServiceMap->UnRegister(_serial);
	}

	void ReactorService::_CloseHandle()
	{
		if (ACE_INVALID_HANDLE == get_handle())
			return;

		_sock.close();
	}

	void ReactorService::_RemoveHandler()
	{
		this->reactor()->cancel_timer(this);
		ACE_Reactor_Mask mask = 
			ACE_Event_Handler::ALL_EVENTS_MASK | ACE_Event_Handler::DONT_CALL;
		this->reactor()->remove_handler(this, mask);
	}

	void ReactorService::_CleanUp()
	{
		_RemoveHandler();
		_CloseHandle();
		_UnRegister();
	}

	void ReactorService::_RegisterTimer()
	{
		ACE_Time_Value intervalTime(0, SystemConfigInst->Interval_Send_Term*1000);
		this->reactor()->schedule_timer(this, 0, ACE_Time_Value::zero, intervalTime);
	}

	void ReactorService::_ActiveClose()
	{
		if (ACE_INVALID_HANDLE == get_handle())
			return;

		_sock.close_writer();
	}

	void ReactorService::_ReceiveClose()
	{
		if (ACE_INVALID_HANDLE == get_handle())
			return;

		_sock.close_reader();
	}

	void ReactorService::QueueID(uint8 id)
	{
		_queue_id = id;
	}

	void ReactorService::ReceiveBufferSize( uint32 size )
	{
		_receive_buffer_size = size;
	}

	void ReactorService::ActiveClose()
	{
		_serviceImpl->SetCloseFlag(eCF_Active);
	}
	void ReactorService::ReceiveClose()
	{
		_serviceImpl->SetCloseFlag(eCF_Receive);
	}

	void ReactorService::ReserveClose()
	{
		_serviceImpl->SetCloseFlag(eCF_Passive);
	}

	void ReactorService::IntervalSend(ACE_Message_Block* block)
	{
		_PushQueue(block, 0);
		//_SmartSend(block);
	}

	void ReactorService::DirectSend(ACE_Message_Block* block)
	{
		_SmartSend(block);
	}

	void ReactorService::_OnEstablish()
	{
		ACE_Message_Block* block = _serviceImpl->AllocateBlock(IP_ADDR_LEN);
		block->copy((const char*)_peer_ip, IP_ADDR_LEN);
		_serviceImpl->PushEventBlock(eMH_Establish, _queue_id, _serial, block);
	}

	void ReactorService::_OnReceive(ACE_Message_Block* block)
	{
		_serviceImpl->PushEventBlock(eMH_Read, _queue_id, _serial, block);
	}

	void ReactorService::_NotifyClose()
	{
		ACE_Message_Block* block = new ACE_Message_Block();
		_serviceImpl->PushEventBlock(eMH_Close, _queue_id, _serial, block);
	}

	void ReactorService::_SmartSend(ACE_Message_Block* block)
	{
		if (false == _serviceImpl->AcquireSendLock())
		{
			_PushQueue(block, 0);
			return;
		}
		ssize_t send_cnt = _sock.send(block->rd_ptr(), block->length());
		if (send_cnt < (ssize_t)block->length())
		{
			// put the remain block
			if (send_cnt == -1)
				send_cnt = 0;
			ACE_Message_Block *remainBlock = 0;
			size_t remaining = static_cast<size_t> ((block->length() - send_cnt));
			remainBlock = _serviceImpl->AllocateBlock(remaining);
			block->rd_ptr(send_cnt);
			remainBlock->copy(block->rd_ptr(), remaining);
			 _PushQueue(remainBlock, 0);
		}
		else
		{
			block->release();
		}

		_serviceImpl->ReleaseSendLock();
		_SendQueuedBlock();
	}

	void ReactorService::_SendQueuedBlock()
	{
		if (false == _serviceImpl->TrySendLock())
			return;
		ACE_Message_Block* block = NULL;
		if (false == _PopQueue(&block))
			return;
		//printf("_SendQueuedBlock(%d, %d)\n", block->length(), block->total_length());
		if (NULL != block->cont())
		{
			ACE_Message_Block* mergedBlock = _serviceImpl->AllocateBlock(block->total_length());
			MakeMergedBlock(block, mergedBlock);
			block->release();
			_SmartSend(mergedBlock);
		}
		else
			_SmartSend(block);
	}

	void ReactorService::_PushQueue(ACE_Message_Block* block, uint32 tick)
	{
		//printf("_PushQueue(%d)\n", block->length());
		_serviceImpl->PushQueue(block, tick);
	}

	bool ReactorService::_PopQueue(ACE_Message_Block** param_block)
	{
		return _serviceImpl->PopQueue(param_block);
	}

	bool ReactorService::_IsCloseFlagActivate()
	{
		ECloseFlag closeFlag = (ECloseFlag)_serviceImpl->GetCloseFlag();
		if (eCF_Passive == closeFlag)
		{
			_NotifyClose();
			return true;
		}
		else if (eCF_Active == closeFlag)
		{
			_ActiveClose();
			ReserveClose();
			return true;
		}
		else if (eCF_Receive == closeFlag)
		{
			_ReceiveClose();
			_serviceImpl->SetCloseFlag(eCF_None);
			return false;
		}
		return false;
	}
} // namespace HalfNetwork