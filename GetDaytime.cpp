//
// client.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <sys/timeb.h>

#include <random>
#include <algorithm>
#include <iterator>
#include <functional>

#include "MessageTypes.h"

#include <tinyxml2.h>
#include <XMLSerialization.h>

using boost::asio::ip::udp;

std::vector<int> generateRandomVector(int inLength = 10)
{
    // First create an instance of an engine.
    std::random_device rnd_device;
    // Specify the engine and distribution.
    mt19937 mersenne_engine(rnd_device());
    uniform_int_distribution<int> dist(1, 52);

    auto gen = std::bind(dist, mersenne_engine);
    std::vector<int> vec(inLength);
    generate(begin(vec), end(vec), gen);
    return vec;
}

class ApplicationSettings: public xmls::Serializable
{
    public:
        ApplicationSettings()
    {
            setClassName("ApplicationSettings");
            Register("Setting1", &Setting1);
            Register("Setting2", &Setting2);
            Register("Setting3", &Setting3);
            Register("Setting4", &Setting4);
            Register("Setting5", &Setting5);
            Register("Setting6", &Setting6);
            Register("Setting7", &Setting7);
            setVersion("2.1");
    }

        xmls::xDouble Setting1;
        xmls::xString Setting2;
        xmls::xString Setting3;
        xmls::xInt Setting4;
        xmls::xInt Setting5;
        xmls::xBool Setting6;
        xmls::xBool Setting7;
};


int main(int argc, char* argv[])
{
    std::vector<int> nums = generateRandomVector(1024);
    std::string ipAddress;
    std::string port = "4001";
    try
    {
        if (argc == 1)
        {
            ipAddress = "127.0.0.1";
        }
        else
        {
            ipAddress = argv[1];
            // std::cerr << "Usage: client <host> <message>" << std::endl;
            // return 1;
        }

        timeb now;
        ftime(&now);
        std::cout << now.time << "." << now.millitm << std::endl;
        std::srand(now.millitm + now.time*1000);

        ApplicationSettings *settings=new ApplicationSettings;
        settings->Setting1=double(std::rand())/double(RAND_MAX);
        settings->Setting2="Settings string 2";
        settings->Setting3="Settings string 3";
        settings->Setting4=1234;
        settings->Setting5=5678;
        settings->Setting6=false;
        settings->Setting7=true;
        // Serialize the Settings object
        cout << "Serializing object... " << endl;
        string xmlData = settings->toXML();
        cout << "OK" << endl << endl;
        cout << "Result:" << endl;
        cout << xmlData << endl << endl;

        boost::asio::io_service io_service;
        udp::resolver resolver(io_service);
        udp::resolver::query query(udp::v4(), ipAddress, port);
        udp::endpoint receiver_endpoint = *resolver.resolve(query);

        udp::socket socket(io_service);
        socket.open(udp::v4());
        std::cout << "Socket open\n";

        //boost::array<char, 1> send_buf;
        //send_buf.assign('a');
        std::string send_buf(xmlData);
        int32_t bytesSent = 0;
        bytesSent += socket.send_to(boost::asio::buffer(send_buf), receiver_endpoint);
        std::cout << bytesSent << " bytes sent\n";

        bytesSent += socket.send_to(boost::asio::buffer(Comm::toString(Comm::float_double)), receiver_endpoint);
        bytesSent += socket.send_to(boost::asio::buffer(nums), receiver_endpoint);
                std::cout << bytesSent << " bytes sent\n";

//        boost::array<char, 1024> recv_buf;
//        udp::endpoint sender_endpoint;
//        size_t len = socket.receive_from(boost::asio::buffer(recv_buf), sender_endpoint);
//        std::cout << "Received: ";
//        std::cout.write(recv_buf.data(), len);

    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

