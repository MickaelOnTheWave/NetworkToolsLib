/*
 * NetworkToolsLib
 * Copyright (C) 2026 Guimarães Tecnologia Ltda
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <catch2/catch_all.hpp>

#include <chrono>

#include "PosixTcpClient.h"
#include "PosixTcpServer.h"

#include <iostream>

using namespace std;
using DataPacket = vector<uint8_t>;

/*******************/

class PosixTcpServerTestFixture
{
public :
   PosixTcpServerTestFixture()
   {
      auto serverConnectionHandler = [this](const string& ip)
      {
         ++serverConnections;
      };
      auto serverDisconnectionHandler = [this](const string& ip)
      {
         ++serverDisconnections;
      };
      auto serverDataHandler = [this](const string& ip, const DataPacket& data)
      {
         serverDataReceived.push_back(data);
      };

      auto clientDisconnectionHandler = [this](const string& ip)
      {
         ++clientDisconnections;
      };
      auto clientDataHandler = [this](const DataPacket& data)
      {
         clientDataReceived.push_back(data);
      };


      server.SetHandlers(serverConnectionHandler, serverDisconnectionHandler, serverDataHandler);
      server.SetWaitTime(std::chrono::duration<double, std::milli>(0));

      client.SetHandlers(clientDisconnectionHandler, clientDataHandler);
   }

protected:
   void SleepFor(const int timeInMs)
   {
      this_thread::sleep_for(chrono::milliseconds(timeInMs));
   }

   const string serverIp = "127.0.0.1";
   unsigned int serverPort = 20000;

   PosixTcpServer server;
   PosixTcpClient client;

   int serverConnections = 0;
   int serverDisconnections = 0;
   vector<DataPacket> serverDataReceived;

   int clientDisconnections = 0;
   vector<DataPacket> clientDataReceived;
};

/*******************/

TEST_CASE_METHOD(PosixTcpServerTestFixture, "Start+Stop doesn't crash")
{
   bool ok = server.Start(serverIp, serverPort);
   REQUIRE( ok == true);

   ok = server.Stop();
   REQUIRE( ok == true);
   CHECK(serverConnections == 0);
   CHECK(serverDisconnections == 0);
   CHECK(serverDataReceived.size() == 0);
}

TEST_CASE_METHOD(PosixTcpServerTestFixture, "Single client connection")
{
   bool ok = server.Start(serverIp, serverPort);
   REQUIRE( ok == true);

   ok = client.Connect(serverIp, serverPort);
   REQUIRE( ok == true);

   const DataPacket packet1 = {0, 1, 2};
   ok = client.Send(packet1);
   REQUIRE( ok == true);

   // To make sure TCP packets don't get concatenated into 1
   SleepFor(1);

   const DataPacket packet2 = {3, 4, 5};
   ok = client.Send(packet2);
   REQUIRE( ok == true);

   ok = client.Disconnect();
   REQUIRE( ok == true);

   ok = server.Stop();
   REQUIRE( ok == true);

   CHECK(serverConnections == 1);
   CHECK(serverDisconnections == 1);
   CHECK(serverDataReceived.size() == 2);
   CHECK(serverDataReceived[0] == packet1);
   CHECK(serverDataReceived[1] == packet2);
}

TEST_CASE_METHOD(PosixTcpServerTestFixture, "Single client connection - Packet concatenation")
{
   serverPort = 10001;
   // To make sure TCP packets get concatenated into 1
   server.SetWaitTime(std::chrono::duration<double, std::milli>(20));

   bool ok = server.Start(serverIp, serverPort);
   REQUIRE( ok == true);

   ok = client.Connect(serverIp, serverPort);
   REQUIRE( ok == true);

   SleepFor(40);

   const DataPacket packet1 = {0, 1, 2};
   ok = client.Send(packet1);
   REQUIRE( ok == true);

   const DataPacket packet2 = {3, 4, 5};
   ok = client.Send(packet2);
   REQUIRE( ok == true);

   ok = client.Disconnect();
   REQUIRE( ok == true);

   SleepFor(40);

   ok = server.Stop();
   REQUIRE( ok == true);

   CHECK(serverConnections == 1);
   CHECK(serverDisconnections == 1);
   REQUIRE(serverDataReceived.size() == 1);

   DataPacket concatenatedPacket = packet1;
   concatenatedPacket.insert(concatenatedPacket.end(), packet2.begin(), packet2.end());
   CHECK(serverDataReceived[0] == concatenatedPacket);
}

