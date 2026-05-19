#ifndef ABSTRACTCLIENT_H
#define ABSTRACTCLIENT_H

#include "AbstractNetworkAgent.h"

#include <atomic>
#include <functional>
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
 * The client does not disconnect if being destroyed while connected.
 * It is the user's responsibility to check if the client is connected and
 * disconnect it if so. Trying to do it here causes some problems as we would be calling
 * virtual functions inside a destructor.
 */
class AbstractClient : public AbstractNetworkAgent
{
public:
   using ReceivedDataHandler = std::function<void(DataFrame)>;

   AbstractClient(std::unique_ptr<AbstractNetworkConnector> _connector);
   virtual ~AbstractClient() = default;

   bool Connect(const std::string& address, const unsigned int port);
   bool Disconnect();
   bool IsConnected() const;

   bool Send(const DataFrame& buffer);

   void SetHandlers(ConnectionHandler _disconnectHandler, ReceivedDataHandler _receivedHandler);

private:
   void HandleNetworkEvents() override;
   void HandleReceivedData();
   void HandleDisconnection();

   void ProcessActionQueue() override;
   void ProcessQueue();
   void ProcessReceivedData();
   void ProcessDisconnection();

   ConnectionHandler disconnectHandler = nullptr;
   ReceivedDataHandler dataReceivedHandler = nullptr;
   bool connected = false;

   std::queue<DataResult> dataQueue;
   std::atomic<bool> pendingDisconnect = false;
};

#endif // ABSTRACTCLIENT_H
