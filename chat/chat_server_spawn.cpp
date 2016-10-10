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
		void list() 
		{
			std::cout << "ChAP " << __FILE__ << ":" <<__LINE__ << " " << __FUNCTION__ << std::endl;	  							
			std::set<chat_session_ptr>::iterator i;
			for (i = sessions_.begin(); i != sessions_.end(); ++i) {
				std::cout << *i << std::endl;
			}
		}
		std::set<chat_session_ptr>::iterator begin() { return sessions_.begin(); }
		std::set<chat_session_ptr>::iterator end() { return sessions_.end(); }
	
		
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
	  *username_ = '\0';
//	  send_disconnect_ = 0;
  }

  tcp::socket& socket()
  {
    return socket_;
  }

  void go()
  {
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
		std::cout << "ChAP " << __FILE__ << ":" <<__LINE__ << " " << __FUNCTION__ << " room_=" << room_ << std::endl;	  				
		print_users();
		room_->list();
		if (read_msg_.message_type() == SET_USERNAME)
		{
			if (add_user(read_msg_))
			{
				room_->join(shared_from_this());
				send_connect_message(read_msg_, yield);
			}
			else {
				socket_.close(); // reply duplicate name with close connection
			}
			
		}	
		else if (read_msg_.message_type() == PUBLIC_MESSAGE)
		{
			std::set<chat_session_ptr>::iterator i;
			for (i = room_->begin(); i != room_->end(); ++i)
			{
				boost::asio::async_write((*i)->socket(), boost::asio::buffer((const char*)&read_msg_, read_msg_.length()), yield);
			}
			
//			boost::asio::async_write(socket_, boost::asio::buffer((const char*)&read_msg_, read_msg_.length()), yield);
		}
		else if (read_msg_.message_type() == GET_USERS)
		{
			get_users(read_msg_, yield);
		}
		else if (read_msg_.message_type() == PRIVATE_MESSAGE)
		{
			send_private_message(read_msg_, yield);
		}
//        boost::asio::async_write(socket_, boost::asio::buffer(data, n), yield);
      }
    }
    catch (std::exception& e)
    {
		if (strlen(username()) != 0) {
			room_->leave(shared_from_this());
			const chat_message_ptr m( new chat_message() );
			m->username(username());
			send_disconnect_message(*m, yield);
		}
	}
/*    	
		char username[USERNAME_MAX_LEN];
		char *u;
		std::cout << "ChAP " << __FILE__ << ":" <<__LINE__ << " " << __FUNCTION__ << " EXCEPTION begin"<< std::endl;	  			
		print_users();
	  if ((u=del_user(read_msg_)) != NULL)
      {
		room_->leave(shared_from_this());
		strncpy(username, u, USERNAME_MAX_LEN);
		send_disconnect_ = 1;
	  }
      socket_.close();
      std::cout << "ChAP " << __FILE__ << ":" <<__LINE__ << " " << __FUNCTION__ << " EXCEPTION end" << std::endl;	  			
		print_users();
//      timer_.cancel();
    }
    if (send_disconnect_)
    {
		const chat_message_ptr m( new chat_message() );
		m->username(username());
    	send_disconnect_message(*m, yield);
    	send_disconnect_=0;
	}
	*/
  }
  
//----------------------------------------------------------------------
int add_user(const chat_message &msg)
{
	std::set<chat_session_ptr>::iterator i;
	for (i = room_->begin(); i != room_->end(); ++i)
	{
		if (strncmp(msg.username(), (*i)->username(), USERNAME_MAX_LEN) == 0)
			return 0; // duplicate
	}
	this->username(msg.username());
	return 1;
}

//----------------------------------------------------------------------
char * del_user(const chat_message &msg)
{
	std::set<chat_session_ptr>::iterator i;
	for (i = room_->begin(); i != room_->end(); ++i)
	{
		try {
//		if (strncmp(msg.username(), (*i)->username(), USERNAME_MAX_LEN) == 0)
			if (this->socket().remote_endpoint() == (*i)->socket().remote_endpoint()) // use endpoint because name is not valid for identification
			{
				std::cout << "ChAP " << __FILE__ << ":" <<__LINE__ << " " << __FUNCTION__ << " endpoint found for user " << username() << std::endl;
				return username(); // user is in the room
			}
		}
		catch (std::exception& e)
		{
			std::cout << "ChAP " << __FILE__ << ":" <<__LINE__ << " " << __FUNCTION__ << " endpoint exception" << std::endl;
			return NULL; // bad file descriptor, user is not in the room
		}
	}
	std::cout << "ChAP " << __FILE__ << ":" <<__LINE__ << " " << __FUNCTION__ << " endpoint not found" << std::endl;
	return NULL; // user is not in the room
}

