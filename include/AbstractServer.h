#ifndef AbstractServer_H
#define AbstractServer_H

#include "AbstractNetworkAgent.h"

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

// TODO Rename this class to remove "Abstract". It is not abstract anymore,
// and is the final class (maybe use final in inheritance?)

/**
 * @brief Base class for Server/Client communication utilities.
 * Created with socket based communication in mind specifically, but
 * meant to be as generic as it can be useful (so might be extended to other types of
 * Server/Client communications).
 *
 * No message protocol is enforced, it is on the user of those classes to give meaning
 * to the data buffers being exchanged.
 *
 * The class runs two separate threads :
 * - One for polling received data
 * - One for executing external handlers
 *
 * Once new data is available (data received, connects, disconnects),
 * the external handlers data is pushed to a queue, and that queue
 * is processed in another thread.
 * For UI (QT in mind), the handlers need to themselves make sure they
 * execute their code in the main UI thread, but this is external to this class.
 */
class AbstractServer : public AbstractNetworkAgent
{
public:
   using ReceivedDataHandler = std::function<void(const std::string&, DataFrame)>;

   AbstractServer(std::unique_ptr<AbstractNetworkConnector> _connector);
   virtual ~AbstractServer() = default;

   bool Start(const std::string& address, const unsigned int port);
   bool Stop();

   bool Send(const std::string& address, const DataFrame& buffer);

   bool DisconnectClient(const std::string& address);
   bool DisconnectClient(const int socket);

   bool DisconnectAllClients();

   void SetHandlers(ConnectionHandler _connectHandler, ConnectionHandler _disconnectHandler,
                    ReceivedDataHandler _receivedHandler);

private:
   using ClientMapIt = std::map<int, std::string>::iterator;

   void HandleNetworkEvents() override;
   void HandleNewConnections();
   void HandleReceivedData();

   void ProcessActionQueue() override;
   void ProcessNewConnections();
   void ProcessReceivedData();
   void ProcessDisconnections();

   ClientMapIt HandleDisconnection(const std::pair<int, std::string>& clientId);

   int FindClientSocket(const std::string& address) const;

   bool DisconnectClient(ClientMapIt clientIt);
   ClientMapIt AddToDisconnectionQueue(const int socket);
   ClientMapIt AddToDisconnectionQueue(ClientMapIt clientIt);

   std::map<int, std::string> connectedClients;

   ConnectionHandler connectHandler;
   ConnectionHandler disconnectHandler;
   ReceivedDataHandler dataReceivedHandler;
   std::queue<std::pair<ClientId, DataResult>> dataQueue;
   std::queue<ClientId> connectionQueue;
   std::queue<ClientId> disconnectionQueue;
   std::mutex connectionMutex;
};

#endif // AbstractServer_H
