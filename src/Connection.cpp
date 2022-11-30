#include "Connection.hpp"

#include "ThreadUtils.hpp"
#include "amqpcpp/linux_tcp.h"
#include "amqpcpp/reliable.h"

#include <chrono>

namespace
{

const auto kInitTimeout              = std::chrono::seconds(10);
const auto kDisconnectTimeout        = std::chrono::seconds(2);
const auto kExecuteTimeout           = std::chrono::seconds(2);
const auto kDefaultHeartbeatInterval = std::chrono::seconds(15);
const auto kReliableThrottleLimit    = 20;

} // namespace

namespace simple_amqpcpp
{

Connection::Connection(const AMQP::Address& address, const ConnectionLostCb& lostCb)
    : mHandler{mEvLoop, kDefaultHeartbeatInterval}
    , mLibConnection{&mHandler, address}
    , mConnectionLostCb{lostCb}
    , mConnected{false}
    , mError{false}
{
    registerHandlerCallbacks();

    startEvLoop();
    waitForInit(); // the goal is to have an already established connection after
                   // construction
}

Connection::~Connection()
{
    // in case we need to close() in the lines below, it could trigger this callback,
    // but by design we don't call this callback when destroying the connection explicitly.
    mConnectionLostCb = nullptr;

    if (connected())
    {
        // close connection
        mEvLoop.post([this] {
            if (mLibConnection.usable() && mLibConnection.initialized() && !mLibConnection.closed())
            {
                mLibConnection.close();
            }
        });

        // wait until closed
        std::unique_lock lk{mStateMutex};
        mCondVariable.wait_for(lk, kDisconnectTimeout, [this] { return !mConnected; });
    }

    stopEvLoop();
}

bool Connection::connected() const
{
    std::scoped_lock lk{mStateMutex};
    return mConnected && !mError;
}

void Connection::waitForInit()
{
    std::unique_lock<std::mutex> lk{mStateMutex};
    mCondVariable.wait_for(lk, kInitTimeout, [this] { return mConnected || mError; });
}

void Connection::registerHandlerCallbacks()
{
    mHandler.runOnReady([this](auto*) { onReady(); });
    mHandler.runOnDetached([this](auto*) { onDetached(); });
    mHandler.runOnError([this](auto*, const std::string& msg) { onError(msg); });
}

void Connection::onError(const std::string& msg)
{
    {
        std::scoped_lock lk{mStateMutex};
        mError = true;
    }
    mCondVariable.notify_all();
}

void Connection::onReady()
{
    {
        std::scoped_lock lk{mStateMutex};
        mConnected = true;
    }
    mCondVariable.notify_all();
}

void Connection::onDetached()
{
    {
        std::scoped_lock lk{mStateMutex};
        if (!mConnected)
        {
            return;
        }
        mConnected = false;
    }

    // this callback is reset in the destructor, to account for explicit disconnections
    if (mConnectionLostCb)
    {
        std::thread(mConnectionLostCb).detach();
    }

    // Note: this must be the last call, because it may unblock the destructor code
    // and it's not safe to access members from now.
    mCondVariable.notify_all();
}

void Connection::startEvLoop()
{
    mEvLoopThread = std::thread([&] { mEvLoop.run(); });
}

void Connection::stopEvLoop()
{
    mEvLoop.stop();
    mEvLoopThread.join();
}

bool Connection::useChannel(const ChannelOperation& channelOperation)
{
    // here we sync with the time point when we know if there is a channel
    // available
    ThreadUtils::SimpleEvent channelAvailEvent;
    std::optional<bool> channelAvail;

    mEvLoop.post([&, channelOperation] {
        channelAvail = makeChannelAvailable();
        channelAvailEvent.notify();

        // we can in practice loose the channel after this check in this case we
        // will skip this if-case, returning true, but shortly after, the caller
        // will get the onError handler invoked, because we will also post().
        if (!channelAvail)
        {
            std::cerr << "Failed to get a channel" << std::endl;
            return;
        }

        channelOperation(*mDefaultChannel);
    });

    channelAvailEvent.waitFor(kExecuteTimeout);

    return channelAvail && *channelAvail == true;
}

bool Connection::useChannel(const ReliableWrapperOperation& reliableOperation)
{
    // here we sync with the time point when we know if there is a channel
    // available
    ThreadUtils::SimpleEvent channelAvailEvent;
    std::optional<bool> channelAvail;

    mEvLoop.post([&, reliableOperation] {
        channelAvail = makeReliableChannelAvailable();
        channelAvailEvent.notify();

        // we can in practice loose the channel after this check in this case we
        // will skip this if-case, returning true, but shortly after, the caller
        // will get the onError handler invoked, because we will also post().
        if (!channelAvail)
        {
            std::cerr << "Failed to get a reliable channel" << std::endl;
            return;
        }

        reliableOperation(*mReliableWrapper);
    });

    channelAvailEvent.waitFor(kExecuteTimeout);

    return channelAvail && *channelAvail == true;
}

bool Connection::makeChannelAvailable()
{
    if (mDefaultChannel && mDefaultChannel->usable())
    {
        return true;
    }

    if (!mLibConnection.usable())
    {
        std::cerr << "Cannot create channel, connection not usable" << std::endl;
        return false;
    }

    mDefaultChannel = std::make_unique<AMQP::TcpChannel>(&mLibConnection);
    mDefaultChannel->onError([this](auto* msg) { std::cerr << "Channel error: " << msg << std::endl; });

    if (!mDefaultChannel->usable())
    {
        std::cerr << "Channel not usable" << std::endl;
        return false;
    }

    return true;
}

bool Connection::makeReliableChannelAvailable()
{
    if (mReliableChannel && mReliableChannel->usable())
    {
        return true;
    }
    if (!mLibConnection.usable())
    {
        std::cerr << "Cannot create reliable channel, connection not usable" << std::endl;
        return false;
    }

    mReliableChannel = std::make_unique<AMQP::TcpChannel>(&mLibConnection);
    mReliableWrapper = std::make_unique<AMQP::Reliable<AMQP::Throttle>>(*mReliableChannel, kReliableThrottleLimit);

    if (!mReliableChannel->usable())
    {
        std::cerr << "Reliable channel not usable" << std::endl;
        return false;
    }

    return true;
}

} // namespace simple_amqpcpp
