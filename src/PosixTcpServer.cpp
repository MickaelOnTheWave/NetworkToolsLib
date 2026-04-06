#include "PosixTcpServer.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

bool PosixTcpServer::StartConnection(const std::string& ip, const unsigned int port)
{
   serverSocket = socket(AF_INET, SOCK_STREAM, 0);
   if (serverSocket < 0)
      return false;

   sockaddr_in serverAddress;
   serverAddress.sin_family = AF_INET;
   serverAddress.sin_port = htons(port);
   serverAddress.sin_addr.s_addr = inet_addr(ip.c_str());//INADDR_ANY;

   const int bindValue = bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
   if (bindValue < 0)
      return false;

   // Set connections acceptance to non-blocking mode.
   const int flags = fcntl(serverSocket, F_GETFL, 0);
   fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);

   const int listenValue = listen(serverSocket, maxConnections);
   if (listenValue < 0)
      return false;
   return true;
}

bool PosixTcpServer::StopConnection()
{
   const int returnValue = close(serverSocket);
   return (returnValue >= 0);
}

std::optional<AbstractServer::ClientId> PosixTcpServer::GetNewConnection()
{
   sockaddr clientAddress;
   socklen_t addressLength = sizeof(clientAddress);
   const int clientSocket = accept(serverSocket, &clientAddress, &addressLength);

   if (clientSocket < 0)
      return nullopt;

   std::string address = inet_ntoa(((struct sockaddr_in*)&clientAddress)->sin_addr);
   address += ":" + std::to_string(ntohs(((struct sockaddr_in*)&clientAddress)->sin_port));

   ClientId clientId;
   clientId.socket = clientSocket;
   clientId.address = address;
   return make_optional(clientId);
}

AbstractServer::DataResult PosixTcpServer::GetNewData(const int clientSocket)
{
   DataResult result;
   vector<uint8_t> buffer;
   buffer.resize(dataBufferSize);
   const ssize_t receiveCode = recv(clientSocket, &buffer, dataBufferSize, 0);

   if (receiveCode < 0)
      result.status = DataStatus::Error;
   else if (receiveCode == 0)
      result.status = DataStatus::Disconnect;
   else
   {
      result.status = DataStatus::Valid;
      result.data = buffer;
   }

   return result;
}
