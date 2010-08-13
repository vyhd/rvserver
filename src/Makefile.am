bin_PROGRAMS = rvserver

noinst_PROGRAMS = rvserver

AM_CXXFLAGS = -ggdb -fno-inline -Wall -pedantic

# some hacky stuff to get a build version auto-updating
.PHONY: build_ver

all: build_ver

build_ver:
	$(CC) -O2 verstub/gen-stub.c -o gen-stub
	chmod +x gen-stub
	./gen-stub

Network = network/Socket.cpp network/Socket.h \
	network/SocketListener.cpp network/SocketListener.h \
	network/DatabaseConnector.cpp network/DatabaseConnector.h \
	network/DatabaseWorker.cpp network/DatabaseWorker.h

Model = model/Room.cpp model/Room.h \
	model/RoomList.cpp model/RoomList.h \
	model/TimedList.cpp model/TimedList.h \
	model/User.cpp model/User.h

Logger = logger/Logger.cpp logger/Logger.h

Packet = packet/ChatPacket.cpp packet/ChatPacket.h \
	packet/PacketHandler.cpp packet/PacketHandler.h \
	packet/PacketUtil.cpp packet/PacketUtil.h \
	packet/MessageCodes.h

Util = util/libb64/cencode.c util/libb64/cencode.h \
	util/Base64.cpp util/Base64.h \
	util/Config.cpp util/Config.h \
	util/FileUtil.cpp util/FileUtil.h \
	util/StringUtil.cpp util/StringUtil.h \
	util/Thread.cpp util/Thread.h \
	util/URLEncoding.cpp util/URLEncoding.h

Handlers = \
	handlers/Action.cpp \
	handlers/AwayMessage.cpp \
	handlers/ListRooms.cpp \
	handlers/ListUsers.cpp \
	handlers/Login.cpp \
	handlers/Logout.cpp \
	handlers/Message.cpp \
	handlers/ModActions.cpp \
	handlers/RoomActions.cpp \
	handlers/PrivateMessage.cpp \
	handlers/SetConfig.cpp \
	handlers/Typing.cpp

# Compile handlers last due to deps
rvserver_SOURCES = $(Network) $(Model) $(Logger) $(Util) $(Packet) \
	ChatServer.cpp ChatServer.h Main.cpp $(Handlers)

# Needed for thread support.
rvserver_LDFLAGS = -lpthread
