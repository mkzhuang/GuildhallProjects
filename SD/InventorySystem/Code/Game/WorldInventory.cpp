#include "Game/WorldInventory.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Inventory.hpp"
#include "Game/Item.hpp"
#include "Game/ItemFactory.hpp"
#include "Game/DatabaseClient.hpp"
#include "Engine/Mesh/Mesh.hpp"

WorldInventory::WorldInventory(InventorySystemConfig config)
{
	m_renderer = config.renderer;
	m_playerInventory = config.inventory;
	m_inventoryID = config.inventoryID;
	m_worldInventory = new Inventory();
}


WorldInventory::~WorldInventory()
{
}


void WorldInventory::Update(float deltaSeconds)
{
	UNUSED (deltaSeconds)
}


void WorldInventory::Render() const
{
	// m_renderer->ClearDepth();
	for (int index = 0; index < (int)m_worldInventory->m_items.size(); index++)
	{
		Item* item = m_worldInventory->m_items[index];
		Mesh* mesh = m_renderer->GetMeshForName(item->GetValueAsString("name").c_str());
		Vec2 const& worldPos = m_itemPos[index];
		mesh->m_modelMatrix = Mat44();
		mesh->m_modelMatrix.AppendTranslation3D(Vec3(worldPos.x, worldPos.y, 0.f));
		mesh->m_modelMatrix.AppendScaleUniform3D(0.05f);
		mesh->Render(m_renderer);
	}
}


void WorldInventory::LoadItemsFromJsonString(std::string const& itemString)
{
	Strings itemCount = SplitStringOnDelimiter(itemString, ',');
	for (std::string itemID : itemCount)
	{
		AddToWorld(ItemFactory::CreateById((uint16_t)std::atoi(itemID.c_str())));
	}
}


void WorldInventory::LoadItemPosFromJsonString(std::string const& posString)
{
	Strings positions = SplitStringOnDelimiter(posString, ',');
	for (std::string position : positions)
	{
		Strings pos = SplitStringOnDelimiter(position, '-');
		pos = SplitStringOnDelimiter(pos[1], ':');
		float posX = (float)std::atof(pos[0].c_str());
		float posY = (float)std::atof(pos[1].c_str());
		m_itemPos.emplace_back(posX, posY);
	}
}


void WorldInventory::AddFromPlayerInventory(int index, Vec2 const& itemPos)
{
	Item* item = m_playerInventory->RemoveItemFromInventory(index);
	m_worldInventory->AddItemToInventory(item);
	AddWorldLocation(itemPos);
	g_theClient->RemoveItemFromInventory(m_playerInventory->m_inventoryID, std::to_string(item->GetID()));
	g_theClient->AddItemToInventory(m_inventoryID, std::to_string(item->GetID()));
	std::string posString = Stringf("%d-%.1f:%.1f", item->GetID(), itemPos.x, itemPos.y);
	g_theClient->AddItemWorldLocation(m_inventoryID, posString);
}


void WorldInventory::RemoveFromWorldInventory(int index)
{
	Item* item = m_worldInventory->RemoveItemFromInventory(index);
	m_playerInventory->AddItemToInventory(item);
	Vec2 const& itemPos = RemoveWorldLocation(index);
	g_theClient->RemoveItemFromInventory(m_inventoryID, std::to_string(item->GetID()));
	g_theClient->AddItemToInventory(m_playerInventory->m_inventoryID, std::to_string(item->GetID()));
	std::string posString = Stringf("%d-%.1f:%.1f", item->GetID(), itemPos.x, itemPos.y);
	g_theClient->RemoveItemWorldLocation(m_inventoryID, posString);
}


void WorldInventory::AddToWorld(Item* item)
{
	m_worldInventory->AddItemToInventory(item);
}


void WorldInventory::AddWorldLocation(Vec2 const& pos)
{
	m_itemPos.push_back(pos);
}


Vec2 const& WorldInventory::RemoveWorldLocation(int index)
{
	Vec2 const& pos = m_itemPos[index];
	m_itemPos.erase(m_itemPos.begin() + index);
	return pos;
}


