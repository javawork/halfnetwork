#ifndef __proactorfactory_h__
#define __proactorfactory_h__

#pragma once
#include "AbstractFactory.h"

namespace HalfNetwork
{

	class ProactorFactory : public AbstractFactory
	{
	public:
		ProactorFactory();
		virtual ~ProactorFactory() {}

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

#endif // __proactorfactory_h__