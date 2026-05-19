#include "AbstractNetworkAgent.h"

using namespace std;

AbstractNetworkAgent::AbstractNetworkAgent(std::unique_ptr<AbstractNetworkConnector> _connector)
   : connector(std::move(_connector))
{
}

AbstractNetworkAgent::~AbstractNetworkAgent()
{
   StopNetworkProcessing();
}

void AbstractNetworkAgent::SetWaitTime(std::chrono::duration<double, std::milli> waitTime)
{
   threadWaitTime = waitTime;
}

void AbstractNetworkAgent::StartNetworkProcessing()
{
   canStop = false;

   LaunchThreadedProcessing(receiveThread, [this]()
   {
      HandleNetworkEvents();
   });

   LaunchThreadedProcessing(processThread, [this]()
   {
      ProcessActionQueue();
   });
}

void AbstractNetworkAgent::StopNetworkProcessing()
{
   canStop = true;
   if (receiveThread && receiveThread->joinable())
      receiveThread->join();
   if (processThread && processThread->joinable())
      processThread->join();
}

void AbstractNetworkAgent::LaunchThreadedProcessing(std::shared_ptr<thread>& thread, ThreadedProcessing processing)
{
   thread.reset();
   thread = std::make_shared<std::thread>([this, processing]()
   {
      while (!canStop)
      {
         processing();
         this_thread::sleep_for(threadWaitTime);
      }
   });
}
