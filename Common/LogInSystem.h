#pragma once

#ifdef LOGINSYSTEM_EXPORTS
#define LOGINSYSTEM_API __declspec(dllexport)
#else
#define LOGINSYSTEM_API __declspec(dllimport)
#endif

#include "LogInSystemErrorCode.h"

// 전방 선언
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
	// 로그인 서버를 오픈하는 함수
	LOGINSYSTEM_API void OpenLogInServer(unsigned short portNumber, unsigned int headCount);
	// 로그인 서버를 종료하는 함수
	LOGINSYSTEM_API void CloseLogInServer();

	// 로그인 서버와 게임 서버를 연결해주는 함수
	LOGINSYSTEM_API void ConnectGameServer(unsigned short portNumber, const char* ipAddress);
	// 로그인 서버 게임 서버의 연결을 종료하는 함수
	LOGINSYSTEM_API void DisconnectGameServer();

	// 로그인 DB 연결 함수
	LOGINSYSTEM_API void ConnectLogInDB(const char* hostName, const char* userId, const char* password, const char* dbName, unsigned short port);
#pragma region Getter
public:
	// 로그인 서버 변경 사항 이벤트 Get
	LOGINSYSTEM_API HANDLE GetChangeEventHandle();
	// 로그인 서버 접속 유저 수 Get
	LOGINSYSTEM_API int GetConnectedUserCount();
	// 로그인 서버 접속 유저의 ID Get
	LOGINSYSTEM_API const char* GetConnectedUserID(int index);
	// 로그인 서버 접속 유저의 IP Get
	LOGINSYSTEM_API const char* GetConnectedUserIP(int index);
	// 로그인 서버 접속 유저의 접속 시각 Get
	LOGINSYSTEM_API const char* GetConnectedUserDate(int index);
#pragma endregion
#pragma endregion

#pragma region Client API
public:
	// 로그인 서버에 연결하는 함수
	LOGINSYSTEM_API void ConnectLogInServer(unsigned short portNumber, const char* ipAddress);
	// 로그인을 시도하는 함수
	LOGINSYSTEM_API void LogIn(const char* id, const char* password);
	// 회원가입을 시도하는 함수
	LOGINSYSTEM_API void SignUp(const char* id, const char* password, ...);
	// ID를 찾는 함수
	LOGINSYSTEM_API void FindID(PVOID userInfo);
	// 비밀번호를 찾는 함수
	LOGINSYSTEM_API void FindPassword(const char* id, ...);
#pragma endregion

#pragma region System 내부 함수
public:
	void	InitializeLogInSystem(eLogInSystemType type);

public:
	// 함수 사용 신뢰성 체크( 사용 가능한 상태 혹은 사용 가능한 타입인지 )
	bool	CheckReliability(eLogInSystemType checkType);
	
private:
	const eLogInSystemType	m_eLogInSystemType;
	LogInObject*			m_pLogInObject;
#pragma endregion
};

