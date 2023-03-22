#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <string>
#include <vector>
#include <mongocxx/client.hpp>
#include <mongocxx/pool.hpp>

enum class QueryType
{
	Login,
	Get,
	Validate,
	GetInventory,
	GetWorldLocation,
	AddWorldLocation,
	RemoveWorldLocation,
	Add,
	Remove,
};

class DBConn
{
public:
	DBConn(QueryType type, EventArgs& args);
	DBConn();
	~DBConn() {};

	void ExecuteQuery(QueryType type, EventArgs& args);
	std::string const& Result();
	void ClearResult();

public:
	std::string m_result;
	std::unique_ptr<mongocxx::pool> m_pool = nullptr;
};