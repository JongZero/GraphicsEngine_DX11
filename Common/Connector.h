#pragma once
#include <shared_mutex>
#include "ObjectPool.h"
#include "SafeQueue.h"

class Connector
{
public:
	Connector(unsigned short port, std::string ip);
	~Connector();

public:
	BOOL		Send(PacketHeader* packet);
	BOOL		Receive(PacketHeader**& networkMessageArray, int& arraySize);
	BOOL		Disconnect();

private:
	BOOL		Run();
	BOOL		Stop();
	void		Update();
	BOOL		CreateThread();

	void		ProcessConnect(SOverlapped* psOverlapped);	// ���� ���� ����
	void		ProcessSend(DWORD dwNumberOfBytesTransferred, SOverlapped* psOverlapped);
	void		ProcessReceive(DWORD dwNumberOfBytesTransferred, SOverlapped* psOverlapped);
	
	void		DoReceive(SOverlapped* psOverlapped = nullptr);
	void		AddNetworkMessage(PacketHeader* packet);

private:
	SOCKET		m_ConnectorSocket;	// ����� ����
	HANDLE		m_hIOCP;			// IOCP �ڵ�

	LPFN_CONNECTEX				m_lpfnConnectEx;
	bool						m_IsConnect;

	int							m_ThreadCount;
	std::thread**				m_WorkThreadArray;
	std::shared_mutex			m_WorkMutex;
	bool						m_IsRun;

	ObjectPool<SOverlapped>		m_sOverlappedPool;
	SafeQueue<PacketHeader*>	m_NetworkMessageQueue;
};

