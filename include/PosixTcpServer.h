#ifndef PosixTcpServer_H
#define PosixTcpServer_H

#include "AbstractServer.h"

/**
 * However, it uses a dual message system to enable variable data buffers :
 *    - First message is the size of the data
 *    - Second message is the data itself
 * This means there is a mechanism to keep track, for each client, whether the next message
 * is a data message or a size message.
*/
class PosixTcpServer : public AbstractServer
{
public:
   PosixTcpServer();
   virtual ~PosixTcpServer();
};

#endif // PosixTcpServer_H
