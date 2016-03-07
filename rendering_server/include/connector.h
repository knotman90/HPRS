#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <utils.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using boost::asio::ip::tcp;

template<typename HOST, typename SERVICE>
class connector {
  typedef boost::shared_ptr<boost::asio::ip::tcp::socket>socket_ptr;

public:

  connector(HOST _host, SERVICE _service)  : host(_host), service(_service) {}

  void connect() {
    try {
      boost::asio::io_service io_service;
      tcp::resolver resolver(io_service);
      tcp::resolver::query query(host, service);
      tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
      tcp::resolver::iterator end;
      socket_ptr ptr(new boost::asio::ip::tcp::socket(io_service));
      m_socket = ptr;

      // attempt connection
      boost::system::error_code error = boost::asio::error::host_not_found;

      while (error && endpoint_iterator != end)
      {
        m_socket->close();
        m_socket->connect(*endpoint_iterator++, error);
      }

      // if connection is not succesfull raise an error
      if (error) throw boost::system::system_error(error);
      else PRINTDEBUG("CONNECTED");
    } catch (std::exception& e) {
      tcp::resolver::iterator end;
    }
  }

#define SIZEBUFFER (128)
  void readJSON() {
    for (;;) {
      // We use a boost::array to hold the received data.
      boost::array<char, SIZEBUFFER>   buf;
      boost::system::error_code error;

      // The boost::asio::buffer() function automatically determines
      // the size of the array to help prevent buffer overruns.
      size_t len = m_socket->read_some(boost::asio::buffer(buf), error);

      if(len < SIZEBUFFER) buf[len] = '\0';

      // When the server closes the connection,
      // the ip::tcp::socket::read_some() function will exit with the
      // boost::asio::error::eof error,
      // which is how we know to exit the loop.
      if (error == boost::asio::error::eof) break;               // Connection
                                                                 // closed
                                                                 // cleanly by
                                                                 // peer.
      else if (error) throw boost::system::system_error(error);  // Some other
                                                                 // error.
    // std::cout << "read "  << len << std::endl;
      //std::cout << m_buffer.str() << std::endl;
      // if (len <= 126) {
      // std::string data(buf.begin(), buf.end());
      // m_buffer << data;
      // }  else {
     const char* tmp = boost::asio::buffer_cast<const char*>(boost::asio::buffer(buf,len));
      m_buffer << tmp;


      // }
    }
std::cout << "read END"  << std::endl;
    std::string buffer_str = m_buffer.str();
    //std::cout << buffer_str.size() << std::endl;
    std::cout <<"start parsing" << std::endl;

    boost::property_tree::read_json(m_buffer, pt);
     // print(pt);
      std::cout << "END" <<buffer_str.size() << std::endl;
  }

  /// Get the underlying socket. Used for making a connection or for accepting
  /// an incoming connection.
  boost::asio::ip::tcp::socket& socket()
  {
    return *m_socket;
  }

  void print(boost::property_tree::ptree const& pt)
  {
    using boost::property_tree::ptree;
    ptree::const_iterator end = pt.end();

    for (ptree::const_iterator it = pt.begin(); it != end; ++it) {
       std::cout << it->first << ": " << it->second.get_value<std::string>() << std::endl;
      print(it->second);
    }
  }

boost::property_tree::ptree& getJson(){
        return pt;
}
private:

  // The underlying socket.
  socket_ptr m_socket;
  HOST host;
  SERVICE service;
  std::stringstream m_buffer;

  boost::property_tree::ptree pt;
};


#endif // CONNECTOR_H
