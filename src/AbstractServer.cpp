#include "AbstractServer.h"

using namespace std;

AbstractServer::AbstractServer(std::unique_ptr<AbstractNetworkConnector> _connector)
   : AbstractNetworkAgent(std::move(_connector))
{
}

bool AbstractServer::Start(const std::string& address, const unsigned int port)
{
   const bool ok = connector->StartServer(address, port);
   if (ok)
      StartNetworkProcessing();
   return ok;
}

bool AbstractServer::Stop()
{
   const bool ok = connector->Stop();
   if (ok)
      StopNetworkProcessing();
   return ok;
}

bool AbstractServer::Send(const std::string& address, const DataFrame& buffer)
{
   const int clientSocket = FindClientSocket(address);
   return connector->Send(clientSocket, buffer);
}

bool AbstractServer::DisconnectClient(const std::string& address)
{
   const int clientSocket = FindClientSocket(address);
   return DisconnectClient(clientSocket);
}

bool AbstractServer::DisconnectAllClients()
{
   bool ok = false;
   for (const auto& client : connectedClients)
      ok |= DisconnectClient(client.first);
   connectedClients.clear();
   return ok;
}

void AbstractServer::SetHandlers(ConnectionHandler _connectHandler, ConnectionHandler _disconnectHandler, ReceivedDataHandler _receivedHandler)
{
   connectHandler = _connectHandler;
   disconnectHandler = _disconnectHandler;
   dataReceivedHandler = _receivedHandler;
}

void AbstractServer::HandleNetworkEvents()
{
   HandleNewConnections();
   HandleReceivedData();
}

void AbstractServer::HandleNewConnections()
{
   lock_guard<mutex> lock(connectionMutex);
   std::optional<ClientId> clientId = connector->GetNewConnection();
   while (clientId.has_value())
   {
      connectionQueue.push(clientId.value());

      clientId = connector->GetNewConnection();
   }
}

void AbstractServer::HandleReceivedData()
{
   map<int, string>::iterator it = connectedClients.begin();
   while (it != connectedClients.end())
   {
      DataResult result = connector->Receive(it->first);
      while (result.status == DataStatus::Valid)
      {
         ClientId clientId;
         clientId.socket = it->first;
         clientId.address = it->second;

         dataMutex.lock();
         dataQueue.push(make_pair(clientId, result));
         dataMutex.unlock();

         result = connector->Receive(it->first);
      }

      if (result.status == DataStatus::Disconnect)
         it = HandleDisconnection(*it);
      else
         ++it;
   }
}

map<int, string>::iterator AbstractServer::HandleDisconnection(const std::pair<int, string>& clientId)
{
   lock_guard<mutex> lock(disconnectionMutex);

   ClientId client;
   client.socket = clientId.first;
   client.address = clientId.second;

   auto clientIt = connectedClients.find(clientId.first);
   map<int, string>::iterator newIt = connectedClients.erase(clientIt);
   disconnectionQueue.push(client);

   return newIt;
}

bool AbstractServer::DisconnectClient(const int socket)
{
   // TODO implement and test
   return false;
}

int AbstractServer::FindClientSocket(const std::string& address) const
{
   for (const auto& client : connectedClients)
   {
      if (client.second == address)
         return client.first;
   }
   return -1;
}

void AbstractServer::ProcessActionQueue()
{
   ProcessNewConnections();
   ProcessReceivedData();
   ProcessDisconnections();
}

void AbstractServer::ProcessNewConnections()
{
   lock_guard<mutex> lock(connectionMutex);
   if (!connectionQueue.empty())
   {
      const auto& client = connectionQueue.front();
      connectedClients[client.socket] = client.address;
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
