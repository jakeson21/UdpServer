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

using boost::asio::ip::udp;

int main()
{
    try
    {
        Comm::UdpServer server;
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
