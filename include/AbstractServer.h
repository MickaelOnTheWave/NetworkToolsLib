#ifndef AbstractServer_H
#define AbstractServer_H

#include <atomic>
#include <functional>
#include <optional>
#include <stdint.h>
#include <string>
#include <thread>
#include <map>

/**
 * @brief Base class for Server/Client communication utilities.
 * Created with socket based communication in mind specifically, but
 * meant to be as generic as it can be useful (so might be extended to other types of
 * Server/Client communications).
 *
 * No message protocol is enforced, it is on the user of those classes to give meaning
 * to the data buffers being exchanged.
 */
class AbstractServer
{
public:
   using ConnectionHandler = std::function<void(const std::string&)>;
   using ReceivedDataHandler = std::function<void(const std::string&, std::vector<uint8_t>)>;

   bool Start(const std::string& address, const unsigned int port);
   bool Stop();

   void SetHandlers(ConnectionHandler _connectHandler, ConnectionHandler _disconnectHandler,
                    ReceivedDataHandler _receivedHandler);

protected:
   struct ClientId
   {
      int socket;
      std::string address;
   };

   enum class DataStatus { Valid, Disconnect, Error };
   struct DataResult
   {
      DataStatus status;
      std::vector<uint8_t> data;
   };

private:
   virtual bool StartConnection(const std::string& ip, const unsigned int port) = 0;
   virtual bool StopConnection() = 0;
   virtual std::optional<ClientId> GetNewConnection() = 0;
   virtual DataResult GetNewData(const int clientSocket) = 0;

   void Run();
   void HandleNewConnections();
   void HandleReceivedData();
   void HandleDisconnection(const std::pair<int, std::string>& clientId);

   ConnectionHandler connectHandler;
   ConnectionHandler disconnectHandler;
   ReceivedDataHandler dataReceivedHandler;
   std::atomic<bool> canStop = true;
   std::map<int, std::string> connectedClients;
   std::unique_ptr<std::thread> serverThread;
};

#endif // AbstractServer_H
