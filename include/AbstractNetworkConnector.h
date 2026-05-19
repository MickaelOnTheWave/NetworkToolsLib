#ifndef ABSTRACTNETWORKCONNECTOR_H
#define ABSTRACTNETWORKCONNECTOR_H

#include <optional>
#include <stdint.h>
#include <string>
#include <vector>

using DataFrame = std::vector<uint8_t>;

enum class DataStatus { Valid, Disconnect, Error };
struct DataResult
{
   DataStatus status;
   DataFrame data;
};

struct ClientId
{
   int socket;
   std::string address;
};

class AbstractNetworkConnector
{
public:
   virtual bool StartClient(const std::string& ip, const unsigned int port) = 0;
   virtual bool StartServer(const std::string& ip, const unsigned int port) = 0;
   virtual bool Stop() = 0;

   virtual bool Send(const int socket, const DataFrame& buffer) = 0;
   virtual DataResult Receive(const int socket) = 0;

   virtual std::optional<ClientId> GetNewConnection() = 0;

   virtual int GetLocalSocket() const = 0;
};

#endif // ABSTRACTNETWORKCONNECTOR_H
