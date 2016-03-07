//

// client.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//


//
// client.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <vector>
#include "connection.hpp" // Must come before boost/serialization headers.
#include <boost/serialization/vector.hpp>
#include <Particle.h>


/// Downloads stock quote information from a server.
template<class T>
class client {
public:

  /// Constructor starts the asynchronous connect operation.
  client(boost::asio::io_service& io_service,
         const std::string& host, const std::string& port)
    : connection_(io_service)
  {
    // Resolve the host name into an IP address.
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(host, port);
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
      resolver.resolve(query);
    boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

    // Start an asynchronous connect operation.
    connection_.socket().async_connect(endpoint,
                                       boost::bind(&client::handle_connect, this,
                                                   boost::asio::placeholders::
                                                   error, ++endpoint_iterator));
  }

  /// Handle completion of a connect operation.
  void handle_connect(const boost::system::error_code& e,
                      boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
  {
    if (!e)
    {
      connection_.async_read(particles,
                             boost::bind(&client::handle_read, this,
                                         boost::asio::placeholders::error));
    }
    else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
    {
      // Try the next endpoint.
      connection_.socket().close();
      boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
      connection_.socket().async_connect(endpoint,
                                         boost::bind(&client::handle_connect,
                                                     this,
                                                     boost::asio::placeholders::
                                                     error, ++endpoint_iterator));
    }
    else
    {
      // An error occurred. Log it and return. Since we are not starting a new
      // operation the io_service will run out of work to do and the client will
      // exit.
      std::cerr << "error in connection" << std::endl;
      std::cerr << e.message() << std::endl;
    }
  }

  /// Handle completion of a read operation.
  void handle_read(const boost::system::error_code& e)
  {
    if (!e)
    {
      for (std::size_t i = 0; i < particles.size(); ++i) {
        particles[i].print(std::cout, ' ');
        std::cout << std::endl;
      }
    }
    else
    {
      // An error occurred.
      std::cerr << e.message() << std::endl;
    }

    // Since we are not starting a new operation the io_service will run out of
    // work to do and the client will exit.
  }

private:

  /// The connection to the server.
  connection connection_;

  /// The data received from the server.
  std::vector < Particle < T >> particles;
};



int main(int argc, char *argv[])
{
  try
  {
    // Check command line arguments.
    if (argc != 3)
    {
      std::cerr << "Usage: client <host> <port>" << std::endl;
      return 1;
    }

    boost::asio::io_service io_service;
    client<float> client(io_service, argv[1], argv[2]);
    io_service.run();
  }
  catch (std::exception& e)
  { std::cerr << "error in main" << std::endl;
    std::cerr << e.what() << std::endl; }
  return 0;
}
