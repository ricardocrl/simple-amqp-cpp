#include "Connection.hpp"

#include <iostream>

void receivedCb(AMQP::TcpChannel& channel, const std::string& queue, const AMQP::Message& message, uint64_t deliveryTag)
{
    std::cerr << "Consumer: receiveCb - queue = " << queue
              << " message = " << std::string(message.body(), message.bodySize()) << std::endl;
    channel.ack(deliveryTag);
}

void onAckCb(const std::string& queue, const std::string& message)
{
    std::cerr << "Publisher: onAckCb - queue = " << queue << " message = " << message << std::endl;
}

void onLostCb(const std::string& queue, const std::string& message)
{
    std::cerr << "Publisher: onLostCb - queue = " << queue << " message = " << message << std::endl;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Broker address (and login credentials) not provided" << std::endl;
    }

    std::cout << "Creating connection" << std::endl;

    // address string format: amqp[s]://[<user>[:<password>]@]<broker_hostname>[:<port>]
    auto connection = std::make_unique<simple_amqpcpp::Connection>(
        AMQP::Address(argv[1]), [] { std::cout << "Connection detached" << std::endl; });

    if (!connection->connected())
    {
        std::cerr << "Failed to establish connection to broker" << std::endl;
        return 1;
    }

    std::cout << std::endl << "Declaring queues and consuming" << std::endl;

    connection->useChannel([](simple_amqpcpp::Channel& channel) {
        channel.declareQueue("queue_1");
        channel.declareQueue("queue_2");

        channel.consume("queue_1").onReceived(
            [&](const auto& message, auto tag, bool) { receivedCb(channel, "queue_1", message, tag); });
        channel.consume("queue_2").onReceived(
            [&](const auto& message, auto tag, bool) { receivedCb(channel, "queue_2", message, tag); });
    });

    std::cout << std::endl << "Sending messages" << std::endl;

    for (size_t i = 0; i < 5; ++i)
    {
        std::string message = "message_" + std::to_string(i);

        // simple channel
        connection->useChannel([=](simple_amqpcpp::Channel& channel) { channel.publish("", "queue_1", message); });

        // robust channel - with AMQP::Reliable and AMQP::Throttle
        connection->useChannel([=](simple_amqpcpp::ReliableWrapper& channel) {
            channel.publish("", "queue_2", message).onAck([=] { onAckCb("queue_2", message); }).onLost([=] {
                onLostCb("queue_2", message);
            });
        });

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
