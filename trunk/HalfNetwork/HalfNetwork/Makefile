INC = -I/usr/src/ACE_wrappers
CFLAGS = -g $(INC) 

OBJS = MessageBlockPool.o MessageQueue.o MessageQueueRepository.o NetworkFacade.o ProactorAcceptor.o ProactorConnector.o ProactorFactory.o ProactorPool.o ProactorService.o ProactorServiceAccessor.o ReactorAcceptor.o ReactorConnector.o ReactorFactory.o ReactorPool.o ReactorService.o ReactorServiceAccessor.o ThreadPool.o TsBlockList.o ServiceImpl.o SimpleConfig.o MemoryBlockPool.o InterlockedValue.o LogCallback.o Logger.o SyncSocket.o
SRCS = MessageBlockPool.cpp MessageQueue.cpp MessageQueueRepository.cpp NetworkFacade.cpp ProactorAcceptor.cpp ProactorConnector.cpp ProactorFactory.cpp ProactorPool.cpp ProactorService.cpp ProactorServiceAccessor.cpp ReactorAcceptor.cpp ReactorConnector.cpp ReactorFactory.cpp ReactorPool.cpp ReactorService.cpp ReactorServiceAccessor.cpp ThreadPool.cpp TsBlockList.cpp ServiceImpl.cpp SimpleConfig.cpp MemoryBlockPool.cpp InterlockedValue.cpp LogCallback.cpp Logger.cpp SyncSocket.cpp
DEF = -DACE_AS_STATIC_LIBS

TARGET = libHalfNetwork.a

all : $(TARGET)

$(TARGET) : 
	g++ -c $(SRCS) $(INC) $(DEF)
	ar rcv $(TARGET) $(OBJS)
	ranlib $(TARGET)
	
clean :
	rm -rf $(OBJS) $(TARGET)

new : 
	$(MAKE) clean 
	$(MAKE) 

