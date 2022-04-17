#pragma once
#include "NetworkStructDefine.h"

enum class eLoginType
{
	S2C_READY,

	C2S_IDCHECK_REQ,
	S2C_IDCHECK_RES,

	C2S_SIGNUP_REQ,
	S2C_SIGNUP_RES,

	C2S_LOGIN_REQ,
	S2C_LOGIN_RES,

	S2S_LOGOUT,

	S2C_USERINFO,
	S2C_FRIENDLIST,
	S2C_FRIENDNOTI,
};

struct S2C_ReadyPacket : public PacketHeader
{
	S2C_ReadyPacket()
	{
		packetType = (unsigned short)eLoginType::S2C_READY;
		packetSize = sizeof(*this) - sizeof(packetSize);
	}
};

struct C2S_LoginPacket : public PacketHeader
{
	C2S_LoginPacket()
	{
		packetType = (unsigned short)eLoginType::C2S_LOGIN_REQ;
		packetSize = sizeof(*this) - sizeof(packetSize);

		ZeroMemory(id, sizeof(id));
		ZeroMemory(password, sizeof(password));
	}

	char	id[21];
	char	password[21];
};

struct S2C_LoginPacket : public PacketHeader
{
	S2C_LoginPacket()
	{
		packetType = (unsigned short)eLoginType::S2C_LOGIN_RES;
		packetSize = sizeof(*this) - sizeof(packetSize);

		uniqueID = -1;
	}

	int		uniqueID;		// 로그인 성공시 -1이 아닌 식별자가 부여
};

struct C2S_SignupPacket : public PacketHeader
{
	C2S_SignupPacket()
	{
		packetType = (unsigned short)eLoginType::C2S_SIGNUP_REQ;
		packetSize = sizeof(*this) - sizeof(packetSize);

		ZeroMemory(name, sizeof(name));
		ZeroMemory(birth, sizeof(birth));
		ZeroMemory(email, sizeof(email));
		ZeroMemory(id, sizeof(id));
		ZeroMemory(password, sizeof(password));
	}

	char name[30];
	char sex;			//0:남 ,1:여
	char birth[11];		//yyyy-mm-dd
	char email[50];		//aaaa@gmail.com
	char id[21];
	char password[21];
};

struct S2C_SignupPacket : public PacketHeader
{
	S2C_SignupPacket()
	{
		packetType = (unsigned short)eLoginType::S2C_SIGNUP_RES;
		packetSize = sizeof(*this) - sizeof(packetSize);

		result = false;
	}

	bool result;
};

struct C2S_IDCheckPacket : public PacketHeader
{
	C2S_IDCheckPacket()
	{
		packetType = (unsigned short)eLoginType::C2S_IDCHECK_REQ;
		packetSize = sizeof(*this) - sizeof(packetSize);

		ZeroMemory(id, sizeof(id));
	}

	char	id[21];
};

struct S2C_IDCheckPacket : public PacketHeader
{
	S2C_IDCheckPacket()
	{
		packetType = (unsigned short)eLoginType::S2C_IDCHECK_RES;
		packetSize = sizeof(*this) - sizeof(packetSize);

		result = false;
	}
	
	bool	result;
};