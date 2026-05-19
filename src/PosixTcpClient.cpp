#include "PosixTcpClient.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "PosixTcpConnector.h"

using namespace std;

PosixTcpClient::PosixTcpClient()
   : AbstractClient(make_unique<PosixTcpConnector>())
{
}

PosixTcpClient::~PosixTcpClient()
{
}

bool PosixTcpClient::Send(const DataFrame& buffer)
{
   return connector->Send(connector->GetLocalSocket(), buffer);
}

bool PosixTcpClient::StartConnection(const std::string& ip, const unsigned int port)
{
   return connector->StartClient(ip, port);
}

bool PosixTcpClient::StopConnection()
{
   return connector->Stop();
}

DataResult PosixTcpClient::GetNewData()
{
   return connector->Receive(connector->GetLocalSocket());
}
