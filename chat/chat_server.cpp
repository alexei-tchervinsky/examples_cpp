//
// chat_server.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2016 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <set>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "chat_message.hpp"

using boost::asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<chat_message> chat_message_queue;

//----------------------------------------------------------------------
class chat_room;
//----------------------------------------------------------------------
class chat_session : 
      public boost::enable_shared_from_this<chat_session> {
	public:
		chat_session(boost::asio::io_service& io_service, chat_room& room);
		tcp::socket& socket();
		void start();
		void deliver(const chat_message& msg);
		void handle_read_body(const boost::system::error_code& error);
		void handle_write(const boost::system::error_code& error);
private:
  tcp::socket socket_;
  chat_room& room_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;		
	};
typedef boost::shared_ptr<chat_session> chat_session_ptr;
//----------------------------------------------------------------------

class chat_room
{
public:
  void join(chat_session_ptr session)
  {
		printf("ChAP %s:%d %s session=%p socket=%p\n", __FILE__, __LINE__, __FUNCTION__, &session, &session->socket());
    sessions_.insert(session);
    std::for_each(recent_msgs_.begin(), recent_msgs_.end(),
        boost::bind(&chat_session::deliver, session, _1));
  }

  void leave(chat_session_ptr session)
  {
    sessions_.erase(session);
  }

  void deliver(const chat_message& msg)
  {

	 recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs)
      recent_msgs_.pop_front();

		printf("ChAP %s:%d %s type=%d username=%s data=%s\n", __FILE__, __LINE__, __FUNCTION__, msg.message_type(), msg.username(), msg.data());
		if (msg.message_type() == SET_USERNAME) {
			const boost::shared_ptr<chat_message> m (new chat_message());
			m->message_type(CONNECT);
//			m->username(msg.username());
			m->username("foo");
			std::for_each(sessions_.begin(), sessions_.end(),
        boost::bind(&chat_session::deliver, _1, boost::ref(*m)));
		}
		else if (msg.message_type() == PUBLIC_MESSAGE)
		{
			std::for_each(sessions_.begin(), sessions_.end(),
					boost::bind(&chat_session::deliver, _1, boost::ref(msg)));
		}
		else if (msg.message_type() == PRIVATE_MESSAGE)
		{
			//boost::bind(&participant::deliver, _1, NULL); // FIXME
		}
		
		
		{
			printf("ChAP %s:%d %s Unknown msg type %d\n", __FILE__, __LINE__, __FUNCTION__, msg.message_type());
		}
			
  }

private:
  std::set<chat_session_ptr> sessions_;
  enum { max_recent_msgs = 100 };
  chat_message_queue recent_msgs_;
};

//----------------------------------------------------------------------

