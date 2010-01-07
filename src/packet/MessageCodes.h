#ifndef MESSAGE_CODES_H
#define MESSAGE_CODES_H

enum MessageCode
{
	// user messages
	USER_LIST	= 0,
	USER_JOIN	= 1,
	USER_PART	= 2,

	// communication messages
	ROOM_MESSAGE	= 3,
	ROOM_ACTION	= 4,
	USER_PM		= 5,

	// moderator commands
	USER_KICK	= 6,
	USER_DISABLE	= 7,
	USER_BAN	= 8,
	USER_UNBAN	= 9,
	USER_MUTE	= 10,
	USER_UNMUTE	= 11,

	SERVER_DOWN	= 12,
	IDLE_KICK	= 13,
	IP_QUERY	= 14,
	DEBUG_COMMAND	= 15,

	// timedban command
	USER_TIMEDBAN	= 16,

	// mod chat
	MOD_CHAT	= 20,

	// login responses
	ACCESS_GRANTED	= 100,
	ACCESS_DENIED	= 101,

	// ???
	PM_BOX		= 200,

	// other mod commands
	FORCE_CLEAR	= 300,
	FORCE_URL	= 301,

	// room commands
	JOIN_ROOM	= 400,
	CREATE_ROOM	= 402,
	DESTROY_ROOM	= 403,
	ROOM_LIST	= 404,
	FORCE_JOIN	= 405,

	// client messages
	CLIENT_IDLE	= 500,
	CLIENT_AWAY	= 501,
	CLIENT_BACK	= 502,
	CLIENT_CONFIG	= 600,

	// global broadcast
	WALL_MESSAGE	= 601,

	// more client messages
	START_TYPING	= 602,
	STOP_TYPING	= 603,
	RESET_TYPING	= 604
};

#endif // MESSAGE_CODES_H
