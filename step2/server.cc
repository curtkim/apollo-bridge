/**
 * Copyright (c) 2019 LG Electronics, Inc.
 *
 * This software contains code licensed as described in LICENSE.
 *
 */
#include "server.h"
#include "client.h"

#include <memory>
#include <functional>
#include <cstdint>
#include <signal.h>
#include <boost/bind.hpp>
#include <iostream>

Server::Server()
    : signals(io)
    , endpoint(boost::asio::ip::tcp::v4(), 9090)
    , acceptor(io, endpoint)
    , socket(io)
{
    signals.add(SIGTERM);
    signals.add(SIGINT);
}

Server::~Server()
{
}

void Server::run()
{
    signals.async_wait(boost::bind(
        &Server::stop,
        shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::signal_number));

    begin_accept();

    io.run();
}

void Server::stop(const boost::system::error_code& ec, int signal_number)
{
    if (ec)
    {
        std::cerr << "Error waiting on signals: " << ec.message();
    }
    else
    {
        std::cout << "Signal " << signal_number << " received, stopping server";
    }
    acceptor.close();
}

void Server::begin_accept()
{
    acceptor.async_accept(
        socket,
        boost::bind(
            &Server::end_accept,
            shared_from_this(),
            boost::asio::placeholders::error));
}

void Server::end_accept(const boost::system::error_code& ec)
{
    if (!acceptor.is_open())
    {
        return;
    }

    if (ec)
    {
        std::cerr << "Error accepting connection: " << ec.message();
    }
    else
    {
        auto client = std::make_shared<Client>(std::move(socket));
        client->start();
    }

    begin_accept();
}
