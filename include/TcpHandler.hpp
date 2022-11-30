#pragma once

#include "amqpcpp.h"
#include "amqpcpp/libboostasio.h"

#include <chrono>
#include <functional>
#include <list>

namespace simple_amqpcpp
{

class TcpHandler : public AMQP::LibBoostAsioHandler
{
public:
    using OnErrorCb    = std::function<void(AMQP::TcpConnection*, const std::string& msg)>;
    using OnReadyCb    = std::function<void(AMQP::TcpConnection*)>;
    using OnClosedCb   = std::function<void(AMQP::TcpConnection*)>;
    using OnLostCb     = std::function<void(AMQP::TcpConnection*)>;
    using OnDetachedCb = std::function<void(AMQP::TcpConnection*)>;

    TcpHandler(boost::asio::io_context& evLoop, const std::chrono::seconds& heartbeatIntervalSec);

    void runOnError(const OnErrorCb& cb);
    void runOnReady(const OnReadyCb& cb);
    void runOnClosed(const OnClosedCb& cb);
    void runOnLost(const OnLostCb& cb);
    void runOnDetached(const OnDetachedCb& cb);

private:
    void onError(AMQP::TcpConnection* connection, const char* msg) override;
    void onReady(AMQP::TcpConnection* connection) override;
    void onClosed(AMQP::TcpConnection* connection) override;
    void onLost(AMQP::TcpConnection* connection) override;
    void onDetached(AMQP::TcpConnection* connection) override;
    uint16_t onNegotiate(AMQP::TcpConnection* connection, uint16_t timeout) override;

    std::chrono::seconds mHeartbeatIntervalSec;
    std::list<OnErrorCb> mOnErrorCbs;
    std::list<OnReadyCb> mOnReadyCbs;
    std::list<OnClosedCb> mOnClosedCbs;
    std::list<OnLostCb> mOnLostCbs;
    std::list<OnDetachedCb> mOnDetachedCbs;
};

} // namespace simple_amqp
