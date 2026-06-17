#include "Item.h"

#include <algorithm>
#include <cctype>
#include <ostream>

Item::Item(const std::string& id, const std::string& name, const std::string& description)
	: Entity(id, name, description)
	, m_isContainer(false)
	, m_isLightSource(false)
	, m_isWeapon(false)
{
}

bool Item::IsContainer() const
{
	return m_isContainer;
}

bool Item::IsLightSource() const
{
	return m_isLightSource;
}

bool Item::IsWeapon() const
{
	return m_isWeapon;
}

void Item::SetContainer(bool container)
{
	m_isContainer = container;
}

void Item::SetLightSource(bool lightSource)
{
	m_isLightSource = lightSource;
}

void Item::SetWeapon(bool weapon)
{
	m_isWeapon = weapon;
}

const std::vector<std::string>& Item::GetAliases() const
{
	return m_aliases;
}

void Item::AddAlias(const std::string& alias)
{
	m_aliases.push_back(alias);
}

void Item::PrintInformation(std::ostream& output) const
{
	Entity::PrintInformation(output);

	if (!m_isContainer)
	{
		return;
	}

	if (m_containedItems.empty())
	{
		output << "Esta vacio.\n";
		return;
	}

	// If the item is a container and it's not empty: print the names of the items inside
	output << "Contiene:\n";
	for (const auto& item : m_containedItems)
		output << "- " << item->GetName() << '\n';
}

const std::vector<std::shared_ptr<Item>>& Item::GetContainedItems() const
{
	return m_containedItems;
}

Item* Item::FindItem(const std::string& target)
{
	if (!m_isContainer)
	{
		return nullptr;
	}

	const auto it = std::find_if(m_containedItems.begin(), m_containedItems.end(),
		[&target](const std::shared_ptr<Item>& item)
		{
			return item->MatchesTarget(target);
		});

	return it != m_containedItems.end() ? it->get() : nullptr;
}

const Item* Item::FindItem(const std::string& target) const
{
	if (!m_isContainer)
	{
		return nullptr;
	}

	const auto it = std::find_if(m_containedItems.begin(), m_containedItems.end(),
		[&target](const std::shared_ptr<Item>& item)
		{
			return item->MatchesTarget(target);
		});

	return it != m_containedItems.end() ? it->get() : nullptr;
}

void Item::AddItem(const std::shared_ptr<Item>& item) 
{
	if (!m_isContainer) 
		return; // <---- GameWorld logic is in charge of checking if an item can contain another items. This return should never be reached. 

	m_containedItems.push_back(item);
}

std::shared_ptr<Item> Item::RemoveItem(const std::string& itemId)
{
	if (!m_isContainer)
		return nullptr; // <---- GameWorld logic is in charge of checking if an item can contain another items. This return should never be reached.

	// Search for the item inside *this* item
	const auto it = std::find_if(m_containedItems.begin(), m_containedItems.end(),
		[&itemId](const std::shared_ptr<Item>& item)
		{
			return item->GetId() == itemId;
		});

	// Didn't find it
	if (it == m_containedItems.end())
		return nullptr;

	// Same as in Room and Player, it just gets removed from the vector
	const auto item = *it;
	m_containedItems.erase(it);

	// The item itself is returned so it can be moved to the player inventory or the room where the player is
	return item;
}

bool Item::MatchesTarget(const std::string& target) const
{
	// First normalize target
	const std::string normalizedTarget = NormalizeTarget(target);
	if (normalizedTarget.empty())
	{
		return false;
	}

	// If it matches de item name
	if (MatchesNormalizedText(GetName(), normalizedTarget))
		return true;

	// Or if it matches any of it's aliases
	return std::find_if(m_aliases.begin(), m_aliases.end(),
		[&normalizedTarget](const std::string& alias)
		{
			return MatchesNormalizedText(alias, normalizedTarget);
		}) != m_aliases.end();
}

/*
Normalizes any text that can identify an item before comparing it with player
input. This keeps matching rules centralized and predictable:

- Removes leading and trailing whitespace.
- Converts uppercase characters to lowercase.

Examples:
- "  Mapa Rasgado  " becomes "mapa rasgado".
- "Caja Fuerte" becomes "caja fuerte".
- "   " becomes "".
*/
std::string Item::NormalizeTarget(std::string text)
{
	const std::string whitespace = " \t\n\r";
	const std::string::size_type firstCharacter = text.find_first_not_of(whitespace);

	if (firstCharacter == std::string::npos)
	{
		return "";
	}

	const std::string::size_type lastCharacter = text.find_last_not_of(whitespace);
	text = text.substr(firstCharacter, lastCharacter - firstCharacter + 1);

	std::transform(text.begin(), text.end(), text.begin(),
		[](unsigned char character) -> char
		{
			return static_cast<char>(std::tolower(character));
		});

	return text;
}

/*
Compares a candidate item text, such as the visible name or one alias, against
an already normalized player target. The target is expected to be normalized
once by MatchesTarget(), so each candidate can reuse the same value.

Examples:
- text "Caja fuerte" matches normalizedTarget "caja fuerte".
- text "caja" does not match normalizedTarget "caja fuerte"; aliases must be
  declared explicitly when shorter commands should be accepted.
*/
bool Item::MatchesNormalizedText(const std::string& text, const std::string& normalizedTarget)
{
	return NormalizeTarget(text) == normalizedTarget;
}
