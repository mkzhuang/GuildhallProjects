#include "Game/DBConn.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include <string>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/options/client.hpp>
#include <bsoncxx/json.hpp>
#include "bson/bson.h"
#include <bsoncxx/builder/stream/document.hpp>

typedef bsoncxx::string::view_or_value bson_string;
//typedef bsoncxx::stdx::optional<bsoncxx::document::value> mongo_document_value;
typedef bsoncxx::stdx::optional<mongocxx::result::update> query_update_result;

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::finalize;

DBConn::DBConn(QueryType type, EventArgs& args) {
	//auto uri_string = "mongodb+srv://test:test123@inventorysystem.0hlfjyh.mongodb.net/?retryWrites=true&w=majority";
	auto uri_string = "mongodb://localhost:27017";
	mongocxx::uri uri(static_cast<bson_string>(uri_string));
	mongocxx::options::client client_options;
	mongocxx::options::server_api api = mongocxx::options::server_api::version::k_version_1;
	client_options.server_api_opts(api);
	mongocxx::client client(uri, client_options);

	mongocxx::database db = client[static_cast<bson_string>("InventorySystem")];
	
	switch (type)
	{
		case QueryType::Login:
		{
			mongocxx::collection collection = db[static_cast<bson_string>("User")];
			std::string user = args.GetValue("user", "");
			std::string password = args.GetValue("password", "");
			mongocxx::cursor cursor = collection.find(document{} << "_id" << user.c_str() << "password" << password.c_str() << finalize);
			for (bsoncxx::document::view doc : cursor)
			{
				std::string doc_string;
				bson_t bson;
				bson_init_static(&bson, doc.data(), doc.length());
				size_t size;
				doc_string = bson_as_json(&bson, &size);
				m_result.append(Stringf("%s\n", doc_string.c_str()));
				DebuggerPrintf("%s\n", doc_string.c_str());
				bson_destroy(&bson);
			}
			break;
		}

		case QueryType::Get:
		{
			std::string collectionName = args.GetValue("collection", "");
			mongocxx::collection collection = db[static_cast<bson_string>(collectionName.c_str())];
			mongocxx::cursor cursor = collection.find({});
			for (bsoncxx::document::view doc : cursor)
			{
				std::string doc_string;
				bson_t bson;
				bson_init_static(&bson, doc.data(), doc.length());
				size_t size;
				doc_string = bson_as_json(&bson, &size);
				m_result.append(Stringf("%s\n", doc_string.c_str()));
				DebuggerPrintf("%s\n", doc_string.c_str());
				bson_destroy(&bson);
			}
			break;
		}

		case QueryType::Validate:
		{
			break;
		}

		case QueryType::GetInventory:
		{
			mongocxx::collection collection = db[static_cast<bson_string>("Inventory")];
			std::string inventoryID = args.GetValue("inventoryID", "");
			mongocxx::cursor cursor = collection.find(document{} << "_id" << inventoryID.c_str() << finalize);
			for (bsoncxx::document::view doc : cursor)
			{
				std::string doc_string;
				bson_t bson;
				bson_init_static(&bson, doc.data(), doc.length());
				size_t size;
				doc_string = bson_as_json(&bson, &size);
				m_result.append(Stringf("%s\n", doc_string.c_str()));
				DebuggerPrintf("%s\n", doc_string.c_str());
				bson_destroy(&bson);
			}
			break;
		}

		case QueryType::Add:
		{
			mongocxx::collection collection = db[static_cast<bson_string>("Inventory")];
			std::string inventoryID = args.GetValue("inventoryID", "");
			std::string itemID = args.GetValue("itemID", "");
			std::string itemString = "items." + itemID;
			query_update_result result = collection.update_one(
				document{} << "_id" << inventoryID.c_str() << finalize,
				document{} << "$inc" << open_document << (bsoncxx::stdx::string_view)itemString << 1 << close_document << finalize);
			m_result = Stringf("%d", result->modified_count());
			break;
		}

		case QueryType::Remove:
		{
			mongocxx::collection collection = db[static_cast<bson_string>("Inventory")];
			std::string inventoryID = args.GetValue("inventoryID", "");
			std::string itemID = args.GetValue("itemID", "");
			std::string itemString = "items." + itemID;
			query_update_result result = collection.update_one(
				document{} << "_id" << inventoryID.c_str() << finalize,
				document{} << "$inc" << open_document << (bsoncxx::stdx::string_view)itemString << -1 << close_document << finalize);
			m_result = Stringf("%d", result->modified_count());
			break;
		}
	}
};


DBConn::DBConn()
{
	auto uri_string = "mongodb://localhost:27017/?retryWrites=true&w=majority&autoReconnect=true&socketTimeoutMS=0&connectTimeoutMS=0";
	mongocxx::uri uri(static_cast<bson_string>(uri_string));
	m_pool = std::move(bsoncxx::stdx::make_unique<mongocxx::pool>(std::move(uri)));
}


