#ifndef __reactorfactory_h__
#define __reactorfactory_h__

#pragma once

#include "AbstractFactory.h"

namespace HalfNetwork
{

	class ReactorFactory : public AbstractFactory
	{
	public:
		ReactorFactory();
		virtual ~ReactorFactory() {}

	public:
		AbstractAcceptor*			CreateAcceptor(const uint16 service_port, const uint8 queue_id);
		AbstractConnector*			CreateConnector();
		AbstractEventPool*			CreateEventPool();
		AbstractServiceAccessor*	CreateServiceAccessor(uint8 send_mode);
		const ACE_TCHAR*				GetFactoryName();

	private:
		ACE_TCHAR						m_factoryName[FACTORY_NAME_SIZE];
	};

} // namespace HalfNetwork

#endif // __reactorfactory_h__