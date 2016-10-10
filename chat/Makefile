# kdg768 2016-09-29
# http://stackoverflow.com/questions/2481269/how-to-make-a-simple-c-makefile

CC=gcc
CXX=g++
RM=rm -f
#CPPFLAGS=-g $(shell root-config --cflags)
#CPPFLAGS=-pthread 
CPPFLAGS_SERVER=-Wall
CCFLAGS_CLIENT=-Wall
#LDFLAGS=-g $(shell root-config --ldflags)
LDFLAGS=
#LDLIBS=$(shell root-config --libs)
#LDLIBS=-lboost_system -lpthread -lboost_thread
LDLIBS_SERVER=-lboost_system -lboost_thread -lpthread
LDLIBS_SERVER_SPAWN = -lboost_system -lboost_thread -lpthread -lboost_coroutine -lboost_context 
LDLIBS_CLIENT=-lpthread

SRCS_SERVER=chat_server.cpp
SRCS_SERVER_SPAWN=chat_server_spawn.cpp
SRCS_CLIENT=client.c chatroom_utils.c
OBJS_SERVER=$(subst .cpp,.o,$(SRCS_SERVER))
OBJS_SERVER_SPAWN=$(subst .cpp,.o,$(SRCS_SERVER_SPAWN))
OBJS_CLIENT=$(subst .c,.o,$(SRCS_CLIENT))

all: chat_server client chat_server_spawn

chat_server: $(OBJS_SERVER) chat_message.hpp chat_message_type.h Makefile
	$(CXX) $(CPPFLAGS_SERVER) $(LDFLAGS) -o chat_server $(OBJS_SERVER) $(LDLIBS_SERVER) 
	
chat_server_spawn: $(OBJS_SERVER_SPAWN) chat_message.hpp chat_message_type.h Makefile
	$(CXX) $(CPPFLAGS_SERVER) $(LDFLAGS) -o chat_server_spawn $(OBJS_SERVER_SPAWN) $(LDLIBS_SERVER_SPAWN) 
	
#chat_client: $(OBJS_CLIENT) Makefile
#	$(CXX) $(LDFLAGS) -o chat_client $(OBJS_CLIENT) $(LDLIBS) 
	
client: $(OBJS_CLIENT) chatroom_utils.h chat_message_type.h Makefile 
	$(CC) $(CCFLAGS_CLIENT) $(LFLAGS) $(OBJS_CLIENT) -o client $(LDLIBS_CLIENT)	
	

depend: .depend

.depend: $(SRCS_SERVER) $(SRCS_CLIENT)
	rm -f ./.depend
#	$(CXX) $(CPPFLAGS_SERVER) -MM $^>>./.depend;
	$(CXX) $(CPPFLAGS_SERVER) -MM $(SRCS_SERVER)>>./.depend;
	$(CC) $(CCFLAGS_CLIENT) -MM $(SRCS_CLIENT)>>./.depend;

clean:
	$(RM) $(OBJS_SERVER) $(OBJS_CLIENT)

dist-clean: clean
	$(RM) *~ .depend

include .depend