TEST_CASE_METHOD(PosixTcpServerTestFixture, "Stop while client connected stops connection gracefully")
{
   serverPort = 10002;
   bool ok = server.Start(serverIp, serverPort);
   REQUIRE( ok == true);

   ok = client.Connect(serverIp, serverPort);
   REQUIRE( ok == true);

   SleepFor(1);

   ok = server.Stop();
   REQUIRE( ok == true);

   CHECK(serverConnections == 1);
   CHECK(serverDisconnections == 0);
   CHECK(serverDataReceived.size() == 0);
}

TEST_CASE_METHOD(PosixTcpServerTestFixture, "Server stops gracefully at destruction")
{
   SKIP();
   serverPort = 10003;
   bool ok = server.Start(serverIp, serverPort);
   REQUIRE( ok == true);

   ok = client.Connect(serverIp, serverPort);
   REQUIRE( ok == true);
}

TEST_CASE_METHOD(PosixTcpServerTestFixture, "Multiple clients")
{
   serverPort = 10004;
   bool ok = server.Start(serverIp, serverPort);
   if (!ok)
      cout << strerror(errno) << endl;
   REQUIRE( ok == true);

   ok = client.Connect(serverIp, serverPort);
   REQUIRE( ok == true);

   SleepFor(1);

   PosixTcpClient client2;
   ok = client2.Connect(serverIp, serverPort);
   REQUIRE( ok == true);

   const DataPacket packet2 = {2, 0, 1, 2};
   ok = client2.Send(packet2);
   REQUIRE( ok == true);

   SleepFor(1);

   const DataPacket packet1 = {1, 4, 5, 6};
   ok = client.Send(packet1);
   REQUIRE( ok == true);

   SleepFor(1);

   PosixTcpClient client3;
   ok = client3.Connect(serverIp, serverPort);
   REQUIRE( ok == true);


   const DataPacket packet1a = {1, 8, 9, 10};
   ok = client.Send(packet1a);
   REQUIRE( ok == true);

   SleepFor(1);

   const DataPacket packet3a = {3, 11, 12, 13};
   ok = client3.Send(packet3a);
   REQUIRE( ok == true);

   SleepFor(1);

   ok = client2.Disconnect();
   REQUIRE( ok == true);

   const DataPacket packet3c = {3, 14, 15, 16};
   ok = client3.Send(packet3c);
   REQUIRE( ok == true);

   SleepFor(1);

   ok = client2.Connect(serverIp, serverPort);
   REQUIRE( ok == true);

   const DataPacket packet2a = {2, 20, 19, 18};
   ok = client2.Send(packet2a);
   REQUIRE( ok == true);

   SleepFor(1);

   ok = server.Stop();
   REQUIRE( ok == true);

   CHECK(serverConnections == 4);
   CHECK(serverDisconnections == 1);
   REQUIRE(serverDataReceived.size() == 6);
   CHECK(serverDataReceived[0] == packet2);
   CHECK(serverDataReceived[1] == packet1);
   CHECK(serverDataReceived[2] == packet1a);
   CHECK(serverDataReceived[3] == packet3a);
   CHECK(serverDataReceived[4] == packet3c);
   CHECK(serverDataReceived[5] == packet2a);
}

TEST_CASE_METHOD(PosixTcpServerTestFixture, "Client can reconnect after disconnection")
{
   serverPort = 10500;
   bool ok = server.Start(serverIp, serverPort);
   REQUIRE( ok == true);

   ok = client.Connect(serverIp, serverPort);
   REQUIRE( ok == true);

   SleepFor(1);

   ok = client.Disconnect();
   REQUIRE( ok == true);

   SleepFor(1);

   CHECK(clientDisconnections == 1);

   ok = client.Connect(serverIp, serverPort);
   REQUIRE( ok == true);

   SleepFor(1);

   ok = client.Disconnect();
   REQUIRE( ok == true);

   SleepFor(1);

   CHECK(clientDisconnections == 2);
}
