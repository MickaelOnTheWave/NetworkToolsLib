#include "PosixTcpServer.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "PosixTcpConnector.h"

using namespace std;

PosixTcpServer::PosixTcpServer()
   : AbstractServer(make_unique<PosixTcpConnector>())
{
}

PosixTcpServer::~PosixTcpServer()
{
}
