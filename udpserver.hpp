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
#include <vector>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/thread.hpp>
#include <mutex>

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

    class UdpServer : public xmls::Serializable
    {
    public:

        /// Constructor
        UdpServer()
        : mMyRole(ROLE::undefined),
          mSendSocket(),
          mReceiveSocket(),
          mRxIoService(),
          mTxIoService(),
          mThread(),
          mThreadShouldRun(false)
        {
            setClassName("udp_server");
            Register("IPAddressV4", &this->mIPAddressV4, "");
            Register("Port", &this->mPort, "");
        }

        /// Default destructor
        virtual ~UdpServer(){}

        virtual void listen_on(std::string inIpAddress, int inPort)
        {
            PRINT("listen_on()");
            if (mReceiveSocket)
            {
                delete mReceiveSocket;
                mReceiveSocket = 0;
            }
            mIPAddressV4 = inIpAddress;
            mPort = inPort;
            mReceiveSocket = new udp::socket(mRxIoService, boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::from_string(mIPAddressV4.c_str())/*udp::v4()*/, inPort));

            start_receive();

            mMyRole = ROLE::server;
            // std::cout << this->toXML() << std::endl;
        }

        virtual udp::endpoint send_on(std::string inIpAddress, int inPort)
        {
            PRINT("send_on()");
            if (mSendSocket)
            {
                mSendSocket->close();
                delete mSendSocket;
                mSendSocket = 0;
            }
            mIPAddressV4 = inIpAddress;
            mPort = inPort;

            udp::resolver resolver(mTxIoService);
            udp::resolver::query query(udp::v4(), mIPAddressV4.toString(), mPort.toString());
            mSendRemoteEndpoint = *resolver.resolve(query);

            mSendSocket = new udp::socket(mTxIoService);
            mSendSocket->open(udp::v4());

            mMyRole = ROLE::client;

            return mSendRemoteEndpoint;
        }

        virtual void Start()
        {
            mThreadShouldRun = true;
            mThread = boost::thread(&Comm::UdpServer::Run, this);
            mThread.detach();
        }

        virtual void Stop()
        {
            if (mReceiveSocket)
            {
                mReceiveSocket->close();
            }
            mRxIoService.stop();

            mThreadShouldRun = false;// Don't really needed anymore but may be ok
            mThread.interrupt();
            mThread.join();
        }

        virtual void Run()
        {
            PRINT("udp_server::Run()");
            while(mThreadShouldRun)
            {
                //DO Something
                PRINT("Starting mRxIoService");
                try {
                    mRxIoService.run();

                    mRxIoService.reset();

                    //boost::this_thread::sleep_for(boost::chrono::seconds(15));
                } catch(boost::thread_interrupted& interrupt) {
                    PRINT("Caught thread_interrupted");
                }
            }
        }

        virtual void send_to(boost::shared_ptr<std::string> inMessage, udp::endpoint& inRemoteEndpoint )
        {
            PRINT("send_to()");
            mSendSocket->async_send_to(boost::asio::buffer(*inMessage), inRemoteEndpoint,
                    boost::bind(&UdpServer::handle_send, this,
                    inMessage,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred,
                    inRemoteEndpoint));
        }


    public:

        virtual void start_receive()
        {
            PRINT("start_receive()");
            recv_buffer_.fill(0); // clear buffer
            mReceiveSocket->async_receive_from(
                    boost::asio::buffer(recv_buffer_, recv_buffer_.size()),
                    mReceiveRemoteEndpoint,
                    boost::bind(&UdpServer::handle_receive,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
            PRINT("start_receive() completed");
        }

        virtual void handle_receive(const boost::system::error_code& error,
                std::size_t bytes_transferred)
        {
            PRINT("handle_receive()");
            if (!error || error == boost::asio::error::message_size)
            {
                std::vector<char> data(recv_buffer_.begin(), recv_buffer_.end());
                std::cout << std::endl << "=========================================" << std::endl;
                std::cout << "Received " << bytes_transferred << " bytes from " << mReceiveRemoteEndpoint.address().to_string()
                              << ":" << mReceiveRemoteEndpoint.port() << std::endl;

                BOOST_FOREACH(char byte, data)
                {
                    std::cout << (int)byte;
                }
                std::cout << std::endl;

                // Send response message
//                boost::shared_ptr<std::string> message(new std::string(Utils::make_daytime_string()));
//                send_to(message, mRemoteEndpoint);

                start_receive();
            }
        }

        virtual void handle_send(boost::shared_ptr<std::string> /*message*/,
                const boost::system::error_code& /*error*/,
                std::size_t bytes/*bytes_transferred*/,
                const udp::endpoint endpoint)
        {
            PRINT("handle_send()");
            std::cout << "Sent " << bytes << " bytes to " << endpoint.address().to_string() << ":" << endpoint.port() << std::endl;
        }

        bool getThreadShouldRun() { return mThreadShouldRun; }

        xmls::xString mIPAddressV4;
        xmls::xInt mPort;

        ROLE mMyRole;

    public:
        boost::array<char, buffer_size> recv_buffer_;
        udp::socket* mSendSocket;
        udp::socket* mReceiveSocket;
        udp::endpoint mSendRemoteEndpoint;
        udp::endpoint mReceiveRemoteEndpoint;
        boost::asio::io_service mRxIoService;
        boost::asio::io_service mTxIoService;

    private:

        boost::thread mThread;
        bool          mThreadShouldRun;
    };

} /* namespace Comm */

#endif /* UDPSERVER_HPP_ */
