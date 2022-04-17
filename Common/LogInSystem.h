#pragma once

#ifdef LOGINSYSTEM_EXPORTS
#define LOGINSYSTEM_API __declspec(dllexport)
#else
#define LOGINSYSTEM_API __declspec(dllimport)
#endif

#include "LogInSystemErrorCode.h"

// ���� ����
class LogInObject;

enum class eLogInSystemType
{
	LogInServer,
	LogInClient
};

class LogInSystem
{
public:
	LOGINSYSTEM_API LogInSystem(eLogInSystemType type);
	~LogInSystem();

#pragma region Server API
public:
	// �α��� ������ �����ϴ� �Լ�
	LOGINSYSTEM_API void OpenLogInServer(unsigned short portNumber, unsigned int headCount);
	// �α��� ������ �����ϴ� �Լ�
	LOGINSYSTEM_API void CloseLogInServer();

	// �α��� ������ ���� ������ �������ִ� �Լ�
	LOGINSYSTEM_API void ConnectGameServer(unsigned short portNumber, const char* ipAddress);
	// �α��� ���� ���� ������ ������ �����ϴ� �Լ�
	LOGINSYSTEM_API void DisconnectGameServer();

	// �α��� DB ���� �Լ�
	LOGINSYSTEM_API void ConnectLogInDB(const char* hostName, const char* userId, const char* password, const char* dbName, unsigned short port);
#pragma region Getter
public:
	// �α��� ���� ���� ���� �̺�Ʈ Get
	LOGINSYSTEM_API HANDLE GetChangeEventHandle();
	// �α��� ���� ���� ���� �� Get
	LOGINSYSTEM_API int GetConnectedUserCount();
	// �α��� ���� ���� ������ ID Get
	LOGINSYSTEM_API const char* GetConnectedUserID(int index);
	// �α��� ���� ���� ������ IP Get
	LOGINSYSTEM_API const char* GetConnectedUserIP(int index);
	// �α��� ���� ���� ������ ���� �ð� Get
	LOGINSYSTEM_API const char* GetConnectedUserDate(int index);
#pragma endregion
#pragma endregion

#pragma region Client API
public:
	// �α��� ������ �����ϴ� �Լ�
	LOGINSYSTEM_API void ConnectLogInServer(unsigned short portNumber, const char* ipAddress);
	// �α����� �õ��ϴ� �Լ�
	LOGINSYSTEM_API void LogIn(const char* id, const char* password);
	// ȸ�������� �õ��ϴ� �Լ�
	LOGINSYSTEM_API void SignUp(const char* id, const char* password, ...);
	// ID�� ã�� �Լ�
	LOGINSYSTEM_API void FindID(PVOID userInfo);
	// ��й�ȣ�� ã�� �Լ�
	LOGINSYSTEM_API void FindPassword(const char* id, ...);
#pragma endregion

#pragma region System ���� �Լ�
public:
	void	InitializeLogInSystem(eLogInSystemType type);

public:
	// �Լ� ��� �ŷڼ� üũ( ��� ������ ���� Ȥ�� ��� ������ Ÿ������ )
	bool	CheckReliability(eLogInSystemType checkType);
	
private:
	const eLogInSystemType	m_eLogInSystemType;
	LogInObject*			m_pLogInObject;
#pragma endregion
};

