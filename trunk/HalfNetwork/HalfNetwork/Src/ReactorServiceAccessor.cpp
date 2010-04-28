
#include <string>
#include <ace/Singleton.h>
#include "HalfNetworkType.h"
#include "HalfNetworkDefine.h"
#include "ReactorServiceAccessor.h"
#include "ReactorService.h"
#include "TimerUtil.h"
#include <ace/Event.h>

namespace HalfNetwork
{

	bool IntervalReactorSend::Send(uint32 streamID, ACE_Message_Block* block)
	{
		ReactorService* service = ReactorServiceMap->Get(streamID);
		if (NULL == service)
			return false;
		return service->IntervalSend(block);
	}

	bool DirectReactorSend::Send(uint32 streamID, ACE_Message_Block* block)
	{
		ReactorService* service = ReactorServiceMap->Get(streamID);
		if (NULL == service)
			return false;
		return service->DirectSend(block);
	}

	ReactorServiceAccessor::ReactorServiceAccessor(uint8 send_mode) : _send_strategy(NULL)
	{
		switch(send_mode)
		{
		case eSM_Interval:
			_send_strategy = new IntervalReactorSend();
			break;
		case eSM_Direct:
			_send_strategy = new DirectReactorSend();
			break;
		}
	}

	ReactorServiceAccessor::~ReactorServiceAccessor()
	{
		SAFE_DELETE(_send_strategy);
	}

	bool ReactorServiceAccessor::SendRequest(const uint32 streamID, ACE_Message_Block* block)
	{
		return _send_strategy->Send(streamID, block);
	}

	bool ReactorServiceAccessor::SendReserve(const uint32 streamID, ACE_Message_Block* block, const uint32 delay)
	{
		return _send_strategy->Send(streamID, block);
	}

	void ReactorServiceAccessor::DisableService(const uint32 streamID)
	{
		ReactorService* service = ReactorServiceMap->Get(streamID);
		if (NULL == service)
			return;
		service->ActiveClose();
	}

	void ReactorServiceAccessor::CloseService()
	{
		for (uint32 i=0;i<Max_ReactorService; ++i)
		{
			ReactorService* service = ReactorServiceMap->Get(i);
			if (NULL == service)
				continue;

			service->ActiveClose();
			//SAFE_DELETE(service);
		}

		ACE_Event wait_event;
		ACE_Time_Value* wait_time = new ACE_Time_Value(ACE_OS::gettimeofday() + ACE_Time_Value(0, 100*1000));
		while (0 != ReactorServiceMap->Size())
		{
			wait_event.wait(wait_time);
		}
		delete wait_time;
	}

	uint32 ReactorServiceAccessor::ServiceCount()
	{
		return ReactorServiceMap->Size();
	}

	void ReactorServiceAccessor::CloseReceiveStream(const uint32 streamID)
	{
		ReactorService* service = ReactorServiceMap->Get(streamID);
		if (NULL == service)
			return;
		service->ReceiveClose();
	}

} // namespace HalfNetwork