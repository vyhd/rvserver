bin_PROGRAMS = rvserver

noinst_PROGRAMS = rvserver

AM_CXXFLAGS = -ggdb -Wall -pedantic

# some hacky stuff to get a build version auto-updating
.PHONY: build_ver

all: build_ver

build_ver:
	$(CC) -O2 verstub/gen-stub.c -o gen-stub
	chmod +x gen-stub
	./gen-stub

Network = network/SocketListener.cpp network/SocketListener.h
#network/MySQLConnector.cpp network/MySQLConnector.h

Model = model/User.cpp model/User.h

Logger = logger/Logger.cpp logger/Logger.h

Packet = packet/ChatPacket.cpp packet/ChatPacket.h \
	packet/PacketHandler.cpp packet/PacketHandler.h \
	packet/PacketUtil.cpp packet/PacketUtil.h \
	packet/MessageCodes.h

Handlers = \
	handlers/Action.cpp \
	handlers/AwayMessage.cpp \
	handlers/Config.cpp \
	handlers/ListRooms.cpp \
	handlers/ListUsers.cpp \
	handlers/Login.cpp \
	handlers/Logout.cpp \
	handlers/Message.cpp \
	handlers/ModActions.cpp \
	handlers/RoomActions.cpp \
	handlers/PrivateMessage.cpp \
	handlers/Typing.cpp

# Compile handlers last due to deps
rvserver_SOURCES = $(Network) $(Model) $(Logger) $(Packet) \
	ChatServer.cpp ChatServer.h $(Handlers)

#rvserver_LDFLAGS = -lmysqlpp
