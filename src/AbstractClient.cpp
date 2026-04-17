#include "AbstractClient.h"

using namespace std;

AbstractClient::~AbstractClient()
{
   const bool forcingShutdown = (canStop == false);
   ShutdownConnection();
   if (forcingShutdown)
      disconnectHandler("server");
}

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
      ShutdownConnection();
      disconnectHandler("server");
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
      {
         // We don't call Disconnect() here because it will run on the receiving thread,
         // and the join will deadlock.
         StopConnection();
         canStop = true;
      }
   }
}

void AbstractClient::ShutdownConnection()
{
   canStop = true;
   receiveThread->join();
}
