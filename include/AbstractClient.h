#ifndef ABSTRACTCLIENT_H
#define ABSTRACTCLIENT_H

#include <atomic>
#include <functional>
#include <mutex>
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
 * The client does not disconnect if being destroyed while connected.
 * It is the user's responsibility to check if the client is connected and
 * disconnect it if so. Trying to do it here causes some problems as we would be calling
 * virtual functions inside a destructor.
 */
class AbstractClient
{
public:
   using ConnectionHandler = std::function<void(const std::string&)>;
   using ReceivedDataHandler = std::function<void(std::vector<uint8_t>)>;

   bool Connect(const std::string& address, const unsigned int port);
   bool Disconnect();
   bool IsConnected() const;

   virtual bool Send(const std::vector<uint8_t>& buffer) = 0;

   void SetHandlers(ConnectionHandler _disconnectHandler, ReceivedDataHandler _receivedHandler);

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
   virtual DataResult GetNewData() = 0;

   void Run();
   void HandleReceivedData();
   void HandleDisconnection();

   void ProcessQueue();
   void ProcessReceivedData();
   void ProcessDisconnection();

   ConnectionHandler disconnectHandler;
   ReceivedDataHandler dataReceivedHandler;
   bool connected = false;

   std::chrono::duration<double, std::milli> threadWaitTime = std::chrono::milliseconds(10);
   std::atomic<bool> canStop = true;
   std::unique_ptr<std::thread> receiveThread;

   std::unique_ptr<std::thread> processThread;

   std::queue<DataResult> dataQueue;
   std::atomic<bool> pendingDisconnect = false;
   std::mutex dataMutex;
   std::mutex disconnectionMutex;
};

#endif // ABSTRACTCLIENT_H
