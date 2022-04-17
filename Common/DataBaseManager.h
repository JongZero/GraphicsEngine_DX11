#pragma once
#include <unordered_map>
/// <summary>
/// [2021-12-09][�ֹΰ�]
/// DB ���� �� ������ ��û ���� �����ϴ� DB ���� Ŭ����
/// </summary>
class DataBaseManager
{
public:
	DataBaseManager();
	DataBaseManager(const char* hostName, const char* userId, const char* password, const char* dbName, unsigned short port);
	~DataBaseManager();

public:
	void ConnectDB(const char* hostName, const char* userId, const char* password, const char* dbName, unsigned short port);
	void DisconnectDB();

	void QueryDB(void*& result, const char* queryStr, ...);
	void UpdateDB();



private:
	void*	m_pDataBase;
	std::unordered_map<const char*, void*> m_DataBaseTableUMap;
};
