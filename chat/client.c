/*
*
* Chatroom - a simple linux commandline client/server C program for group chat.
* Author: Andrew Herriot
* License: Public Domain
*
*/

// Modified by Alexei Tchervinsky (alexey dot tchervinsky at gmail dot com)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>


#include <sys/socket.h>
#include <arpa/inet.h>
#include "chat_message_type.h"
#include "chatroom_utils.h"

// get a username from the user.
void get_username(char *username)
{
  while(true)
  {
    printf("Enter a username: ");
    fflush(stdout);
    memset(username, 0, 1000);
    fgets(username, 22, stdin);
    trim_newline(username);

    if(strlen(username) > 20)
    {
      // clear_stdin_buffer();
      puts("Username must be 20 characters or less.");

    } else {
      break;
    }
  }
}

//send local username to the server.
void set_username(connection_info *connection)
{
  message msg;
  msg.type = SET_USERNAME;
  strncpy(msg.username, connection->username, 20);

  if(send(connection->socket, (void*)&msg, sizeof(msg), 0) < 0)
  {
    perror("Send failed");
    exit(1);
  }
}

void stop_client(connection_info *connection)
{
  close(connection->socket);
  exit(0);
}

//initialize connection to the server.
void connect_to_server(connection_info *connection, char *address, char *port)
{

  while(true)
  {
    get_username(connection->username);

    //Create socket
    if ((connection->socket = socket(AF_INET, SOCK_STREAM , IPPROTO_TCP)) < 0)
    {
        perror("Could not create socket");
    }

    connection->address.sin_addr.s_addr = inet_addr(address);
    connection->address.sin_family = AF_INET;
    connection->address.sin_port = htons(atoi(port));

    //Connect to remote server
    if (connect(connection->socket, (struct sockaddr *)&connection->address , sizeof(connection->address)) < 0)
    {
        perror("Connect failed.");
        exit(1);
    }

    set_username(connection);

    message msg;
    ssize_t recv_val = recv(connection->socket, &msg, sizeof(message), 0);
    if(recv_val < 0)
    {
        perror("recv failed");
        exit(1);

    }
    else if(recv_val == 0)
    {
      close(connection->socket);
      printf("The username \"%s\" is taken, please try another name.\n", connection->username);
      continue;
    }

    break;
  }


  puts("Connected to server.");
  puts("Type /help for usage.");
}

void send_public_message(connection_info *connection, char *input) 
{
    message msg;
    msg.type = PUBLIC_MESSAGE;
    strncpy(msg.username, connection->username, 20);

    // clear_stdin_buffer();

    if(strlen(input) == 0) {
        return;
    }

    strncpy(msg.data, input, 255);
//    printf("ChAP %s:%d %s Public Message msg.type=%d sizeof(msg)=%d\n", __FILE__, __LINE__, __FUNCTION__, msg.type, sizeof(msg)); 
    //Send some data
    if(send(connection->socket, &msg, sizeof(message), 0) < 0)
    {
        perror("Send failed");
        exit(1);
    }

}

int silent_mode = 0;
int batch_mode = 0;
int batch_limit = 0;
int batch_counter = 0;

