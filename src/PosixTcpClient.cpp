#include "PosixTcpClient.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

bool PosixTcpClient::Send(const std::vector<uint8_t>& buffer)
{
   const ssize_t bytesWritten = write(clientSocket, &buffer, buffer.size());
   return (bytesWritten > 0 && bytesWritten == buffer.size());
}

bool PosixTcpClient::StartConnection(const std::string& ip, const unsigned int port)
{
   clientSocket = socket(AF_INET, SOCK_STREAM, 0);
   if (clientSocket < 0)
      return false;

   sockaddr_in serverAddress;
   serverAddress.sin_family = AF_INET;
   serverAddress.sin_port = htons(port);
   serverAddress.sin_addr.s_addr = inet_addr(ip.c_str());

   const int returnValue = connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
   if (returnValue != 0)
      return false;
   return true;
}

bool PosixTcpClient::StopConnection()
{
   close(clientSocket);
   return true;
}

AbstractClient::DataResult PosixTcpClient::GetNewData()
{
   DataResult result;
   vector<uint8_t> buffer;
   buffer.resize(dataBufferSize);

   const ssize_t bytesRead = recv(clientSocket, &buffer, dataBufferSize, MSG_DONTWAIT);
   if (bytesRead < 0)
      result.status = DataStatus::Error;
   else if (bytesRead == 0)
      result.status = DataStatus::Disconnect;
   else
   {
      result.status = DataStatus::Valid;
      result.data = buffer;
   }
   return result;
}
