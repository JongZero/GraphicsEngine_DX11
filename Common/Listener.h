#pragma once
#include <shared_mutex>
#include "ObjectPool.h"
#include "SafeQueue.h"
/// <summary>
/// [2021-12-14][�ֹΰ�]
/// �ٸ� Ŀ���͵��� ������ �޴� ������
/// </summary>
class Listener
{
public:
	Listener(unsigned short port, unsigned int headCount);
	~Listener();

public:
	BOOL		Send(SSocket* psSocket, PacketHeader* packet);
	BOOL		Receive(std::pair<SSocket*, PacketHeader*>*& networkMessageArray, int& arraySize);
	BOOL		DoDisconnect(SOCKET socket, SOverlapped* psOverlapped = nullptr);			// ������ ����(���� ����) ����

private:
	BOOL		RegisterExFunctionPointer(GUID guid, LPVOID lpfnEx);
	BOOL		BindListenSocket(unsigned short port);

	BOOL		Run();
	BOOL		Stop();
	void		Update();			// Work Function
	BOOL		CreateThread();
	void		DoAccept(SOverlapped* psOverlapped);
	void		DoReceive(SOCKET socket, SOverlapped* psOverlapped = nullptr);

	void		AddNetworkMessage(SSocket* psSocket, PacketHeader* packet);
	BOOL		AddUserList(SSocket* psSocket);

	SSocket*	FindUserSocket(SOCKET socket);

	//void		CheckTimeWaitSocket();		// ������ ���� ���� TIME_WAIT�� ���� �ٽ� ������ ���� �� �ִ� ���� üũ
	//void		CreateTimeWaitThread();
	//BOOL		IsTimeWaitSocketExist();	// ���� ���Ḧ ��ٸ��� ������ �ִ��� Ȯ���ϴ� �Լ�

protected:
	void		ProcessAccept(SOverlapped* psOverlapped);		// ���� ��û ����
	void		ProcessDisconnect(SOverlapped* psOverlapped);	// ���� ���� ����
	void		ProcessSend(DWORD dwNumberOfBytesTransferred, SOverlapped* psOverlapped, SSocket* psSocket);
	void		ProcessReceive(DWORD dwNumberOfBytesTransferred, SOverlapped* psOverlapped, SSocket* psSocket);
	
public:
	std::vector<SSocket*>	m_UserVec;	// socket ����
	
private:
	SOCKET			m_ListenSocket;	// ���� ����
	HANDLE			m_hIOCP;		// IOCP �ڵ�
	unsigned int	m_HeadCount;

	int							m_ThreadCount;
	bool						m_IsRun;				// �۾� ������ ���� �÷���
	std::thread**				m_WorkThreadArray;		// �۾� ������ �迭
	mutable std::shared_mutex	m_WorkMutex;

	LPFN_ACCEPTEX				m_lpfnAcceptEx;
	LPFN_DISCONNECTEX			m_lpfnDisconnectEx;
	LPFN_GETACCEPTEXSOCKADDRS	m_lpfnGetAcceptExSockAddrs;

	ObjectPool<SSocket>		m_sSocketPool;
	ObjectPool<SOverlapped>	m_sOverlappedPool;

	SafeQueue<std::pair<SSocket*, PacketHeader*>>	m_NetworkMessageQueue;
	
	/*std::thread							m_TimeWaitThread;
	std::vector<std::pair<DWORD, SSocket*>>	m_WaitSocketVec;
	bool									m_IsTimeWait;
	mutable std::shared_mutex				m_TimeWaitMutex;*/
};