void handle_user_input(connection_info *connection)
{
  char input[255];
  fgets(input, 255, stdin);
  trim_newline(input);

  if(strcmp(input, "/q") == 0 || strcmp(input, "/quit") == 0)
  {
    stop_client(connection);
  }
  else if(strcmp(input, "/l") == 0 || strcmp(input, "/list") == 0)
  {
    message msg;
    msg.type = GET_USERS;

    if(send(connection->socket, &msg, sizeof(message), 0) < 0)
    {
        perror("Send failed");
        exit(1);
    }
  }
  else if(strcmp(input, "/h") == 0 || strcmp(input, "/help") == 0)
  {
    puts("/quit or /q: Exit the program.");
    puts("/help or /h: Displays help information.");
    puts("/list or /l: Displays list of users in chatroom.");
    puts("/batch or /b: Send <number of repeats (0 - endless)> <global message>.");
    puts("/cancel or /c: Cancel batch mode (useful by endless batch.");
    puts("/silent or /s: Silent mode <1 or 0>.");
    puts("/m <username> <message> Send a private message to given username.");
  }
  else if(strncmp(input, "/m", 2) == 0)
  {
    message msg;
    msg.type = PRIVATE_MESSAGE;

    char *toUsername, *chatMsg;

    toUsername = strtok(input+3, " ");

    if(toUsername == NULL)
    {
      puts(KRED "The format for private messages is: /m <username> <message>" RESET);
      return;
    }

    if(strlen(toUsername) == 0)
    {
      puts(KRED "You must enter a username for a private message." RESET);
      return;
    }

    if(strlen(toUsername) > 20)
    {
      puts(KRED "The username must be between 1 and 20 characters." RESET);
      return;
    }

    chatMsg = strtok(NULL, "");

    if(chatMsg == NULL)
    {
      puts(KRED "You must enter a message to send to the specified user." RESET);
      return;
    }

    //printf("|%s|%s|\n", toUsername, chatMsg);
    strncpy(msg.username, toUsername, 20);
    strncpy(msg.data, chatMsg, 255);

    if(send(connection->socket, &msg, sizeof(message), 0) < 0)
    {
        perror("Send failed");
        exit(1);
    }

  }
  else if(strncmp(input, "/b", 2) == 0)
  {

	batch_mode = 1;
	
    char *chatMsg = NULL;


    batch_limit = atoi(strtok(input+3, " "));
    printf("ChAP batch_limit=%d\n", batch_limit);
    batch_counter = 0;
    
    chatMsg = strtok(NULL, "");
    
    send_public_message(connection, chatMsg);

/*    
    if (number_of_repeats != 0)
		for (i=0; i<number_of_repeats; i++)
		{
			send_public_message(connection, chatMsg);
		}
	else 
		while (1)
		{
			send_public_message(connection, chatMsg);
		}
*/ 
  }
  else if(strncmp(input, "/s", 2) == 0)
  {
    silent_mode = atoi(strtok(input+3, " "));
  }
  
  else if(strncmp(input, "/c", 2) == 0)
  {
    batch_mode = 0;
  }
  

  else //regular public message
  {
	  send_public_message(connection, input);
  }

}

//static i=0;


