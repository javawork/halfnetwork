#ifndef __reactorserviceaccessor_h__
#define __reactorserviceaccessor_h__

#pragma once
#include "AbstractServiceAccessor.h"

namespace HalfNetwork
{

	class AbstractReactorSend
	{
	public:
		virtual ~AbstractReactorSend() {}

	public:
		virtual	bool	Send(uint32 streamID, ACE_Message_Block* block) = 0;
	};

	class IntervalReactorSend : public AbstractReactorSend
	{
	public:
		bool	Send(uint32 streamID, ACE_Message_Block* block);
	};

	class DirectReactorSend : public AbstractReactorSend
	{
	public:
		bool	Send(uint32 streamID, ACE_Message_Block* block);
	};

	/////////////////////////////////////////////
	// Description:
	//   ReactorServiceAccessor Accessor
	/////////////////////////////////////////////
	class ReactorServiceAccessor : public AbstractServiceAccessor
	{
	public:
		ReactorServiceAccessor(uint8 send_mode);
		virtual ~ReactorServiceAccessor();

	public:
		virtual	bool	SendRequest(const uint32 streamID, ACE_Message_Block* block);
		virtual	bool	SendReserve(const uint32 streamID, ACE_Message_Block* block, const uint32 delay);
		virtual	void	DisableService(const uint32 streamID);
		virtual	void	CloseService();
		virtual	uint32	ServiceCount();
		virtual	void	CloseReceiveStream(const uint32 streamID);

	private:
		AbstractReactorSend*	_send_strategy;

	};

} // namespace HalfNetwork

#endif // __reactorserviceaccessor_h__