//class chat_session

  chat_session::chat_session(boost::asio::io_service& io_service, chat_room& room)
    : socket_(io_service),
      room_(room)
  {
		printf("ChAP %s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
  }

  tcp::socket& chat_session::socket()
  {
    return socket_;
  }

  void chat_session::start()
  {
	printf("ChAP %s:%d %s socket=%p\n", __FILE__, __LINE__, __FUNCTION__, &socket_);
    room_.join(shared_from_this());
    boost::asio::async_read(socket_,
//        boost::asio::buffer(read_msg_.data(), chat_message::header_length),
        boost::asio::buffer(read_msg_.msg(), read_msg_.length()),
//        boost::asio::buffer(read_msg_.msg(), 1),
//        boost::asio::buffer(read_msg_.msg(), chat_message::max_msg_length),
        boost::bind(
//          &chat_session::handle_read_header, shared_from_this(),
          &chat_session::handle_read_body, shared_from_this(),
          boost::asio::placeholders::error));
  }

  void chat_session::deliver(const chat_message& msg)
  {
		printf("ChAP %s:%d %s type=%d username=%s data=%s\n", __FILE__, __LINE__, __FUNCTION__, msg.message_type(), msg.username(), msg.data());
//		boost::shared_ptr<chat_message> m (new chat_message());
//		m->message_type(msg.message_type());
//		m->username("foo");
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress)
    {
			printf("ChAP %s:%d %s socket=%p username=%s\n", __FILE__, __LINE__, __FUNCTION__, &socket_, msg.username());
      boost::asio::async_write(socket_,
//          boost::asio::buffer(write_msgs_.front().data(),
//            write_msgs_.front().length()),
//          boost::asio::buffer(&m, sizeof(::message_type)+strlen(m->username())),
          boost::asio::buffer((const char*)&msg, sizeof(::message_type)+strlen(msg.username())),
          boost::bind(&chat_session::handle_write, shared_from_this(),
            boost::asio::placeholders::error));
    }
    else {
			printf("ChAP %s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
		}
  }
/*
  void handle_read_header(const boost::system::error_code& error)
  {
		printf("ChAP %s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
    if (!error && read_msg_.decode_header())
    {
			printf("ChAP %s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
      boost::asio::async_read(socket_,
          boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
          boost::bind(&chat_session::handle_read_body, shared_from_this(),
            boost::asio::placeholders::error));
    }
    else
    {
			printf("ChAP %s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
      room_.leave(shared_from_this());
    }
  }
*/
  void chat_session::handle_read_body(const boost::system::error_code& error)
  {
		printf("ChAP %s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
    if (!error)
    {
			printf("ChAP %s:%d %s length=%d\n", __FILE__, __LINE__, __FUNCTION__, read_msg_.length());
      room_.deliver(read_msg_);
      boost::asio::async_read(socket_,
//          boost::asio::buffer(read_msg_.data(), chat_message::header_length),
//          boost::asio::buffer(read_msg_.msg(), read_msg_, chat_message::max_msg_length),
          boost::asio::buffer(read_msg_.msg(), read_msg_.length()),
//          boost::bind(&chat_session::handle_read_header, shared_from_this(),
          boost::bind(&chat_session::handle_read_body, shared_from_this(),
            boost::asio::placeholders::error));
    }
    else
    {
			printf("ChAP %s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
      room_.leave(shared_from_this());
    }
  }

  void chat_session::handle_write(const boost::system::error_code& error)
  {
		printf("ChAP %s:%d %s error %d\n", __FILE__, __LINE__, __FUNCTION__, error);
		
    if (!error)
    {
      write_msgs_.pop_front();
      if (!write_msgs_.empty()) 
      {
				printf("ChAP %s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
        boost::asio::async_write(socket_,
//            boost::asio::buffer(write_msgs_.front().data(),
            boost::asio::buffer(write_msgs_.front().msg(),
              write_msgs_.front().length()),
//             chat_message::max_msg_length,
//             (int)MSG_MAX_LEN,
            boost::bind(&chat_session::handle_write, shared_from_this(),
              boost::asio::placeholders::error));
      }
    }
    else
    {
			printf("ChAP %s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
			perror("error");
      room_.leave(shared_from_this());
    }
  }


//----------------------------------------------------------------------

class chat_server
{
public:
  chat_server(boost::asio::io_service& io_service,
      const tcp::endpoint& endpoint)
    : io_service_(io_service),
      acceptor_(io_service, endpoint)
  {
    start_accept();
  }

  void start_accept()
  {
    chat_session_ptr new_session(new chat_session(io_service_, room_));
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&chat_server::handle_accept, this, new_session,
          boost::asio::placeholders::error));
  }

  void handle_accept(chat_session_ptr session,
      const boost::system::error_code& error)
  {
		printf("ChAP %s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
    if (!error)
    {
      session->start();
    }

    start_accept();
  }

private:
  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
  chat_room room_;
};

typedef boost::shared_ptr<chat_server> chat_server_ptr;
typedef std::list<chat_server_ptr> chat_server_list;

//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
  printf("ChAP %s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
  try
  {
    if (argc < 2)
    {
      std::cerr << "Usage: chat_server <port> [<port> ...]\n";
      return 1;
    }

    boost::asio::io_service io_service;

    chat_server_list servers;
    for (int i = 1; i < argc; ++i)
    {
      using namespace std; // For atoi.
      tcp::endpoint endpoint(tcp::v4(), atoi(argv[i]));
      chat_server_ptr server(new chat_server(io_service, endpoint));
      servers.push_back(server);
    }

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
