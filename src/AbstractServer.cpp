#include "AbstractServer.h"

using namespace std;

bool AbstractServer::Start(const std::string& address, const unsigned int port)
{
   const bool ok = StartConnection(address, port);
   if (ok)
   {
      serverThread.reset();
      canStop = false;
      serverThread = std::make_unique<std::thread>([this]()
      {
         Run();
      });
   }
   return ok;
}

bool AbstractServer::Stop()
{
   const bool ok = StopConnection();
   if (ok)
   {
      canStop = true;
      serverThread->join();
   }
   return ok;
}

void AbstractServer::SetHandlers(ConnectionHandler _connectHandler, ConnectionHandler _disconnectHandler, ReceivedDataHandler _receivedHandler)
{
   connectHandler = _connectHandler;
   disconnectHandler = _disconnectHandler;
   dataReceivedHandler = _receivedHandler;
}

void AbstractServer::Run()
{
   while (!canStop)
   {
      HandleNewConnections();
      HandleReceivedData();
   }
}

void AbstractServer::HandleNewConnections()
{
   std::optional<ClientId> clientId = GetNewConnection();
   while (clientId.has_value())
   {
      connectedClients[clientId.value().socket] = clientId.value().address;
      connectHandler(clientId.value().address);
      clientId = GetNewConnection();
   }
}

void AbstractServer::HandleReceivedData()
{
   for (const auto client : connectedClients)
   {
      DataResult result = GetNewData(client.first);
      while (result.status == DataStatus::Valid)
      {
         dataReceivedHandler(client.second, result.data);
         result = GetNewData(client.first);
      }

      if (result.status == DataStatus::Disconnect)
         HandleDisconnection(client);
   }
}

void AbstractServer::HandleDisconnection(const std::pair<int, string>& clientId)
{
   connectedClients.erase(clientId.first);
   disconnectHandler(clientId.second);
}
