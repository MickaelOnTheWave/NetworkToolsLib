#include "AbstractClient.h"

using namespace std;

AbstractClient::AbstractClient(std::unique_ptr<AbstractNetworkConnector> _connector)
   : AbstractNetworkAgent(std::move(_connector))
{
}

bool AbstractClient::Connect(const string& address, const unsigned int port)
{
   const bool ok = connector->StartClient(address, port);
   if (ok)
      StartNetworkProcessing();
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

int AbstractClient::GetId() const
{
   return connector->GetLocalSocket();
}

bool AbstractClient::Send(const DataFrame& buffer)
{
   return connector->Send(connector->GetLocalSocket(), buffer);
}

void AbstractClient::SetHandlers(ConnectionHandler _disconnectHandler, ReceivedDataHandler _receivedHandler)
{
   disconnectHandler = _disconnectHandler;
   dataReceivedHandler = _receivedHandler;
}

void AbstractClient::HandleNetworkEvents()
{
   HandleReceivedData();
}

void AbstractClient::HandleReceivedData()
{
   lock_guard<mutex> lock(dataMutex);
   const DataResult result = connector->Receive(connector->GetLocalSocket());
   if (result.status == DataStatus::Valid)
      dataQueue.push(result);
   else if (result.status == DataStatus::Disconnect)
      pendingDisconnect = true;
}

void AbstractClient::ProcessActionQueue()
{
   ProcessReceivedData();
   ProcessDisconnection();
}

void AbstractClient::ProcessReceivedData()
{
   if (!dataQueue.empty())
   {
      lock_guard<mutex> lock(dataMutex);
      const auto& data = dataQueue.front();
      if (dataReceivedHandler)
         dataReceivedHandler(data.data);
      dataQueue.pop();
   }
}

void AbstractClient::ProcessDisconnection()
{
   if (pendingDisconnect)
   {
      lock_guard<mutex> lock(disconnectionMutex);
      const bool ok = connector->Stop();
      if (ok)
      {
         canStop = true;
         if (disconnectHandler)
            disconnectHandler("server");
      }
      pendingDisconnect = false;
   }
}
