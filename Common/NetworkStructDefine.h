#pragma once
#include <string>
#include <WS2tcpip.h>
#include <MSWSock.h>

struct SOverlapped
{
	enum class eIOType
	{
		eAccept,
		eConnect,
		eDisconnect,

		eReceive,
		eSend,

		eMax
	};

	SOverlapped()
	{
		ZeroMemory(&wsaOverlapped, sizeof(wsaOverlapped));
		socket = INVALID_SOCKET;
		iDataSize = 0;
		ZeroMemory(buffer, 1024);
	}

	WSAOVERLAPPED	wsaOverlapped;
	eIOType			eType;

	SOCKET			socket;
	char			buffer[1024];
	int				iDataSize;
};

struct SSocket
{
	SSocket()
	{
		socket = INVALID_SOCKET;
		port = 0;
	}

	SOCKET			socket;
	std::string		ip;
	unsigned short	port;
};

struct PacketHeader
{
	unsigned short packetSize;
	unsigned short packetType;
};

struct Packet : public PacketHeader
{
	Packet()
	{
		packetSize = sizeof(*this) - sizeof(packetSize);
		packetType = 0;
	}
};

struct BigPacket : public PacketHeader
{
	BigPacket(int id, int order, unsigned int totalSize)
	{
		this->id = id;
		this->orderNumber = order;
		this->packetSize = totalSize;

		this->buffer = new char[totalSize] {0, };
		nowSize = 0;
	}
	~BigPacket()
	{
		delete[] buffer;
		buffer = nullptr;
	}

	std::string		userId;			// ���� ���� ID
	int				id;				// ���� �ĺ���
	int				orderNumber;	// ����
	unsigned int	packetSize;		// �� ��Ŷ ������
	unsigned int	nowSize;		// ������� ���� ��Ŷ ������(buffer Pointer)
	char*			buffer;			// ����
};