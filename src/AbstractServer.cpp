#include "AbstractServer.h"

using namespace std;

bool AbstractServer::Start(const std::string& address, const unsigned int port)
{
   const bool ok = StartConnection(address, port);
   if (ok)
   {
      canStop = false;

      receiveThread.reset();
      receiveThread = std::make_unique<std::thread>([this]()
      {
         Run();
      });

      processThread.reset();
      processThread = std::make_unique<std::thread>([this]()
      {
         ProcessDataQueue();
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
      receiveThread->join();
      processThread->join();
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
      connectionMutex.lock();
      connectionQueue.push(clientId.value());
      connectionMutex.unlock();

      clientId = GetNewConnection();
   }
}

void AbstractServer::HandleReceivedData()
{
   map<int, string>::iterator it = connectedClients.begin();
   for (; it != connectedClients.end(); ++it)
   {
      DataResult result = GetNewData(it->first);
      while (result.status == DataStatus::Valid)
      {
         ClientId clientId;
         clientId.socket = it->first;
         clientId.address = it->second;

         dataMutex.lock();
         dataQueue.push(make_pair(clientId, result));
         dataMutex.unlock();

         result = GetNewData(it->first);
      }

      if (result.status == DataStatus::Disconnect)
         it = HandleDisconnection(*it);
   }
}

map<int, string>::iterator AbstractServer::HandleDisconnection(const std::pair<int, string>& clientId)
{
   auto clientIt = connectedClients.find(clientId.first);
   map<int, string>::iterator newIt = connectedClients.erase(clientIt);

   ClientId client;
   client.socket = clientId.first;
   client.address = clientId.second;
   disconnectionMutex.lock();
   disconnectionQueue.push(client);
   disconnectionMutex.unlock();
   return newIt;
}

void AbstractServer::ProcessDataQueue()
{
   while (!canStop)
   {
      ProcessNewConnections();
      ProcessReceivedData();
      ProcessDisconnections();
   }
}

void AbstractServer::ProcessNewConnections()
{
   lock_guard<mutex> lock(connectionMutex);
   if (!connectionQueue.empty())
   {
      const auto& client = connectionQueue.front();
      connectHandler(client.address);
      connectionQueue.pop();
   }
}

void AbstractServer::ProcessReceivedData()
{
   lock_guard<mutex> lock(dataMutex);
   if (!dataQueue.empty())
   {
      const auto& data = dataQueue.front();
      dataReceivedHandler(data.first.address, data.second.data);
      dataQueue.pop();
   }
}

void AbstractServer::ProcessDisconnections()
{
   lock_guard<mutex> lock(disconnectionMutex);
   if (!disconnectionQueue.empty())
   {
      const auto& client = disconnectionQueue.front();
      disconnectHandler(client.address);
      disconnectionQueue.pop();
   }
}
