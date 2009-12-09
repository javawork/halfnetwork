#ifndef __proactorconnector_h__
#define __proactorconnector_h__

#pragma once
#include <ace/Asynch_Connector.h>
#include "ProactorService.h"
#include "AbstractConnector.h"

namespace HalfNetwork
{

	/////////////////////////////////////////////////////
	// Description:
	// Custom Asynch_Connector
	/////////////////////////////////////////////////////
	class CustomConnector : public ACE_Asynch_Connector<ProactorService>
	{
	public:
		CustomConnector();
		virtual ~CustomConnector() {}

	public:
		ProactorService*	make_handler();
		void QueueID(uint8 id);
		void ReceiveBufferSize(uint32 size);

	private:
		uint8	_queueId;
		uint32 _receive_buffer_size;
	};

	/////////////////////////////////////////
	// Description:
	// Connector Interface for proactor
	/////////////////////////////////////////
	class ProactorConnector : public AbstractConnector
	{
	public:
		ProactorConnector();
		virtual ~ProactorConnector() {}

	public:
		bool	Open();
		void	Close();

	public:
		virtual bool	Connect(const ACE_TCHAR* ip, const uint16 port, const uint8 queue_id);
		virtual bool	Connect(const ACE_TCHAR* ip, const uint16 port, const uint8 queue_id, const uint32 receiveBufferSize);

	private:
		CustomConnector	m_connector;
		ACE_INET_Addr	m_connectAddr;
	};

} // namespace HalfNetwork

#endif // __proactorconnector_h__