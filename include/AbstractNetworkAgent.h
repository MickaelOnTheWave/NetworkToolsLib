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

#include "AbstractNetworkConnector.h"

/**
 * @brief Base class for Server and Client communication utilities.
 */
class AbstractNetworkAgent
{
public:
   using DataFrame = std::vector<uint8_t>;
   using ConnectionHandler = std::function<void(const std::string&)>;

   AbstractNetworkAgent(std::unique_ptr<AbstractNetworkConnector> _connector);
   virtual ~AbstractNetworkAgent();

   void SetWaitTime(std::chrono::duration<double, std::milli> waitTime);

protected:

   void StartNetworkProcessing();
   void StopNetworkProcessing();

   std::mutex dataMutex;
   std::mutex disconnectionMutex;

protected:
   std::unique_ptr<AbstractNetworkConnector> connector = nullptr;
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
