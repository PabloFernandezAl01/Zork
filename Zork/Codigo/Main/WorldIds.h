#pragma once

/*
Internal identifiers used by gameplay rules and world setup.

These values are not player-facing text. Names, descriptions and aliases remain
with the content that the player sees.
*/

namespace PlayerIds
{
	constexpr char Player[] = "player";
}

namespace RoomIds
{
	constexpr char TownEntrance[] = "entrada_pueblo";
	constexpr char AbandonedStable[] = "establo_abandonado";
	constexpr char MainStreet[] = "calle_principal";
	constexpr char Saloon[] = "saloon";
	constexpr char SheriffOffice[] = "oficina_sheriff";
	constexpr char BackCell[] = "celda_trasera";
	constexpr char OldChurch[] = "iglesia_vieja";
	constexpr char Crypt[] = "cripta";
}

namespace ItemIds
{
	constexpr char WhiskyBottle[] = "botella_whisky";
	constexpr char TornMap[] = "mapa_rasgado";
	constexpr char Lantern[] = "farol";
	constexpr char Matches[] = "cerillas";
	constexpr char BoltCutter[] = "cizalla";
	constexpr char PotatoSack[] = "saco_patatas";
	constexpr char SmallKey[] = "llave";
	constexpr char BartenderNote[] = "nota_tabernero";
	constexpr char SafeBox[] = "caja_fuerte";
	constexpr char Revolver[] = "revolver";
	constexpr char Ammunition[] = "municion";
	constexpr char SheriffDiary[] = "diario_sheriff";
	constexpr char SilverCross[] = "cruz_plata";
	constexpr char EliasHat[] = "sombrero_elias";
}

namespace WorldTargets
{
	constexpr char Cell[] = "celda";
	constexpr char CellDoor[] = "puerta de la celda";
	constexpr char Lock[] = "cerradura";
	constexpr char CryptLock[] = "cerradura de la cripta";
	constexpr char Chain[] = "cadena";
	constexpr char Chains[] = "cadenas";
	constexpr char Sheriff[] = "sheriff";
}
