#include "PosixTcpClient.h"

#include "PosixTcpConnector.h"

PosixTcpClient::PosixTcpClient()
   : AbstractClient(std::make_unique<PosixTcpConnector>())
{
}
