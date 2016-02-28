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

#include "../GenericDelegate/GenericDelegate.h"

#define PRINT(X) std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << ": " << (X) << std::endl;

using boost::asio::ip::udp;

namespace Comm
{

    // this typedef should match to function signature of what it will be used to call. This could be a different class then the one performing the callback.
    typedef Help::GenericDelegate<const boost::shared_ptr<std::vector<uint8_t> >&, const boost::shared_ptr<udp::endpoint>& > ReceiveCallbackSignature;

    static const int buffer_size = 4096;

    enum ROLE{
        UNCONFIGURED,
        SERVER,
        CLIENT,
        CLIENTSERVER
    };

    class UdpServer : public xmls::Serializable
    {
    public:

        UdpServer()
        : mMyRole(ROLE::UNCONFIGURED),
          mSendSocket(),
          mReceiveSocket(),
          mIoService(),
          mHandleReceiveCallback(),
          mThread(),
          mThreadShouldRun(false),
          mReceiveCallbackIsDefined(false)
        {
            setClassName("udp_server");
            Register("IPAddressV4", &this->mIPAddressV4, "");
            Register("Port", &this->mPort, "");
        }


        /// Constructor
        UdpServer(ReceiveCallbackSignature inReceiveHandler)
        : mMyRole(ROLE::UNCONFIGURED),
          mSendSocket(),
          mReceiveSocket(),
          mIoService(),
          mHandleReceiveCallback(inReceiveHandler),
          mThread(),
          mThreadShouldRun(false),
          mReceiveCallbackIsDefined(true)
        {
            setClassName("udp_server");
            Register("IPAddressV4", &this->mIPAddressV4, "");
            Register("Port", &this->mPort, "");
        }

        /// Default destructor
        virtual ~UdpServer(){
            mThreadShouldRun = false;
            mIoService.stop();
        }

        virtual void listen_on(std::string inIpAddress, int inPort)
        {
            PRINT("Attempting to open socket");

            if (mReceiveSocket)
            {
                PRINT("Removing existing socket");
                if (mReceiveSocket->is_open()) mReceiveSocket->close();
                delete mReceiveSocket;
                mReceiveSocket = 0;
            }
            mIPAddressV4 = inIpAddress;
            mPort = inPort;
            mReceiveSocket = new udp::socket(mIoService, boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::from_string(mIPAddressV4.c_str())/*udp::v4()*/, inPort));

            mMyRole = ROLE::SERVER;
        }

        virtual udp::endpoint send_on(std::string inIpAddress, int inPort)
        {
            PRINT("");
            if (mSendSocket)
            {
                mSendSocket->close();
                delete mSendSocket;
                mSendSocket = 0;
            }
            mIPAddressV4 = inIpAddress;
            mPort = inPort;

            boost::system::error_code ec;
            udp::resolver resolver(mIoService);
            udp::resolver::query query(udp::v4(), mIPAddressV4.toString(), mPort.toString());
            mSendRemoteEndpoint = *resolver.resolve(query, ec);

//            udp::resolver::iterator iterator = resolver.resolve(query, ec);
            if (ec)
            {
                // An error occurred.
                std::cout << "Caught error " << ec.value() << " " << ec.message() << std::endl;
                delete mSendSocket;
                mSendSocket = 0;
                return udp::endpoint();
            }
//            mSendRemoteEndpoint = iterator->endpoint();
            std::cout << mSendRemoteEndpoint.address().to_string() << mSendRemoteEndpoint.port() << std::endl;

            mSendSocket = new udp::socket(mIoService);
//            mSendSocket->open(udp::v4());
            mSendSocket->open(udp::v4(), ec);
            if (ec)
            {
                // An error occurred.
                std::cout << "Caught error " << ec.value() << " " << ec.message() << std::endl;
                delete mSendSocket;
                mSendSocket = 0;
            }
            mMyRole = ROLE::CLIENT;

            return mSendRemoteEndpoint;
        }

        virtual void Start()
        {
            mThreadShouldRun = true;
            start_receive();
            boost::thread t(bind(&Comm::UdpServer::Run, this));
            mThread.swap(t);

//            start_receive();
        }

