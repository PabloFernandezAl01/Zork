#pragma once

#include "Entity.h"

#include <memory>
#include <vector>

class Item : public Entity
{
public:

	Item(const std::string& id, const std::string& name, const std::string& description);

	bool IsContainer() const;
	bool IsLightSource() const;
	bool IsWeapon() const;

	void SetContainer(bool container);
	void SetLightSource(bool lightSource);
	void SetWeapon(bool weapon);

	const std::vector<std::shared_ptr<Item>>& GetContainedItems() const;
	void AddItem(const std::shared_ptr<Item>& item);

private:

	bool m_isContainer;
	bool m_isLightSource;
	bool m_isWeapon;

	std::vector<std::shared_ptr<Item>> m_containedItems;
};
