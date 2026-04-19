#ifndef AbstractServer_H
#define AbstractServer_H

#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <stdint.h>
#include <string>
#include <thread>

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
   std::map<int, std::string>::iterator HandleDisconnection(const std::pair<int, std::string>& clientId);

   void ProcessDataQueue();
   void ProcessNewConnections();
   void ProcessReceivedData();
   void ProcessDisconnections();

   std::atomic<bool> canStop = true;
   std::map<int, std::string> connectedClients;
   std::unique_ptr<std::thread> receiveThread;

   ConnectionHandler connectHandler;
   ConnectionHandler disconnectHandler;
   ReceivedDataHandler dataReceivedHandler;
   std::unique_ptr<std::thread> processThread;
   std::queue<std::pair<ClientId, DataResult>> dataQueue;
   std::queue<ClientId> connectionQueue;
   std::queue<ClientId> disconnectionQueue;
   std::mutex dataMutex;
   std::mutex connectionMutex;
   std::mutex disconnectionMutex;
};

#endif // AbstractServer_H
