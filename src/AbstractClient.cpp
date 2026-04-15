#include "AbstractClient.h"

using namespace std;

bool AbstractClient::Connect(const string& address, const unsigned int port)
{
   const bool ok = StartConnection(address, port);
   if (ok)
   {
      receiveThread.reset();
      canStop = false;
      receiveThread = std::make_unique<std::thread>([this]()
      {
         Run();
      });
   }
   return ok;
}

bool AbstractClient::Disconnect()
{
   const bool ok = StopConnection();
   if (ok)
   {
      canStop = true;
      receiveThread->join();
   }
   return ok;
}

void AbstractClient::SetHandlers(ConnectionHandler _connectHandler, ConnectionHandler _disconnectHandler, ReceivedDataHandler _receivedHandler)
{
   connectHandler = _connectHandler;
   disconnectHandler = _disconnectHandler;
   dataReceivedHandler = _receivedHandler;
}

void AbstractClient::Run()
{
   while (!canStop)
   {
      const DataResult result = GetNewData();
      if (result.status == DataStatus::Valid)
         dataReceivedHandler(result.data);
      if (result.status == DataStatus::Disconnect)
         HandleDisconnection();
   }
}
