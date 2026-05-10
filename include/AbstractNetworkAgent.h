#ifndef ABSTRACTNETWORKAGENT_H
#define ABSTRACTNETWORKAGENT_H

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <string>
#include <thread>

/**
 * @brief Base class for Server and Client communication utilities.
 */
class AbstractNetworkAgent
{
public:
   using DataFrame = std::vector<uint8_t>;
   using ConnectionHandler = std::function<void(const std::string&)>;

   AbstractNetworkAgent() = default;
   virtual ~AbstractNetworkAgent();

   void SetWaitTime(std::chrono::duration<double, std::milli> waitTime);

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
      DataFrame data;
   };

   void StartNetworkProcessing();
   void StopNetworkProcessing();

   std::mutex dataMutex;
   std::mutex disconnectionMutex;

protected:
   std::atomic<bool> canStop = true;
   std::shared_ptr<std::thread> receiveThread = nullptr;
   std::shared_ptr<std::thread> processThread = nullptr;

private:
   using ThreadedProcessing = std::function<void(void)>;
   void LaunchThreadedProcessing(std::shared_ptr<std::thread>& thread, ThreadedProcessing processing);

   virtual void HandleNetworkEvents() = 0;
   virtual void ProcessActionQueue() = 0;

   std::chrono::duration<double, std::milli> threadWaitTime = std::chrono::milliseconds(10);
};

#endif // ABSTRACTNETWORKAGENT_H
