//

// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <ctime>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/serialization/vector.hpp>
#include <cstring>
#include <vector>
#include<ctime>
#include <Particle.h>
#include <utils.h>
#include "connection.hpp"


using boost::asio::ip::tcp;

/// Serves stock quote information to any client that connects to it.
class server
{
public:
  /// Constructor opens the acceptor and starts waiting for the first incoming
  /// connection.
  server(boost::asio::io_service& io_service, unsigned short port, unsigned int size)
    : acceptor_(io_service,
        boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
  {

    this->initparticles(1,200);
    std::cout<<"size "<<particles.size()<<std::endl;
    salt=size;

    // Start an accept operation for a new connection.
    connection_ptr new_conn(new connection(acceptor_.get_io_service()));
    acceptor_.async_accept(new_conn->socket(),
        boost::bind(&server::handle_accept, this,
          boost::asio::placeholders::error, new_conn));
  }

  /// Handle completion of a accept operation.
  void handle_accept(const boost::system::error_code& e, connection_ptr conn)
  {
    if (!e)
    {
    std::cout<<"Accepting connection"<<std::endl;
      // Successfully accepted a new connection. Send the list of stocks to the
      // client. The connection::async_write() function will automatically
      // serialize the data structure for us.
      conn->async_write(particles,
          boost::bind(&server::handle_write, this,
            boost::asio::placeholders::error, conn));
    }

    // Start an accept operation for a new connection.
    connection_ptr new_conn(new connection(acceptor_.get_io_service()));
    acceptor_.async_accept(new_conn->socket(),
        boost::bind(&server::handle_accept, this,
          boost::asio::placeholders::error, new_conn));
  }

  /// Handle completion of a write operation.
  void handle_write(const boost::system::error_code& e, connection_ptr conn)
  {
    // Nothing to do. The socket will be closed automatically when the last
    // reference to the connection object goes away.

  }

private:
  /// The acceptor object used to accept incoming socket connections.
  boost::asio::ip::tcp::acceptor acceptor_;

  /// The data to be sent to each client.
  std::vector<Particle<float>> particles;
  uint salt ;

  void initparticles(uint nproc, uint chunk) {
      auto s = time(0)*salt;
    srand(s);
    std::cout<<"salt is "<<s<<std::endl;
    if ((nproc <= 0) || (nproc > 8)) {
      nproc = 2;
    }

    for (int i = 0; i < nproc; ++i)
    {
      for (int i = 0; i < chunk; ++i)
      {
        Particle<float> *p = new Particle<float>();
        float xx           = d_rand<float>(0, 1);
        float yy           = d_rand<float>(0, 2);
        float zz           = d_rand<float>(0, 1);
        p->p = *(new glm::vec3(xx, yy, zz));

        xx   = d_rand<float>(0, 1);
        yy   = d_rand<float>(0, 1);
        zz   = d_rand<float>(0, 1);
        p->v = *(new glm::vec3(xx, yy, zz));

        particles.push_back(*p);
      }
    }
  }

};

int main(int argc, char* argv[])
{
  try
  {
    // Check command line arguments.
    if (argc != 2)
    {
      std::cerr << "Usage: server <port>" << std::endl;
      return 1;
    }
    int port =std::stoi(argv[1]);
    unsigned int n = port % 200;
    boost::asio::io_service io_service;
    server server(io_service, port , n);
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

