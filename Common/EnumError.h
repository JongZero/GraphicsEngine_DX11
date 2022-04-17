#pragma once
/// <summary>
/// [2021-12-07][�ֹΰ�]
/// ���� �߻� �ÿ� throw�Ǵ� �����ڵ��� ���� ����
/// </summary>
enum class ErrorCode
{
	ObjectPool_NumberIsZero,		// ������ƮǮ �����ÿ� ���� ������ 0, Ȥ�� 0 ������ ���� ������ ��

	Network_WSAStartup,				// Network - WSAStartup ����
	Network_AlreadyExistName,		// Network - �̹� ��ϵ� �̸��� Ŀ����/������
	Network_NotExist,				// Network - ��ϵ��� ���� Ŀ����/������ ã��

	Listener_hIOCPCreate,			// Listener - IOCP �ڵ� ���� ����
	Listener_CreateListenSocket,	// Listener - ���� ���� ���� ����
	Listener_BindListenSocket,		// Listener - ���� ���� ���ε� ����
	Listener_GetExFunction,			// Listener - Ex �Լ��� �������µ��� ����
	Listener_ArrayCreate,			// Listener - ��Ʈ��ũ �޼��� �迭 ���� ����
	Listener_SocketCreate,			// Listener - ���� ���� ����
	Listener_AcceptEx,				// Listener - AcceptEx ����
	Listener_DisconnectEx,			// Listener - DisconnectEx ����
	Listener_WSAReceive,			// Listener - WSAReceive ����
	Listener_WSASend,				// Listener - WSASend ����
	Listener_Listen,				// Listener - Listen ����
	Listener_OverBuffer,			// Listener - Buffer���� ū ��Ŷ ���� �õ�

	Connector_hIOCPCreate,			// Connector - IOCP �ڵ� ���� ����
	Connector_ArrayCreate,			// Connector - ��Ʈ��ũ �޼��� �迭 ���� ����
	Connector_CreateConnectSocket,	// Connector - Ŀ���� ���� ���� ����
	Connector_BindConnectSocket,	// Connector - Ŀ���� ���� ���ε� ����
	Connector_GetExFunction,		// Connector - Ex �Լ��� �������µ��� ����
};