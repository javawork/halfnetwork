#include <string>
#include <ace/Singleton.h>
#include "HalfNetworkType.h"
#include "ProactorConnector.h"

namespace HalfNetwork
{
	///////////////////////
	// CustomConnector
	///////////////////////
	CustomConnector::CustomConnector() : _queueId(0), _receive_buffer_size(0)
	{
	}

	ProactorService* CustomConnector::make_handler()
	{
		ProactorService* handler = new ProactorService();
		handler->QueueID(_queueId);
		if (0 != _receive_buffer_size)
			handler->ReceiveBufferSize(_receive_buffer_size);
		return handler;
	}

	void CustomConnector::QueueID(uint8 id)
	{
		_queueId = id;
	}

	void CustomConnector::ReceiveBufferSize( uint32 size )
	{
		_receive_buffer_size = size;
	}
	///////////////////////
	// ProactorConnector
	///////////////////////
	ProactorConnector::ProactorConnector()
	{
	}

	bool ProactorConnector::Open()
	{
		m_connector.open();
		return true;
	}

	void ProactorConnector::Close()
	{
	}

	bool ProactorConnector::Connect(const ACE_TCHAR* ip, const uint16 port, const uint8 queue_id)
	{
		return Connect(ip, port, queue_id, 0);
	}

	bool ProactorConnector::Connect( const ACE_TCHAR* ip, const uint16 port, const uint8 queue_id, const uint32 receiveBufferSize )
	{
		m_connector.QueueID(queue_id);
		m_connectAddr.set(port, ip);
		if (0 != receiveBufferSize)
			m_connector.ReceiveBufferSize(receiveBufferSize);
		if (-1 == m_connector.connect(m_connectAddr))
			return false;

		return true;
	}
} // namespace HalfNetwork