void handle_server_message(connection_info *connection)
{
  message msg;
//  printf("ChAP %s:%d %s i=%d\n", __FILE__, __LINE__, __FUNCTION__, i++);
  //Receive a reply from the server
  ssize_t recv_val = recv(connection->socket, &msg, sizeof(message), 0);
  if(recv_val < 0)
  {
      perror("recv failed");
      exit(1);

  }
  else if(recv_val == 0)
  {
    close(connection->socket);
    puts("Server disconnected.");
    exit(0);
  }
  
//  printf("ChAP %s:%d %s msg.type=%d\n", __FILE__, __LINE__, __FUNCTION__, msg.type);
 
  switch(msg.type)
  {

    case CONNECT:
      printf(KYEL "%s has connected." RESET "\n", msg.username);
    break;

    case DISCONNECT:
      printf(KYEL "%s has disconnected." RESET "\n" , msg.username);
    break;

    case GET_USERS:
      printf("%s", msg.data);
    break;

    case SET_USERNAME:
      //TODO: implement: name changes in the future?
    break;

    case PUBLIC_MESSAGE:
      if (batch_mode)
      {
        if (batch_limit != 0)
        {
          if (batch_counter++ < batch_limit)
          {
            if (silent_mode == 0)
              printf(KGRN "%s" RESET ": %d of %d: %s\n", msg.username,  batch_counter, batch_limit,  msg.data);
              
            send_public_message(connection, msg.data); // repeat the public message
          }
          else
          {
            batch_mode = 0;
          }
        }
        else
        {
          batch_counter++;
          if (silent_mode == 0)
            printf(KGRN "%s" RESET ": %d of endless %s\n", msg.username, batch_counter, msg.data);
            
          send_public_message(connection, msg.data); // repeat the public message
        }
      }
	  else 
      { // single mode
        if (silent_mode == 0)
          printf(KGRN "%s" RESET ": %s\n", msg.username, msg.data);
      }
		
    break;

    case PRIVATE_MESSAGE:
      printf(KWHT "From %s:" KCYN " %s\n" RESET, msg.username, msg.data);
    break;
    
    case USERNAME_ERROR:
		printf(KRED "%s\n" RESET, msg.data);
	break;

    case TOO_FULL:
      fprintf(stderr, KRED "Server chatroom is too full to accept new clients." RESET "\n");
      exit(0);
    break;

    default:
      fprintf(stderr, KRED "Unknown message type received." RESET "\n");
    break;
  }
 /*
    if (msg.type == CONNECT) 
    {
      printf(KYEL "%s has connected." RESET "\n", msg.username);
	}
	else if (msg.type == DISCONNECT)
	{
      printf(KYEL "%s has disconnected." RESET "\n" , msg.username);
    } 
    else if (msg.type == GET_USERS)
    {
      printf("%s", msg.data);
    }
    else if (msg.type == SET_USERNAME)
	{
      //TODO: implement: name changes in the future?
	}
    else if (msg.type == PUBLIC_MESSAGE)
    {
      printf(KGRN "%s" RESET ": %s\n", msg.username, msg.data);
	}
	else if (msg.type == PRIVATE_MESSAGE)
	{
      printf(KWHT "From %s:" KCYN " %s\n" RESET, msg.username, msg.data);
	}
    else if (msg.type == TOO_FULL)
    {
      fprintf(stderr, KRED "Server chatroom is too full to accept new clients." RESET "\n");
      exit(0);
	}
    else 
    {
      fprintf(stderr, KRED "Unknown message type received." RESET "\n");
    }
*/
}

int main(int argc, char *argv[])
{
  connection_info connection;
  fd_set file_descriptors;

  if (argc != 3) {
    fprintf(stderr,"Usage: %s <IP> <port>\n", argv[0]);
    exit(1);
  }

  connect_to_server(&connection, argv[1], argv[2]);

  //keep communicating with server
  while(true)
  {
    FD_ZERO(&file_descriptors);
    FD_SET(STDIN_FILENO, &file_descriptors);
    FD_SET(connection.socket, &file_descriptors);
    fflush(stdin);

    if(select(connection.socket+1, &file_descriptors, NULL, NULL, NULL) < 0)
    {
      perror("Select failed.");
      exit(1);
    }

    if(FD_ISSET(STDIN_FILENO, &file_descriptors))
    {
      handle_user_input(&connection);
    }

    if(FD_ISSET(connection.socket, &file_descriptors))
    {
      handle_server_message(&connection);
    }
  }

  close(connection.socket);
  return 0;
}


//something to look into for better stdin
//http://stackoverflow.com/questions/421860/capture-characters-from-standard-input-without-waiting-for-enter-to-be-pressed
// #include <unistd.h>
// #include <termios.h>
//
// char getch() {
//         char buf = 0;
//         struct termios old = {0};
//         if (tcgetattr(0, &old) < 0)
//                 perror("tcsetattr()");
//         old.c_lflag &= ~ICANON;
//         old.c_lflag &= ~ECHO;
//         old.c_cc[VMIN] = 1;
//         old.c_cc[VTIME] = 0;
//         if (tcsetattr(0, TCSANOW, &old) < 0)
//                 perror("tcsetattr ICANON");
//         if (read(0, &buf, 1) < 0)
//                 perror ("read()");
//         old.c_lflag |= ICANON;
//         old.c_lflag |= ECHO;
//         if (tcsetattr(0, TCSADRAIN, &old) < 0)
//                 perror ("tcsetattr ~ICANON");
//         return (buf);
// }
