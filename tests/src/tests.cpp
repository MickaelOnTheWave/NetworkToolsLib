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

      server.SetHandlers(serverConnectionHandler, serverDisconnectionHandler, serverDataHandler);
      server.SetWaitTime(std::chrono::duration<double, std::milli>(0));
   }

protected:
   const string serverIp = "127.0.0.1";
   unsigned int serverPort = 20000;

   PosixTcpServer server;
   PosixTcpClient client;

   int serverConnections = 0;
   int serverDisconnections = 0;
   vector<DataPacket> serverDataReceived;
};

/*******************/

TEST_CASE_METHOD(PosixTcpServerTestFixture, "Start+Stop doesn't crash")
{
   serverPort = 10000;
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
   serverPort = 10001;
   bool ok = server.Start(serverIp, serverPort);
   REQUIRE( ok == true);

   ok = client.Connect(serverIp, serverPort);
   REQUIRE( ok == true);

   const DataPacket packet1 = {0, 1, 2};
   ok = client.Send(packet1);
   REQUIRE( ok == true);

   // To make sure TCP packets don't get concatenated into 1
   this_thread::sleep_for(chrono::milliseconds(1));

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

TEST_CASE_METHOD(PosixTcpServerTestFixture, "Stop while client connected stops connection gracefully")
{
   serverPort = 10002;
   bool ok = server.Start(serverIp, serverPort);

   REQUIRE( ok == true);

   ok = client.Connect(serverIp, serverPort);
   REQUIRE( ok == true);

   ok = server.Stop();
   REQUIRE( ok == true);

   CHECK(serverConnections == 1);
   CHECK(serverDisconnections == 0);
   CHECK(serverDataReceived.size() == 0);
}
