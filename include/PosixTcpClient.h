#ifndef POSIXTCPCLIENT_H
#define POSIXTCPCLIENT_H

#include "AbstractClient.h"

class PosixTcpClient : public AbstractClient
{
public:
   PosixTcpClient();
   virtual ~PosixTcpClient();

   bool Send(const DataFrame& buffer) override;

private:
   bool StartConnection(const std::string& ip, const unsigned int port) override;
   bool StopConnection() override;
   DataResult GetNewData() override;
};

#endif // POSIXTCPCLIENT_H
