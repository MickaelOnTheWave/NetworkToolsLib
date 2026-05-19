#include "PosixTcpServer.h"

#include "PosixTcpConnector.h"

PosixTcpServer::PosixTcpServer()
   : AbstractServer(std::make_unique<PosixTcpConnector>())
{
}
