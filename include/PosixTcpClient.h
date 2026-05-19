#ifndef POSIXTCPCLIENT_H
#define POSIXTCPCLIENT_H

#include "AbstractClient.h"

// TODO Either remove complete this class (too simple)
// or create a single header file with all types of clients/servers.

class PosixTcpClient : public AbstractClient
{
public:
   PosixTcpClient();
};

#endif // POSIXTCPCLIENT_H
