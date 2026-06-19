#pragma once

#include "Entity.h"

#include <memory>
#include <iosfwd>
#include <string>
#include <vector>

enum class LightState
{
	NotApplicable,
	Off,
	On
};

enum class LoadState
{
	NotApplicable,
	Unloaded,
	Loaded
};

enum class ContainerState
{
	NotApplicable,
	Open,
	Closed,
	Locked
};

class Item : public Entity
{
public:

	Item(const std::string& id, const std::string& name, const std::string& description);

	/*
	*  Gameplay attributes GET/SET
	*/
	bool IsContainer() const;
	bool IsOpen() const;
	bool IsLocked() const;
	ContainerState GetContainerState() const;
	void SetContainerState(ContainerState state);

	bool IsLightSource() const;
	bool IsTurnedOn() const;
	LightState GetLightState() const;
	void SetLightState(LightState state);

	bool IsWeapon() const;
	bool IsLoaded() const;
	LoadState GetLoadState() const;
	void SetLoadState(LoadState state);

	bool RequiresLightToExamine() const;
	void SetRequiresLightToExamine(bool requiresLight);

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

	bool AddItem(const std::shared_ptr<Item>& item);
	std::shared_ptr<Item> RemoveItem(const std::string& itemId);
	Item* FindItem(const std::string& target);
	const Item* FindItem(const std::string& target) const;

private:

	/*
	*  Helpers to match the user input to actual items
	*/
	static std::string NormalizeTarget(std::string text);
	static bool MatchesNormalizedText(const std::string& text, const std::string& normalizedTarget);

	ContainerState m_containerState;
	LightState m_lightState;
	LoadState m_loadState;
	bool m_requiresLightToExamine;

	std::vector<std::string> m_aliases;
	std::vector<std::shared_ptr<Item>> m_containedItems;
};
