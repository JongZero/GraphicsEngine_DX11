#pragma once
#include <string>
#include <unordered_map>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include "NetworkStructDefine.h"

// ���� ����
class Connector;
class Listener;

/// <summary>
/// [2021-12-06][�ֹΰ�]
/// Ŀ���Ϳ� �����ʸ� �����ϴ� ��Ʈ��ũ Ŭ����
/// </summary>
class Network
{
public:
	Network();
	~Network();

#pragma region �ܺο��� Network Ŭ������ ����� �� ������ API
public:
	// name : ������ Ŀ���͸� ������ �̸�, port : ������ Ŀ������ ��Ʈ ��ȣ, ip : ������ Ŀ������ ip �ּ�
	BOOL CreateConnector(std::string connectorName, unsigned short port, std::string ip);
	// name : ������ �����ʸ� ������ �̸�, port : ������ �������� ��Ʈ ��ȣ, headCount : �����ʿ��� ������ �ο� ����
	BOOL CreateListener(std::string listenerName, unsigned short port, unsigned int headCount);

	// connectorName : �ҷ��� Ŀ������ �̸�, packet : ���� ��Ŷ�� ������
	BOOL ConnectorSend(std::string connectorName, PacketHeader* packet);
	// connectorName : �ҷ��� Ŀ������ �̸�, networkMessageArray : <SSocket*, packet>�� �迭, arraySize : �迭�� ũ��
	BOOL ConnectorReceive(std::string connectorName, PacketHeader**& networkMessageArray, int& arraySize);
	// connectorName : ������ ������ Ŀ������ �̸�
	BOOL ConnectorDisconnect(std::string connectorName);

	// listenerName : �ҷ��� �������� �̸�, psSocket : ��� socket, packet : ���� ��Ŷ�� ������
	BOOL ListenerSend(std::string listenerName, SSocket* psSocket, PacketHeader* packet);
	// listenerName : �ҷ��� �������� �̸�, networkMessageArray : <SSocket*, packet>�� �迭, arraySize : �迭�� ũ��
	BOOL ListenerReceive(std::string listenerName, std::pair<SSocket*, PacketHeader*>*& networkMessageArray, int& arraySize);
	// listenerName : ������ ������ �������� �̸�, socket : ��� socket
	BOOL ListenerDisconnect(std::string listenerName, SOCKET socket);
	BOOL ListenerDisconnect(std::string listenerName, SSocket* psSocket);
	
	Listener*	FindListener(std::string name);
	Connector*	FindConnector(std::string name);
#pragma endregion

#pragma region Network Ŭ���� ���ο��� ����� ��� �Լ�
public:

#pragma endregion


#pragma region Network Ŭ�������� �ʿ��� ��� ����
private:
	WSADATA		m_WSAData;		// ���� ������ (�� �ѹ��� ���� �� ����)

	std::unordered_map<std::string, Listener*>		m_ListenerUMap;
	std::unordered_map<std::string, Connector*>		m_ConnectorUMap;
#pragma endregion

};

