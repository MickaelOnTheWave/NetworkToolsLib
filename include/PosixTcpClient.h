#ifndef POSIXTCPCLIENT_H
#define POSIXTCPCLIENT_H

#include "AbstractClient.h"

class PosixTcpClient : public AbstractClient
{
public:
   bool Send(const std::vector<uint8_t>& buffer) override;

private:
   bool StartConnection(const std::string& ip, const unsigned int port) override;
   bool StopConnection() override;
   DataResult GetNewData() override;

   int clientSocket = -1;
   int dataBufferSize = 1024;
};

#endif // POSIXTCPCLIENT_H
