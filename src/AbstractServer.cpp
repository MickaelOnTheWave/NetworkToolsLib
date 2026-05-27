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
   while (!connectedClients.empty())
   {
      const auto clientIt = connectedClients.begin();
      const bool ok = DisconnectClient(clientIt->first);
      if (!ok)
         return false;
   }
   return true;
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
   ClientMapIt it = connectedClients.begin();
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

AbstractServer::ClientMapIt AbstractServer::HandleDisconnection(const std::pair<int, string>& clientId)
{
   lock_guard<mutex> lock(disconnectionMutex);
   return AddToDisconnectionQueue(clientId.first);
}

bool AbstractServer::DisconnectClient(const int socket)
{
   const bool ok = connector->StopClient(socket);
   if (ok)
      AddToDisconnectionQueue(socket);
   return ok;
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

bool AbstractServer::DisconnectClient(ClientMapIt clientIt)
{
   const bool ok = connector->StopClient(clientIt->first);
   if (ok)
      AddToDisconnectionQueue(clientIt);
   return ok;
}

AbstractServer::ClientMapIt AbstractServer::AddToDisconnectionQueue(const int socket)
{
   auto clientIt = connectedClients.find(socket);
   return AddToDisconnectionQueue(clientIt);
}

AbstractServer::ClientMapIt AbstractServer::AddToDisconnectionQueue(ClientMapIt clientIt)
{
   if (clientIt == connectedClients.end())
      return connectedClients.end();

   ClientId client;
   client.socket = clientIt->first;
   client.address = clientIt->second;

   map<int, string>::iterator newIt = connectedClients.erase(clientIt);

   disconnectionQueue.push(client);
   return newIt;
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
      const string clientAddress = disconnectionQueue.front().address;
      disconnectHandler(clientAddress);
      disconnectionQueue.pop();
   }
}