        virtual void Stop()
        {
            if (mReceiveSocket && mReceiveSocket->is_open())
            {
                mReceiveSocket->close();
            }
            mIoService.stop();

            mThreadShouldRun = false;// Don't really needed anymore but may be ok
            mThread.interrupt();
            mThread.join();
        }

        virtual void send_to(boost::shared_ptr<std::string> inMessage, udp::endpoint& inRemoteEndpoint )
        {
            std::cout << "Sending to " << inRemoteEndpoint.address().to_string() << ":" << inRemoteEndpoint.port() << std::endl;
            mSendSocket->async_send_to(boost::asio::buffer(*inMessage), inRemoteEndpoint,
                    boost::bind(&UdpServer::handle_send, this,
                    inMessage,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred,
                    inRemoteEndpoint));
        }

        bool getThreadShouldRun() { return mThreadShouldRun; }

    private:

        virtual void Run()
        {
            while(mThreadShouldRun)
            {
                //DO Something
                PRINT("Starting mIoService thread");
                try {
                    mIoService.run();

                    mIoService.reset();

                } catch(boost::thread_interrupted& interrupt) {
                    PRINT("Caught thread_interrupted");
                }
            }
        }

        void start_receive()
        {
            PRINT("starting");
            recv_buffer_.fill(0); // clear buffer

            mReceiveSocket->async_receive_from(
                    boost::asio::buffer(recv_buffer_, recv_buffer_.size()),
                    mReceiveRemoteEndpoint,
                    boost::bind(&UdpServer::handle_receive,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));

            PRINT("completed");
        }

        void handle_receive(const boost::system::error_code& inError,
                std::size_t inBytes_transferred)
        {
            PRINT("Test for error");
            if (!inError || inError == boost::asio::error::message_size)
            {
                PRINT("UDP packet received");

                std::vector<uint8_t> data(recv_buffer_.begin(), recv_buffer_.end());

                std::cout << "Received " << inBytes_transferred << " bytes from " << mReceiveRemoteEndpoint.address().to_string() << ":" << mReceiveRemoteEndpoint.port() << std::endl;

                // process the data & notify whoever is interested
                udp::endpoint ep(mReceiveRemoteEndpoint);
                notify_packet_handler(data, ep);

                start_receive();
            }
            else if (inError == boost::asio::error::operation_aborted)
            {
                PRINT("Receive operation aborted");
            }
            else
            {
                PRINT("UNHANDLED ERROR");
            }
        }

        ///Notify all interested parties of a new packet received.
        ///
        /// \param inData the data that was received from someone
        /// \param inEndpoint the network details of the other party
        ///
        void notify_packet_handler(const std::vector<uint8_t> inData,
                                   const udp::endpoint inEndpoint)
        {
            PRINT("Notification received");
            boost::shared_ptr<std::vector<uint8_t> > aData(new std::vector<uint8_t>(inData));
            boost::shared_ptr<udp::endpoint> aEndPoint(new udp::endpoint(inEndpoint));

            if (mReceiveCallbackIsDefined)
            {
                PRINT("calling receive callback handler");
                mHandleReceiveCallback(aData, aEndPoint);
            }
            else
            {
                PRINT("No receive callback handler defined");
                for (unsigned int n=0; n < inData.size(); n++)
                {
                    std::cout << inData[n];
                }
                std::cout << std::endl;
            }
        }

        virtual void handle_send(boost::shared_ptr<std::string> /*message*/,
                const boost::system::error_code& /*error*/,
                std::size_t bytes/*bytes_transferred*/,
                const udp::endpoint endpoint)
        {
            PRINT("sending message");
            std::cout << "Sent " << bytes << " bytes to " << endpoint.address().to_string() << ":" << endpoint.port() << std::endl;
        }

        xmls::xString mIPAddressV4;
        xmls::xInt mPort;

        ROLE mMyRole;


    private:
        boost::array<char, buffer_size> recv_buffer_;
        udp::socket* mSendSocket;
        udp::socket* mReceiveSocket;
        udp::endpoint mSendRemoteEndpoint;
        udp::endpoint mReceiveRemoteEndpoint;
        boost::asio::io_service mIoService;
        ReceiveCallbackSignature mHandleReceiveCallback;

        boost::thread mThread;
        bool          mThreadShouldRun;
        bool          mReceiveCallbackIsDefined;
    };

} /* namespace Comm */

#endif /* UDPSERVER_HPP_ */
