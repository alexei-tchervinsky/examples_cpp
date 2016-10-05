// ChAP separate file for message type

#ifndef CHAT_MESSAGE_TYPE_H_
#define CHAT_MESSAGE_TYPE_H_

//Enum of different messages possible.
/*
typedef enum
{
  CONNECT = 0,
  DISCONNECT,
  GET_USERS,
  SET_USERNAME,
  PUBLIC_MESSAGE,
  PRIVATE_MESSAGE,
  TOO_FULL,
  USERNAME_ERROR,
  SUCCESS,
  ERROR

} message_type;
*/
typedef int message_type;

#define CONNECT  0
#define DISCONNECT  1
#define GET_USERS  2
#define SET_USERNAME  3
#define PUBLIC_MESSAGE  4
#define PRIVATE_MESSAGE  5
#define TOO_FULL  6
#define USERNAME_ERROR  7
#define SUCCESS  8
#define ERROR  9


#define USERNAME_MAX_LEN 21
#define DATA_MAX_LEN 256
/*#define MSG_MAX_LEN (sizeof(message_type)+USERNAME_MAX_LEN+DATA_MAX_LEN) */
#define MSG_MAX_LEN (4/*sizeof(message_type)*/+USERNAME_MAX_LEN+DATA_MAX_LEN)
typedef struct
{
  message_type type;
//  char username[21];
  char username[USERNAME_MAX_LEN];
//  char data[256];
  char data[DATA_MAX_LEN];

} message;


#endif
