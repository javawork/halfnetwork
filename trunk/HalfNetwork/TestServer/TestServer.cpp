// TestServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <ace/ACE.h>
#include "NetworkFacade.h"
#ifdef WIN32
	#include "ProactorFactory.h"
#else
	#include "ReactorFactory.h"
#endif
#include "MessageHeader.h"
#include "SimpleConfig.h"

void OnAccept(unsigned int streamID, const char* ip);
void OnRead(unsigned int streamID, char* buffer, unsigned int length);
void OnClose(unsigned int streamID);

bool NetworkInit(const HalfNetwork::SimpleConfig& configReader, unsigned char queueID)
{
	unsigned short port = configReader.GetValue<unsigned short>(ACE_TEXT("port"));
	HalfNetwork::SystemConfig config;
	config.Worker_Thread_Count = configReader.GetValue<unsigned char>(ACE_TEXT("workerthread"));
	config.Receive_Buffer_Len = configReader.GetValue<unsigned int>(ACE_TEXT("receivebufferlength"));
	config.Interval_Send_Term = configReader.GetValue<unsigned int>(ACE_TEXT("updateterm"));
	config.Send_Mode = HalfNetwork::eSM_Interval;
	
	HALF_LOG(ConsoleLogger, ACE_TEXT("Port(%d)"), port);
	HALF_LOG(ConsoleLogger, ACE_TEXT("WorkerThread Count(%d)"), config.Worker_Thread_Count);
	HALF_LOG(ConsoleLogger, ACE_TEXT("Receive buffer length(%d)"), config.Receive_Buffer_Len);
	HALF_LOG(ConsoleLogger, ACE_TEXT("Update term(%d)"), config.Interval_Send_Term);

#ifdef WIN32
	if (false == NetworkInstance->Create<HalfNetwork::ProactorFactory>())
#else
	if (false == NetworkInstance->Create<HalfNetwork::ReactorFactory>())
#endif
		return false;
	if (false == NetworkInstance->AddAcceptor(NULL, port, queueID))
		return false;
	if (false == NetworkInstance->Open(&config))
		return false;

	HALF_LOG(ConsoleLogger, ACE_TEXT("Network start"), 0);
	return true;
}

void NetworkFini()
{
	NetworkInstance->Close();
	NetworkInstance->Destroy();
}

void NetworkUpdate(unsigned char queueID)
{
	using namespace HalfNetwork;
	ACE_Message_Block* headBlock = NULL;
	ACE_Message_Block* commandBlock = NULL;

	bool receiveData = NetworkInstance->PopAllMessage(queueID, &headBlock, -1);
	if (false == receiveData)
		return;

	commandBlock = headBlock;
	while(NULL != commandBlock)
	{
		MessagePostee postee;
		memcpy(&postee, commandBlock->rd_ptr(), sizeof(MessagePostee));
		commandBlock->rd_ptr(sizeof(MessagePostee));
		ACE_Message_Block* payloadBlock = commandBlock->cont();

		switch(postee.command)
		{
		case eMH_Establish:
			OnAccept(postee.stream_id, payloadBlock->rd_ptr());
			break;
		case eMH_Read:
			OnRead(postee.stream_id, payloadBlock->rd_ptr(), (unsigned int)payloadBlock->length());
			break;
		case eMH_Close:
			OnClose(postee.stream_id);
			break;
		case eMH_Custom:
			// process user defined data
			break;
		default:
			break;
		}
		commandBlock = payloadBlock->cont();
	}
	headBlock->release();
}

void OnAccept(unsigned int streamID, const char* ip)
{
	HALF_LOG (ConsoleLogger, ACE_TEXT("OnAccept(%d)."), streamID);
}

void OnRead(unsigned int streamID, char* buffer, unsigned int length)
{
	HALF_LOG (ConsoleLogger, ACE_TEXT("OnRead(%d, %d)."), streamID, length);
	ACE_Message_Block* block = NetworkInstance->AllocateBlock(length);
	block->copy(buffer, length);
	NetworkInstance->SendRequest(streamID, block);
}

void OnClose(unsigned int streamID)
{
	HALF_LOG(ConsoleLogger, ACE_TEXT("OnClose(%d)."), streamID);
}

int ACE_TMAIN (int, ACE_TCHAR *[])
{
	ACE::init();
	ConsoleLogger->SetConsoleLog();
	ConsoleLogger->AppendThreadId();

	const ACE_TCHAR* ConfigFilename = ACE_TEXT("TestServerConfig.txt");
	HalfNetwork::SimpleConfig configReader;
	if (false == configReader.ReadFile(ConfigFilename))
	{
		HALF_LOG(ConsoleLogger, ACE_TEXT("Read config fail(%s)"), ConfigFilename);
		return 0;
	}

	const unsigned char QueueID = 103;
	if (false == NetworkInit(configReader, QueueID))
	{
		HALF_LOG(ConsoleLogger, ACE_TEXT("NetworkInit Fail."), 0);
		return 0;
	}

	while(true)
		NetworkUpdate(QueueID);

	NetworkFini();
	ACE::fini();
	return 0;
}

