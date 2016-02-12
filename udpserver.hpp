/*
 * udpserver.h
 *
 *  Created on: Feb 9, 2016
 *      Author: fuguru
 */

#ifndef UDPSERVER_HPP_
#define UDPSERVER_HPP_

#include <ctime>
#include <iostream>
#include <iomanip>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include "MessageTypes.h"
#include "daytimestring.hpp"

#include "tinyxml2.h"
#include "XMLSerialization.h"

#define PRINT(X) std::cout << __FILE__ << ":" << __LINE__ << ": " << (X) << std::endl;

using boost::asio::ip::udp;

namespace Comm
{
    static const int buffer_size = 1024;

    enum ROLE{
        undefined,
        server,
        client
    };

    class udp_server : public xmls::Serializable
    {
    public:

        /// Constructor
        udp_server()
        : mIoService(),
          mSocket(),
          mMyRole(ROLE::undefined)

        {
            setClassName("udp_server");
            Register("IPAddressV4", &this->mIPAddressV4, "");
            Register("Port", &this->mPort, "");
            mIoService = new boost::asio::io_service;
        }

        /// Default destructor
        virtual ~udp_server(){}

        virtual void listen_on(std::string inIpAddress, int inPort)
        {
            PRINT("listen_on()");
            if (mSocket)
            {
                delete mSocket;
                mSocket = 0;
            }
            mIPAddressV4 = inIpAddress;
            mPort = inPort;
            mSocket = new udp::socket(*mIoService, boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::from_string(mIPAddressV4.c_str())/*udp::v4()*/, inPort));

            start_receive();

            mMyRole = ROLE::server;

            std::cout << this->toXML() << std::endl;
        }

        virtual void send_on(std::string inIpAddress, int inPort)
        {
            PRINT("send_on()");
            if (mSocket)
            {
                delete mSocket;
                mSocket = 0;
            }
            mIPAddressV4 = inIpAddress;
            mPort = inPort;

            udp::resolver resolver(*mIoService);
            udp::resolver::query query(udp::v4(), mIPAddressV4.toString(), mPort.toString());
            mRemoteEndpoint = *resolver.resolve(query);

            mSocket = new udp::socket(*mIoService);
            mSocket->open(udp::v4());

            mMyRole = ROLE::client;
        }

        virtual void run()
        {
            mIoService->run();
        }

        virtual void stop()
        {
            mIoService->stop();
        }

        virtual void send_to(boost::shared_ptr<std::string> inMessage, udp::endpoint& inRemoteEndpoint ){
            PRINT("send_to()");
            mSocket->async_send_to(boost::asio::buffer(*inMessage), inRemoteEndpoint,
                    boost::bind(&udp_server::handle_send, this,
                    inMessage,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred,
                    inRemoteEndpoint));
        }


    protected:

        virtual void start_receive()
        {
            PRINT("start_receive()");
            recv_buffer_.fill(0); // clear buffer
            mSocket->async_receive_from(
                    boost::asio::buffer(recv_buffer_, recv_buffer_.size()),
                    mRemoteEndpoint,
                    boost::bind(&udp_server::handle_receive,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        }

        virtual void handle_receive(const boost::system::error_code& error,
                std::size_t bytes_transferred)
        {
            PRINT("handle_receive()");
            if (!error || error == boost::asio::error::message_size)
            {
                std::vector<char> data(recv_buffer_.begin(), recv_buffer_.end());
                std::cout << std::endl << "=========================================" << std::endl;
                std::cout << "Received " << bytes_transferred << " bytes from " << mRemoteEndpoint.address().to_string()
                              << ":" << mRemoteEndpoint.port() << std::endl;

                BOOST_FOREACH(char byte, data)
                {
                    std::cout << (int)byte;
                }
                std::cout << std::endl;

                // Send response message
                boost::shared_ptr<std::string> message(new std::string(Utils::make_daytime_string()));
                send_to(message, mRemoteEndpoint);

                start_receive();
            }
        }

        virtual void handle_send(boost::shared_ptr<std::string> message/*message*/,
                const boost::system::error_code& error/*error*/,
                std::size_t bytes/*bytes_transferred*/,
                const udp::endpoint endpoint)
        {
            PRINT("handle_send()");
            std::cout << "Sent " << bytes << " bytes to " << endpoint.address().to_string() << ":" << endpoint.port() << std::endl;
        }

        xmls::xString mIPAddressV4;
        xmls::xInt mPort;

        boost::asio::io_service* mIoService;
        udp::socket* mSocket;
        udp::endpoint mRemoteEndpoint;
        boost::array<char, buffer_size> recv_buffer_;

        ROLE mMyRole;
    };

} /* namespace Comm */

#endif /* UDPSERVER_HPP_ */
