#include "PosixTcpConnector.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;


PosixTcpConnector::~PosixTcpConnector()
{
   if (socketId >= 0)
      close(socketId);
}

bool PosixTcpConnector::StartClient(const string& ip, const unsigned int port)
{
   socketId = socket(AF_INET, SOCK_STREAM, 0);
   if (socketId < 0)
      return false;

   sockaddr_in serverAddress = CreateSocketAddress(ip, port);

   const int returnValue = connect(socketId, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
   if (returnValue != 0)
      return false;
   return true;
}

bool PosixTcpConnector::StartServer(const string& ip, const unsigned int port)
{
   socketId = socket(AF_INET, SOCK_STREAM, 0);
   if (socketId < 0)
      return false;

   sockaddr_in serverAddress = CreateSocketAddress(ip, port);

   const int bindValue = bind(socketId, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
   if (bindValue < 0)
      return false;

   // Set connections acceptance to non-blocking mode.
   const int flags = fcntl(socketId, F_GETFL, 0);
   fcntl(socketId, F_SETFL, flags | O_NONBLOCK);

   const int listenValue = listen(socketId, maxServerConnections);
   if (listenValue < 0)
      return false;
   return true;
}

bool PosixTcpConnector::Stop()
{
   return StopClient(socketId);
}

bool PosixTcpConnector::StopClient(const int socket)
{
   const int returnValue = close(socket);
   return (returnValue >= 0);
}

bool PosixTcpConnector::Send(const int socket, const DataFrame& buffer)
{
   const ssize_t bytesWritten = write(socket, buffer.data(), buffer.size());
   return (bytesWritten > 0 && bytesWritten == buffer.size());
}

DataResult PosixTcpConnector::Receive(const int socket)
{
   DataResult result;
   char* buffer = new char[dataBufferSize];

   const ssize_t bytesRead = recv(socket, buffer, dataBufferSize, MSG_DONTWAIT);
   if (bytesRead < 0)
      result.status = DataStatus::Error;
   else if (bytesRead == 0)
      result.status = DataStatus::Disconnect;
   else
   {
      result.status = DataStatus::Valid;
      result.data.resize(bytesRead);
      for (int i=0; i<bytesRead; ++i)
         result.data[i] = buffer[i];
   }
   delete [] buffer;
   return result;
}

std::optional<ClientId> PosixTcpConnector::GetNewConnection()
{
   sockaddr clientAddress;
   socklen_t addressLength = sizeof(clientAddress);
   const int clientSocket = accept(socketId, &clientAddress, &addressLength);

   if (clientSocket < 0)
      return nullopt;

   string address = inet_ntoa(((struct sockaddr_in*)&clientAddress)->sin_addr);
   address += ":" + to_string(ntohs(((struct sockaddr_in*)&clientAddress)->sin_port));

   ClientId clientId;
   clientId.socket = clientSocket;
   clientId.address = address;
   return make_optional(clientId);
}

int PosixTcpConnector::GetLocalSocket() const
{
   return socketId;
}

sockaddr_in PosixTcpConnector::CreateSocketAddress(const string& ip, const unsigned int port)
{
   sockaddr_in address;
   address.sin_family = AF_INET;
   address.sin_port = htons(port);
   address.sin_addr.s_addr = inet_addr(ip.c_str());
   return address;
}
