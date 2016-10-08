//
// echo_server.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2016 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/write.hpp>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <set>

#include "chat_message_type.h"
#include "chat_message.hpp"


using boost::asio::ip::tcp;


class chat_session; // forward declaration
typedef boost::shared_ptr<chat_session> chat_session_ptr;

//----------------------------------------------------------------------
// Room only handles set of sessions
//----------------------------------------------------------------------
class chat_room
{
	public:
		void join(chat_session_ptr session) { sessions_.insert(session); }
		void leave(chat_session_ptr session) { sessions_.erase(session); }
		std::set<chat_session_ptr> sessions() { return sessions_; }
	private:
	  std::set<chat_session_ptr> sessions_;
};

//----------------------------------------------------------------------
class chat_session : public boost::enable_shared_from_this<chat_session>
{
public:
  explicit chat_session(boost::asio::io_service& io_service, boost::shared_ptr<chat_room> room)
    : strand_(io_service),
      socket_(io_service),
      timer_(io_service),
      room_(room)
  {
  }

  tcp::socket& socket()
  {
    return socket_;
  }

  void go()
  {
	std::cout << "ChAP " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    boost::asio::spawn(strand_,
        boost::bind(&chat_session::chat,
          shared_from_this(), _1));
//    boost::asio::spawn(strand_,
//        boost::bind(&session::timeout,
//          shared_from_this(), _1));
  }

private:
  void chat(boost::asio::yield_context yield)
  {
    try
    {
//      char data[128];
      for (;;)
      {
//       timer_.expires_from_now(boost::posix_time::seconds(10));
        //std::size_t n = socket_.async_read_some(boost::asio::buffer(data), yield);
        boost::asio::async_read(
            socket_, 
			boost::asio::buffer(read_msg_.msg(), read_msg_.length()), yield);
		if (read_msg_.message_type() == SET_USERNAME)
		{
			this->username(read_msg_.username());
			room_->join(shared_from_this());
			chat_message m;
			m.message_type(CONNECT);
			boost::asio::async_write(socket_, boost::asio::buffer((const char*)&m, m.length()), yield);
		}	
		else if (read_msg_.message_type() == PUBLIC_MESSAGE)
		{
			std::set<chat_session_ptr>::iterator i;
			for (i = room_->sessions().begin(); i != room_->sessions().end(); ++i)
			{
				boost::asio::async_write((*i)->socket(), boost::asio::buffer((const char*)&read_msg_, read_msg_.length()), yield);
			}
			
//			boost::asio::async_write(socket_, boost::asio::buffer((const char*)&read_msg_, read_msg_.length()), yield);
		}
//        boost::asio::async_write(socket_, boost::asio::buffer(data, n), yield);
      }
    }
    catch (std::exception& e)
    {
      socket_.close();
      timer_.cancel();
    }
  }

  void timeout(boost::asio::yield_context yield)
  {
    while (socket_.is_open())
    {
      boost::system::error_code ignored_ec;
      timer_.async_wait(yield[ignored_ec]);
      if (timer_.expires_from_now() <= boost::posix_time::seconds(0))
        socket_.close();
    }
  }
  
  void username( const char *name )
  {
	  strncpy(username_, name, USERNAME_MAX_LEN);
  }
  
  char *username()
  {
	  return username_;
  }
  

  boost::asio::io_service::strand strand_;
  tcp::socket socket_;
  boost::asio::deadline_timer timer_;
  chat_message read_msg_;
  char username_[USERNAME_MAX_LEN];
  boost::shared_ptr<chat_room> room_;
  
};

//----------------------------------------------------------------------
void do_accept(boost::asio::io_service& io_service,
    unsigned short port, boost::shared_ptr<chat_room> room, boost::asio::yield_context yield)
{
  tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), port));

  for (;;)
  {
    boost::system::error_code ec;
//    boost::shared_ptr<chat_session> new_session(new chat_session(io_service), room);
    boost::shared_ptr<chat_session> new_session(new chat_session(io_service, room));
    acceptor.async_accept(new_session->socket(), yield[ec]);
    if (!ec) new_session->go();
  }
}

//----------------------------------------------------------------------
int main(int argc, char* argv[])
{
	
	
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: echo_server <port>\n";
      return 1;
    }

	boost::shared_ptr<chat_room> room (new chat_room);
	
    boost::asio::io_service io_service;

    boost::asio::spawn(io_service,
        boost::bind(do_accept,
          boost::ref(io_service), atoi(argv[1]), room, _1));

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
