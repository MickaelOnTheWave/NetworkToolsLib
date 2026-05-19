#ifndef POSIXTCPCONNECTOR_H
#define POSIXTCPCONNECTOR_H

#include "AbstractNetworkConnector.h"

#include <netinet/in.h>

class PosixTcpConnector : public AbstractNetworkConnector
{
public:
   virtual ~PosixTcpConnector();

   bool StartClient(const std::string& ip, const unsigned int port) override;
   bool StartServer(const std::string& ip, const unsigned int port) override;
   bool Stop() override;

   bool Send(const int socket, const DataFrame& buffer) override;
   DataResult Receive(const int socket) override;

   std::optional<ClientId> GetNewConnection() override;

   int GetLocalSocket() const override;

private:
   sockaddr_in CreateSocketAddress(const std::string& ip, const unsigned int port);

   int socketId = -1;
   int maxServerConnections = 10;
   int dataBufferSize = 1024;

};

#endif // POSIXTCPCONNECTOR_H
