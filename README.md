# Simple AMQP-CPP

## Overview

This is a RabbitMQ client, built for Linux-TCP sockets, based on the AMQP-CPP project:
https://github.com/CopernicaMarketingSoftware/AMQP-CPP

The fundamental idea of Simple AMQP-CPP is to provider a `Connection` class that offers:
- thread-safe implementation
- integrated event loop (using Boost.Asio)
- connect & disconnect through object contruction & destruction (though, connection can be lost while the object is alive)

## Requirements

- Compiler with C++17 support (tested with g++ 9.4.0).
- Linux-only integration, with TCP sockets.
- You need a RabbitMQ server to test the client.

## Building Library and Example Application

Tested under Ubuntu 20.04.

Compile:

    $ ./build.sh

Clean and compile:

    $ ./build.sh --clean

You can find the sample program `example` and the static library `libsimple-amqpcpp.a` in the build/ directory.

## Running the Example Application

    $ build/example <image_path>

## AMQP-CPP event loop integration

In AMQP-CPP, there are 2 open subjects to be integrated by the user: 1) the protocol layer integration, that carries AMQP messages, and 2) the dispatching of AMQP events, via an event loop integration.

For the first topic, I used the lib provided TCP socket integration for Linux.
For the second topic, AMPQ-CPP has integrations availble for LibEV, LibUV, LibEvent and library Boost.Asio. I picked Boost.Asio.

It turned out, there are some issues with that Boost integration, and I troubleshoot myself some problems, and pushed a fix upstream:
https://github.com/CopernicaMarketingSoftware/AMQP-CPP/pull/463

In order to make it thread safe, I used boost::asio::context_io with single-thread. However, because we cannot perform any operation outside the event loop, to ensure thread-safety, I opted to create the APIs:

```
bool useChannel(const ChannelOperation& channelOperation);
bool useChannel(const ReliableWrapperOperation& reliableOperation);
```

The operation arguments provided are functions that the user can register/post in the event-loop. See the example provided in example.cpp file.
