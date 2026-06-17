#pragma once

#include "Entity.h"
#include "Item.h"

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

/*
* This class represents the player. It has the information of the room where the player is
* and the list of items the player has. 
* 
* PrintInformation overrides Entity adding the information related to all the names of the items
* in player's inventory. 
* 
* It adds funcionality to add/remove items from the inventory and to check if an item exists in
* the inventory. 
*/

class Player : public Entity
{
public:

	Player(const std::string& id, const std::string& name, const std::string& description);

	const std::string& GetCurrentRoomId() const;
	const std::vector<std::shared_ptr<Item>>& GetInventory() const;
	Item* FindItem(const std::string& target);
	const Item* FindItem(const std::string& target) const;

	void PrintInformation(std::ostream& output) const override;

	void SetCurrentRoomId(const std::string& roomId);
	void AddItemToInventory(const std::shared_ptr<Item>& item);
	std::shared_ptr<Item> RemoveItemFromInventory(const std::string& itemId);

private:

	std::string m_currentRoomId;
	std::vector<std::shared_ptr<Item>> m_inventory;
};
