#include "AbstractClient.h"

using namespace std;

bool AbstractClient::Connect(const string& address, const unsigned int port)
{
   const bool ok = StartConnection(address, port);
   if (ok)
   {
      receiveThread.reset();
      canStop = false;
      receiveThread = make_unique<std::thread>([this]()
      {
         Run();
      });

      processThread = make_unique<std::thread>([this]()
      {
         ProcessQueue();
      });
   }
   return ok;
}

bool AbstractClient::Disconnect()
{
   pendingDisconnect = true;
   receiveThread->join();
   processThread->join();
   return true;
}

bool AbstractClient::IsConnected() const
{
   return !canStop;
}

void AbstractClient::SetHandlers(ConnectionHandler _disconnectHandler, ReceivedDataHandler _receivedHandler)
{
   disconnectHandler = _disconnectHandler;
   dataReceivedHandler = _receivedHandler;
}

void AbstractClient::Run()
{
   while (!canStop)
   {
      HandleReceivedData();
      this_thread::sleep_for(threadWaitTime);
   }
}

void AbstractClient::HandleReceivedData()
{
   lock_guard<mutex> lock(dataMutex);
   const DataResult result = GetNewData();
   if (result.status == DataStatus::Valid)
      dataQueue.push(result);
   else if (result.status == DataStatus::Disconnect)
      pendingDisconnect = true;
}

void AbstractClient::ProcessQueue()
{
   while (!canStop)
   {
      ProcessReceivedData();
      ProcessDisconnection();
      this_thread::sleep_for(threadWaitTime);
   }
}

void AbstractClient::ProcessReceivedData()
{
   if (!dataQueue.empty())
   {
      lock_guard<mutex> lock(dataMutex);
      const auto& data = dataQueue.front();
      dataReceivedHandler(data.data);
      dataQueue.pop();
   }
}

void AbstractClient::ProcessDisconnection()
{
   if (pendingDisconnect)
   {
      lock_guard<mutex> lock(disconnectionMutex);
      const bool ok = StopConnection();
      if (ok)
      {
         canStop = true;
         disconnectHandler("server");
      }
      pendingDisconnect = false;
   }
}
