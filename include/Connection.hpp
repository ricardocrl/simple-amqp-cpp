#pragma once

#include "TcpHandler.hpp"
#include "amqpcpp.h"

#include <boost/asio/io_context.hpp>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace simple_amqpcpp
{

using ConnectionLostCb         = std::function<void()>;
using Channel                  = AMQP::TcpChannel;
using ChannelOperation         = std::function<void(AMQP::TcpChannel&)>;
using ReliableWrapper          = AMQP::Reliable<AMQP::Throttle>;
using ReliableWrapperOperation = std::function<void(ReliableWrapper&)>;

class Connection
{
public:
    static inline const std::string kDefaultExchangeName = "";
    static inline const std::string kDefaultRoutingKey   = "";

    /**
     * @brief Construct a @c Connection object
     * @param address   Address to connect to.
     * @param lostCb    Callback to be called when a connection is lost.
     *
     * Note: when a Connection object is destroyed, the underlying connection
     * is closed, but the @arg lostCb is not called. It is only called when
     * the connection closes outside the destructor.
     */
    Connection(const AMQP::Address& address, const ConnectionLostCb& lostCb);
    ~Connection();

    bool connected() const;
    bool useChannel(const ChannelOperation& channelOperation);
    bool useChannel(const ReliableWrapperOperation& reliableOperation);

private:
    void startEvLoop();
    void stopEvLoop();
    void waitForInit();
    void waitForDetached();
    void registerHandlerCallbacks();
    void onReady();
    void onDetached();
    void onError(const std::string& msg);
    bool makeChannelAvailable();
    bool makeReliableChannelAvailable();

    // connection support variables
    std::thread mEvLoopThread;
    boost::asio::io_context mEvLoop;
    TcpHandler mHandler;
    AMQP::TcpConnection mLibConnection;
    ConnectionLostCb mConnectionLostCb;

    // connection state variables
    mutable std::mutex mStateMutex;
    std::condition_variable mCondVariable;
    bool mConnected;
    bool mError;

    // channel variables
    std::unique_ptr<Channel> mDefaultChannel;
    std::unique_ptr<Channel> mReliableChannel;
    std::unique_ptr<ReliableWrapper> mReliableWrapper;
};

} // namespace simple_amqpcpp
