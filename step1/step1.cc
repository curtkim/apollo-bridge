#include <iostream>
#include <boost/asio.hpp>

#include "server.h"

int main(int argc, char* argv[])
{
    try
    {
        //boost::asio::io_service io_service;
        auto server = std::make_shared<Server>();
        server->run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