void DBConn::ExecuteQuery(QueryType type, EventArgs& args)
{
	ClearResult();
	auto client = m_pool->acquire();
	auto database = (*client)[static_cast<bson_string>("InventorySystem")];
	switch (type)
	{
		case QueryType::Login:
		{
			mongocxx::collection collection = database[static_cast<bson_string>("User")];
			std::string user = args.GetValue("user", "");
			std::string password = args.GetValue("password", "");
			mongocxx::cursor cursor = collection.find(document{} << "_id" << user.c_str() << "password" << password.c_str() << finalize);
			for (bsoncxx::document::view doc : cursor)
			{
				std::string doc_string;
				bson_t bson;
				bson_init_static(&bson, doc.data(), doc.length());
				size_t size;
				doc_string = bson_as_json(&bson, &size);
				m_result.append(Stringf("%s\n", doc_string.c_str()));
				DebuggerPrintf("%s\n", doc_string.c_str());
				bson_destroy(&bson);
			}
			break;
		}

		case QueryType::Get:
		{
			std::string collectionName = args.GetValue("collection", "");
			mongocxx::collection collection = database[static_cast<bson_string>(collectionName.c_str())];
			mongocxx::cursor cursor = collection.find({});
			for (bsoncxx::document::view doc : cursor)
			{
				std::string doc_string;
				bson_t bson;
				bson_init_static(&bson, doc.data(), doc.length());
				size_t size;
				doc_string = bson_as_json(&bson, &size);
				m_result.append(Stringf("%s\n", doc_string.c_str()));
				DebuggerPrintf("%s\n", doc_string.c_str());
				bson_destroy(&bson);
			}
			break;
		}

		case QueryType::Validate:
		{
			break;
		}

		case QueryType::GetInventory:
		{
			mongocxx::collection collection = database[static_cast<bson_string>("Inventory")];
			std::string inventoryID = args.GetValue("inventoryID", "");
			mongocxx::cursor cursor = collection.find(document{} << "_id" << inventoryID.c_str() << finalize);
			for (bsoncxx::document::view doc : cursor)
			{
				std::string doc_string;
				bson_t bson;
				bson_init_static(&bson, doc.data(), doc.length());
				size_t size;
				doc_string = bson_as_json(&bson, &size);
				m_result.append(Stringf("%s\n", doc_string.c_str()));
				DebuggerPrintf("%s\n", doc_string.c_str());
				bson_destroy(&bson);
			}
			break;
		}

		case QueryType::GetWorldLocation:
		{
			mongocxx::collection collection = database[static_cast<bson_string>("WorldItemLocation")];
			std::string inventoryID = args.GetValue("inventoryID", "");
			mongocxx::cursor cursor = collection.find(document{} << "_id" << inventoryID.c_str() << finalize);
			for (bsoncxx::document::view doc : cursor)
			{
				std::string doc_string;
				bson_t bson;
				bson_init_static(&bson, doc.data(), doc.length());
				size_t size;
				doc_string = bson_as_json(&bson, &size);
				m_result.append(Stringf("%s\n", doc_string.c_str()));
				DebuggerPrintf("%s\n", doc_string.c_str());
				bson_destroy(&bson);
			}
			break;
		}

		case QueryType::AddWorldLocation:
		{
			mongocxx::collection collection = database[static_cast<bson_string>("WorldItemLocation")];
			std::string inventoryID = args.GetValue("inventoryID", "");
			std::string location = args.GetValue("location", "");
			query_update_result result = collection.update_one(
				document{} << "_id" << inventoryID.c_str() << finalize,
				document{} << "$push" << open_document << "items" << location << close_document << finalize);
			m_result = Stringf("%d", result->modified_count());
			break;
		}

		case QueryType::RemoveWorldLocation:
		{
			mongocxx::collection collection = database[static_cast<bson_string>("WorldItemLocation")];
			std::string inventoryID = args.GetValue("inventoryID", "");
			std::string location = args.GetValue("location", "");
			//uint16_t item = (uint16_t)std::atoi(location.c_str());
			query_update_result result = collection.update_one(
				document{} << "_id" << inventoryID.c_str() << finalize,
				document{} << "$pull" << open_document << "items" << location << close_document << finalize);
			m_result = Stringf("%d", result->modified_count());
			break;
		}

		case QueryType::Add:
		{
			mongocxx::collection collection = database[static_cast<bson_string>("Inventory")];
			std::string inventoryID = args.GetValue("inventoryID", "");
			std::string itemID = args.GetValue("itemID", "");
			uint16_t item = (uint16_t)std::atoi(itemID.c_str());
			query_update_result result = collection.update_one(
				document{} << "_id" << inventoryID.c_str() << finalize,
				document{} << "$push" << open_document << "items" << item << close_document << finalize);
			m_result = Stringf("%d", result->modified_count());
			break;
		}

		case QueryType::Remove:
		{
			mongocxx::collection collection = database[static_cast<bson_string>("Inventory")];
			std::string inventoryID = args.GetValue("inventoryID", "");
			std::string itemID = args.GetValue("itemID", "");
			uint16_t item = (uint16_t)std::atoi(itemID.c_str());
			query_update_result result = collection.update_one(
				document{} << "_id" << inventoryID.c_str() << finalize,
				document{} << "$pull" << open_document << "items" << item << close_document << finalize);
			m_result = Stringf("%d", result->modified_count());
			break;
		}
	}
}


std::string const& DBConn::Result()
{
	return m_result;
}


void DBConn::ClearResult()
{
	m_result = "";
}


