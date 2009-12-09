#include <string>
#include <ace/Singleton.h>
#include "HalfNetworkType.h"
#include "ServiceImpl.h"
#include "NetworkFacade.h"
#include "TimerUtil.h"
#include "MessageBlockUtil.h"
#include "InterlockedValue.h"

namespace HalfNetwork
{

	ServiceImpl::ServiceImpl() : 
			_block_queue(new ACE_Message_Queue<ACE_MT_SYNCH>),
			_closeFlag(new InterlockedValue()),
			_timerLock(new InterlockedValue()),
			_sendLock(new InterlockedValue())
	{
	}

	ServiceImpl::~ServiceImpl()
	{
		_block_queue->close();
		SAFE_DELETE(_block_queue);
		SAFE_DELETE(_closeFlag);
		SAFE_DELETE(_timerLock);
		SAFE_DELETE(_sendLock);
	}

	void ServiceImpl::PushQueue(ACE_Message_Block* block, uint32 tick)
	{
		block->msg_priority(tick);
		_block_queue->enqueue_prio(block);
	}

	bool ServiceImpl::PopQueue(ACE_Message_Block** param_block)
	{
		ACE_Time_Value wait_time(0, 0);
		uint32 current_tick = GetTick();
		ACE_Message_Block* block = NULL;
		uint32 block_count = 0;
		do 
		{
			if (-1 == _block_queue->dequeue_prio(block, &wait_time))
				break;
			if (block->msg_priority() > current_tick)
			{
				_block_queue->enqueue_prio(block, &wait_time);
				break;
			}

			if (++block_count <= 1)
			{
				*param_block = block;
				continue;
			}

			ACE_Message_Block* tail_block = FindTailBlock(*param_block);
			if (NULL != tail_block)
				tail_block->cont(block);
		} while(true);

		if (0 == block_count)
			return false;
		return true;
	}

	void ServiceImpl::PushEventBlock( int8 eventType, uint8 queueID, uint32 serial, ACE_Message_Block* block )
	{
		NetworkInstance->PushMessage(queueID, eventType, serial, block);
	}

	ACE_Message_Block* ServiceImpl::AllocateBlock( size_t size )
	{
		return NetworkInstance->AllocateBlock(size);
	}

	int8 ServiceImpl::GetCloseFlag()
	{
		if (_closeFlag->Compare(eCF_Passive))
			return eCF_Passive;
		else if (_closeFlag->Compare(eCF_Active))
			return eCF_Active;
		else if (_closeFlag->Compare(eCF_Receive))
			return eCF_Receive;
		return eCF_None;
	}

	void ServiceImpl::SetCloseFlag( int8 flag )
	{
		_closeFlag->Exchange(flag);
	}

	bool ServiceImpl::AcquireTimerLock()
	{
		return  _timerLock->Acquire();
	}

	void ServiceImpl::ReleaseTimerLock()
	{
		_timerLock->Release();
	}

	bool ServiceImpl::AcquireSendLock()
	{
		return  _sendLock->Acquire();
	}

	void ServiceImpl::ReleaseSendLock()
	{
		_sendLock->Release();
	}

	bool ServiceImpl::TrySendLock()
	{
		return _sendLock->Compare(0);
	}
} // namespace HalfNetwork
