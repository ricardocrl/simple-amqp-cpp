#include "TcpHandler.hpp"

namespace
{
const auto kHeartbeatTimeoutToIntervalRatio = 2;
}

namespace simple_amqpcpp
{

TcpHandler::TcpHandler(boost::asio::io_context& context, const std::chrono::seconds& heartbeatIntervalSec)
    : AMQP::LibBoostAsioHandler(context)
    , mHeartbeatIntervalSec{heartbeatIntervalSec}
{
}

void TcpHandler::runOnError(const OnErrorCb& cb)
{
    mOnErrorCbs.emplace_back(cb);
}

void TcpHandler::runOnReady(const OnReadyCb& cb)
{
    mOnReadyCbs.emplace_back(cb);
}

void TcpHandler::runOnClosed(const OnClosedCb& cb)
{
    mOnClosedCbs.emplace_back(cb);
}

void TcpHandler::runOnLost(const OnLostCb& cb)
{
    mOnLostCbs.emplace_back(cb);
}

void TcpHandler::runOnDetached(const OnDetachedCb& cb)
{
    mOnDetachedCbs.emplace_back(cb);
}

void TcpHandler::onError(AMQP::TcpConnection* connection, const char* msg)
{
    for (const auto& cb : mOnErrorCbs)
    {
        cb(connection, msg);
    }
}

void TcpHandler::onReady(AMQP::TcpConnection* connection)
{
    for (const auto& cb : mOnReadyCbs)
    {
        cb(connection);
    }
}

void TcpHandler::onClosed(AMQP::TcpConnection* connection)
{
    for (const auto& cb : mOnClosedCbs)
    {
        cb(connection);
    }
}

void TcpHandler::onLost(AMQP::TcpConnection* connection)
{
    for (const auto& cb : mOnLostCbs)
    {
        cb(connection);
    }
}

void TcpHandler::onDetached(AMQP::TcpConnection* connection)
{
    for (const auto& cb : mOnDetachedCbs)
    {
        cb(connection);
    }
}

uint16_t TcpHandler::onNegotiate(AMQP::TcpConnection* connection, uint16_t interval)
{
    auto overrideInterval = mHeartbeatIntervalSec.count() * kHeartbeatTimeoutToIntervalRatio;

    return AMQP::LibBoostAsioHandler::onNegotiate(connection, overrideInterval);
}

} // namespace simple_amqp
