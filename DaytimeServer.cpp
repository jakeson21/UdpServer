//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include "MessageTypes.h"

#include <tinyxml2.h>
#include <XMLSerialization.h>
#include "daytimestring.hpp"
#include "udpserver.hpp"
#include "../GenericDelegate/GenericDelegate.h"

using boost::asio::ip::udp;

class ReceiveHandler
{
    public:
        ReceiveHandler(){};
        virtual ~ReceiveHandler(){};

        void handle_receive(const boost::shared_ptr<std::vector<uint8_t> >& inData,
                                       const boost::shared_ptr<udp::endpoint>& inEndpoint)
        {
            std::string receivedData;
            BOOST_FOREACH(uint8_t byte, *inData)
            {
                receivedData.append(std::string(1, char(byte)));
            }
            std::cout << "Received from address=" << inEndpoint->address().to_string() << ":" << inEndpoint->port() << std::endl;
            std::cout << receivedData << std::endl << std::endl;
        }
};

int main()
{
    ReceiveHandler myHandler;
    Comm::ReceiveCallbackSignature mMethodCallback(&myHandler, &ReceiveHandler::handle_receive);
    Comm::UdpServer server(mMethodCallback);

    try
    {
        server.listen_on("127.0.0.1", 4001);
        server.Start();
        while (server.getThreadShouldRun())
        {
            sleep(1);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

