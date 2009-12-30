bin_PROGRAMS = rvserver

noinst_PROGRAMS = rvserver

AM_CXXFLAGS = -ggdb

Network = network/SocketListener.cpp network/SocketListener.h
#network/MySQLConnector.cpp network/MySQLConnector.h

Model = model/User.cpp model/User.h

Logger = logger/Logger.cpp logger/Logger.h

Packet = packet/ChatPacket.cpp packet/ChatPacket.h \
	packet/PacketHandler.cpp packet/PacketHandler.h \
	packet/PacketUtil.cpp packet/PacketUtil.h \
	packet/MessageCodes.h

Handlers = handlers/Config.cpp \
	handlers/ListUsers.cpp \
	handlers/Login.cpp \
	handlers/Logout.cpp \
	handlers/RoomAction.cpp \
	handlers/RoomList.cpp \
	handlers/RoomMessage.cpp \
	handlers/PrivateMessage.cpp \
	handlers/Typing.cpp \
	handlers/UserKick.cpp \
	handlers/UserDisable.cpp \
	handlers/WallMessage.cpp

# Compile handlers last due to deps
rvserver_SOURCES = $(Network) $(Model) $(Logger) $(Packet) \
	ChatServer.cpp ChatServer.h $(Handlers)

#rvserver_LDFLAGS = -lmysqlpp