//----------------------------------------------------------------------
  void send_connect_message(const chat_message &msg, boost::asio::yield_context yield)
  {
	std::cout << "ChAP " << __FILE__ << ":" <<__LINE__ << " " << __FUNCTION__ << std::endl;	  	  
	print_users();
	const chat_message_ptr m( new chat_message() );

	m->username(msg.username());
	
	std::set<chat_session_ptr>::iterator i;
	for (i = room_->begin(); i != room_->end(); ++i)
	{
		if (strncmp(this->username(), (*i)->username(), USERNAME_MAX_LEN) == 0)
			m->message_type(SUCCESS);
		else
			m->message_type(CONNECT);
			
//		std::cout << "ChAP " << __FILE__ << ":" <<__LINE__ << " " << __FUNCTION__ << " type=" << m->message_type() << std::endl;
			
//		boost::asio::async_write((*i)->socket(), boost::asio::buffer((const char*)&m, m->length()), yield);
		(*i)->deliver(*m, yield);
	}
  }
  
//----------------------------------------------------------------------
  void send_private_message(const chat_message &msg, boost::asio::yield_context yield)
  {
	  const chat_message_ptr m( new chat_message() );
	  
	std::set<chat_session_ptr>::iterator i;
	for (i = room_->begin(); i != room_->end(); ++i)
	{
		if (strncmp((*i)->username(), msg.username(), USERNAME_MAX_LEN) == 0)
		{
			m->username(this->username());
			m->message_type(PRIVATE_MESSAGE);
			m->data(msg.data());
			(*i)->deliver(*m, yield);
			return;
		}
	}
	m->message_type(USERNAME_ERROR);
	char unem[DATA_MAX_LEN];
	sprintf(unem, "Username \"%s\" does not exist or is not logged in.", msg.username());
	m->data(unem);
	this->deliver(*m, yield);
  }
  

//----------------------------------------------------------------------
  void send_disconnect_message(const chat_message &msg, boost::asio::yield_context yield)
  {
	std::cout << "ChAP " << __FILE__ << ":" <<__LINE__ << " " << __FUNCTION__ << std::endl;	  
	const chat_message_ptr m( new chat_message() );

	m->username(msg.username());
	m->message_type(DISCONNECT);
	
	print_users();
	
	std::set<chat_session_ptr>::iterator i;
	for (i = room_->begin(); i != room_->end(); ++i)
	{
		(*i)->print();
		(*i)->deliver(*m, yield);
	}
  }
  
//----------------------------------------------------------------------
	void get_users(const chat_message &msg, boost::asio::yield_context yield)
	{

		print_users();

		const boost::shared_ptr<chat_message> m (new chat_message()); // new message to modify
		char l[DATA_MAX_LEN]; 
		char *lp = l;
		m->message_type(msg.message_type());
		std::set<chat_session_ptr>::iterator i;
		for (i = room_->begin(); i != room_->end(); ++i)
		{
			std::cout << "ChAP user " << (*i)->username() << std::endl;
			lp = stpcpy(lp, (*i)->username()); // NB stpcpy
			std::cout << "ChAP l " << l << std::endl;
			lp = stpcpy(lp, "\n"); // NB stpcpy
			std::cout << "ChAP l " << l << std::endl;
		}
		*lp = 0;
		std::cout << "ChAP user list" << l << std::endl;
		strncpy(m->data(), l, DATA_MAX_LEN);
		this->deliver(*m, yield);
//		std::set<chat_session_ptr>::iterator i;
//		for (i = room_->begin(); i != room_->end(); ++i)
//		{
//			(*i)->deliver(*m, yield);
//		}
  }

  
//----------------------------------------------------------------------  
  void deliver(const chat_message &m, boost::asio::yield_context yield)
  {
	  	boost::asio::async_write(this->socket(), boost::asio::buffer((const char*)&m, m.length()), yield);
  }
  
//----------------------------------------------------------------------
	void print()
	{
		std::cout << username() << std::endl;
	}
	
//----------------------------------------------------------------------
void print_users()
{
	std::cout << "ChAP " << __FILE__ << ":" <<__LINE__ << " " << __FUNCTION__ << std::endl;	  
	std::set<chat_session_ptr>::iterator i;
	std::cout << *(room_->begin()) << std::endl;
	std::cout << *(room_->end()) << std::endl;
	for (i = room_->begin(); i != room_->end(); ++i)
	{
		(*i)->print();
	}
	/*
	std::cout << "room methods for iterations" << std::endl;
	for (i = room_->begin(); i != room_->end(); ++i)
	{
		(*i)->print();
	}

	room_->list();
	*/
}	
	
//----------------------------------------------------------------------	
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
 // int send_disconnect_;
  
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
