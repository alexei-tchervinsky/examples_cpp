//
// chat_message.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2016 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <boost/shared_ptr.hpp>

#include "chat_message_type.h"

class chat_message
{
public:

//  enum { header_length = 4 };
//  enum { max_body_length = 512 };

  enum { header_length = 0 };
  enum { max_username_length = 21 };
  enum { max_data_length = 256 };
  enum { max_msg_length = header_length + max_username_length + max_data_length };
//  enum { max_msg_length = MSG_MAX_LEN };

/*
  chat_message()
    : body_length_(0)
  {
  }
*/  
	chat_message()
	: body_length_(MSG_MAX_LEN)
  {
  }


  ::message_type message_type() const
  {
//	  return msg_.message_type_;
	  return msg_.type;
  }
  
  void message_type(::message_type msg_type)
  {
//	  msg_.message_type_= msg_type;
	  msg_.type = msg_type;
  }
       
  const char* data() const
  {
//    return msg_.data_;
    return msg_.data;
  }

  char* data()
  {
//    return msg_.data_;
    return msg_.data;
  }

  size_t length() const
  {
//   return header_length + body_length_;
//	return body_length_;
	return sizeof(msg_);//body_length_;
  }

  const char* body() const
  {
//    return data_ + header_length;
//    return msg_.data_;
    return msg_.data;
  }

  char* body()
  {
//    return data_ + header_length;
//    return msg_.data_;
    return msg_.data;
  }

  size_t body_length() const
  {
    return body_length_;
  }
/*
  void body_length(size_t new_length)
  {
    body_length_ = new_length;
    if (body_length_ > max_body_length)
      body_length_ = max_msg_length;
  }
*/
  void body_length(size_t new_length)
  {
  }
  
/*
  bool decode_header()
  {
    using namespace std; // For strncat and atoi.
    char header[header_length + 1] = "";
    strncat(header, data_, header_length);
    body_length_ = atoi(header);
    if (body_length_ > max_body_length)
    {
      body_length_ = 0;
      return false;
    }
    return true;
  }
*/  
  bool decode_header()
  {
    return true;
  }
  /*

  void encode_header()
  {
    using namespace std; // For sprintf and memcpy.
    char header[header_length + 1] = "";
    sprintf(header, "%4d", static_cast<int>(body_length_));
    memcpy(data_, header, header_length);
  }
*/
  void encode_header(::message_type msg_type)
  {
    using namespace std; // For sprintf and memcpy.
//    char header[header_length + 1] = "";
//    sprintf(header, "%4d", static_cast<int>(body_length_));
//    memcpy(message_type_, msg_type);
    message_type(msg_type);
  }
/*
  void encode_header()
  {
    using namespace std; // For sprintf and memcpy.
//    char header[header_length + 1] = "";
//    sprintf(header, "%4d", static_cast<int>(body_length_));
//    memcpy(message_type_, msg_type);
    //message_type(message_type.PUBLIC_MESSAGE);
    message_type(PUBLIC_MESSAGE);
  }
*/

// ChAP:
    const char* username() const
    {
//        return msg_.username_;
        return msg_.username;
    }
    
    void username (const char *name) 
    {
//        strncpy(msg_.username_, name, max_username_length);
        strncpy(msg_.username, name, USERNAME_MAX_LEN);
    }
    
    char* msg()
    {
        return (char*)&msg_;
    }

    void data(const char *data) 
    {
        strncpy(msg_.data, data, DATA_MAX_LEN);
    }
private:
/*
  struct {
	  	::message_type message_type_; 	
	  	char username_[max_username_length];
	  	char data_[max_data_length];
	} msg_;
*/    
//#pragma pack(1)
    message msg_;
//  message_type type; 	
//  char data_[max_username_length + max_body_length];
    size_t body_length_;
};

//----------------------------------------------------------------------
typedef boost::shared_ptr<chat_message> chat_message_ptr;

#endif // CHAT_MESSAGE_HPP
