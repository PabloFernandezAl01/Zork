#pragma once

#include "Entity.h"
#include "Item.h"

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

class Player : public Entity
{
public:

	Player(const std::string& id, const std::string& name, const std::string& description);

	const std::string& GetCurrentRoomId() const;
	const std::vector<std::shared_ptr<Item>>& GetInventory() const;

	void PrintInformation(std::ostream& output) const override;

	void SetCurrentRoomId(const std::string& roomId);
	void AddItemToInventory(const std::shared_ptr<Item>& item);
	std::shared_ptr<Item> RemoveItemFromInventory(const std::string& itemId);

private:

	std::string m_currentRoomId;
	std::vector<std::shared_ptr<Item>> m_inventory;
};
