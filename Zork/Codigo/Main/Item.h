#pragma once

#include "Entity.h"

#include <memory>
#include <iosfwd>
#include <string>
#include <vector>

class Item : public Entity
{
public:

	Item(const std::string& id, const std::string& name, const std::string& description);

	/*
	*  Gameplay attributes GET/SET
	*/
	bool IsContainer() const;
	void SetContainer(bool container);

	bool IsLightSource() const;
	void SetLightSource(bool lightSource);

	bool IsWeapon() const;
	void SetWeapon(bool weapon);

	/*
	*  Helpers to match the user input to actual items
	*  Aliases system to soften the matching
	*/
	const std::vector<std::string>& GetAliases() const;
	void AddAlias(const std::string& alias);
	bool MatchesTarget(const std::string& target) const;

	/*
	*  Prints to the output the info of the item:
	* - Name
	* - Description
	* - Names of the items inside, if any
	*/ 
	void PrintInformation(std::ostream& output) const override;

	void AddItem(const std::shared_ptr<Item>& item);
	std::shared_ptr<Item> RemoveItem(const std::string& itemId);
	const std::vector<std::shared_ptr<Item>>& GetContainedItems() const;
	Item* FindItem(const std::string& target);
	const Item* FindItem(const std::string& target) const;

private:

	/*
	*  Helpers to match the user input to actual items
	*/
	static std::string NormalizeTarget(std::string text);
	static bool MatchesNormalizedText(const std::string& text, const std::string& normalizedTarget);

	bool m_isContainer;
	bool m_isLightSource;
	bool m_isWeapon;

	std::vector<std::string> m_aliases;
	std::vector<std::shared_ptr<Item>> m_containedItems;
};
