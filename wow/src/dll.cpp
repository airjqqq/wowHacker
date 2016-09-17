
#include "pch.h"
#include <tlhelp32.h>
#include <thread>
#include "targetver.h"
#include <map>
#include "memory.h"

#define LUA_API static


using namespace std;

typedef unsigned char byte;
typedef struct lua_State lua_State;
typedef int scriptFunction_t(lua_State *);
typedef bool objCallback_t(void*, void*);
typedef struct functionPair_t {
	const char* name;
	scriptFunction_t* function;
	struct functionPair_t *next;
} functionPair_t;

//
//long offsetFrameScript_RegisterFunction = 0x00b9000; //48 89 5C 24 08 57 48 83 EC 20 48 8B 3D ?? ?? ?? ?? 48 8B D9 45 33 C0 48 8B CF E8
//long offsetClntObjMgrEnumVisibleObjects = 0x042d100; //48 89 74 24 10 57 48 83 EC 20 48 8B 05 ?? ?? ?? ?? 48 8B F1 48 8B FA 48 8B 88 A0 01 00 00 F6 C1
//long offsetGetObjectPtrByGUID = 0x042db10; //40 53 48 83 EC 30 48 83 3D ?? ?? ?? ?? ?? 8B DA
//long offsetGetGUIDByUnitId = 0x0650630; //48 89 5C 24 18 48 89 74 24 20 48 89 4C 24 08 55 57 41 57 48 8B EC 48 83
//long offsetGetNameByObjectPtr = 0x04810e0; //48 89 5C 24 08 48 89 7C 24 10 55 48 8B EC 48 81 EC 80 00 00 00 48 8B FA 48 8B D9 45 85 C0 74 20
//long offsetGetGUIDStringByGUID = 0x0a79670; //48 89 5C 24 18 48 89 74 24 20 41 56 48 83 EC 20 41 8B F0 48 8B DA 4C 8B F1 45 85 C0 75 13 33 C0
//long offsetGetGUIDByGUIDString = 0x0a78c30; //48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 54 41 55 41 56 41 57 48 83 EC 20 33 C0 48 8B
//long offsetInteractByGUID = 0x06c2630; //40 57 48 83 EC 30 48 8B 05 ?? ?? ?? ?? 48 8B F9 48 85 C0 75 3D 48 8D 4C
//long offsetGetWorldBase = 0x0715060; //48 8B 05 ?? ?? ?? ?? 48 85 C0 74 08 48 8B 80 38
//long offsetPreHeal = 0x047ec80; //40 56 48 83 EC 30 48 8B 01 48 8B F1 FF 90 60 03
//long offsetAbsorb = 0x047ffa0; //40 53 48 83 EC 20 48 8B 01 48 8B D9 FF 90 60 03 00 00 84 C0 74 08 33 C0 48 83 C4 20 5B C3 4C 8D 4C 24 30 4C 8D 05 ?? ?? ?? ?? BA 45 00 00 00 48
//long offsetHealAbsorb = 0x047fff0; //40 53 48 83 EC 20 48 8B 01 48 8B D9 FF 90 60 03 00 00 84 C0 74 08 33 C0 48 83 C4 20 5B C3 4C 8D 4C 24 30 4C 8D 05 ?? ?? ?? ?? BA 2D 01 00 00 48
//long offsetUnitCanAttack = 0x046cf40; //48 89 5C 24 20 57 48 83 EC 20 48 8B 41 10 45 0F
//long offsetUnitCanAssist = 0x046ccf0; //48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 41 0F B6 F0 48 8B DA 48 8B F9 45 84 C9 75 1C 48 8B
//long offsetHandleTerrainClick = 0x06bc4f0; //40 53 48 83 EC 20 83 79 1C 04 48 8B D9 74 09 83
//long offsetGetPlayerFacing = 0x0644560; //40 53 48 83 EC 30 48 8B D9 E8 ?? ?? ?? ?? 48 85 C0 75 16 0F 57 C9 48 8B
//long offsetLua_pushstring = 0x019a220; //48 85 D2 75 1C 48 8B 51 18 48 8B 05 ?? ?? ?? ?? 48 89 42 10 C7 42 08 00
//long offsetLua_pushnumber = 0x019a200; //48 8B 51 18 48 8B 05 ?? ?? ?? ?? 48 89 42 10 C7 42 08 03 00 00 00 F2 0F
//long offsetLua_pushboolean = 0x0199f40; //48 8B 05 ?? ?? ?? ?? 4C 8B 41 18 49 89 40 10 33
//long offsetLua_pushnil = 0x019a1e0; //48 8B 51 18 48 8B 05 ?? ?? ?? ?? 48 89 42 10 C7 42 08 00 00 00 00 48 83 41 18 18 C3 CC
//long offsetLua_checkstack = 0x0199c40; //48 83 EC 28 E8 57 F2 FF FF 48 8D 0D ?? ?? ?? ??
//long offsetLua_tolstring = 0x019acd0; //48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 49 8B D8 8B F2 48 8B F9 E8 B4 E1 FF FF 4C 8B D0 83
//long offsetLua_tointeger = 0x019ac50; //40 57 48 83 EC 40 48 8B F9 E8 42 E2 FF FF 83 78
//long offsetLua_tonumber = 0x019ad70; //48 83 EC 48 E8 27 E1 FF FF 83 78 08 03 74 1A 48
//long offsetLua_getTop = 0x0199900; //48 8B 51 18 48 B8 AB AA AA AA AA AA AA 2A 48 2B
//long offsetLua_setTop = 0x019aba0; //85 D2 78 53 48 63 C2 48 8D 14 40 4C 8D 04 D5 00
//
//

long offsetFrameScript_RegisterFunction = 0x00b9000; //48 89 5C 24 08 57 48 83 EC 20 48 8B 3D ?? ?? ?? ?? 48 8B D9 45 33 C0 48 8B CF E8
long offsetClntObjMgrEnumVisibleObjects = 0x042d100; //48 89 74 24 10 57 48 83 EC 20 48 8B 05 ?? ?? ?? ?? 48 8B F1 48 8B FA 48 8B 88 A0 01 00 00 F6 C1
long offsetGetObjectPtrByGUID = 0x042db10; //40 53 48 83 EC 30 48 83 3D ?? ?? ?? ?? ?? 8B DA
long offsetGetGUIDByUnitId = 0x0650630; //48 89 5C 24 18 48 89 74 24 20 48 89 4C 24 08 55 57 41 57 48 8B EC 48 83
long offsetGetNameByObjectPtr = 0x04810e0; //48 89 5C 24 08 48 89 7C 24 10 55 48 8B EC 48 81 EC 80 00 00 00 48 8B FA 48 8B D9 45 85 C0 74 20
long offsetGetGUIDStringByGUID = 0x0a79670; //48 89 5C 24 18 48 89 74 24 20 41 56 48 83 EC 20 41 8B F0 48 8B DA 4C 8B F1 45 85 C0 75 13 33 C0
long offsetGetGUIDByGUIDString = 0x0a78c30; //48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 54 41 55 41 56 41 57 48 83 EC 20 33 C0 48 8B
long offsetInteractByGUID = 0x06c2630; //40 57 48 83 EC 30 48 8B 05 ?? ?? ?? ?? 48 8B F9 48 85 C0 75 3D 48 8D 4C
long offsetGetWorldBase = 0x0715060; //48 8B 05 ?? ?? ?? ?? 48 85 C0 74 08 48 8B 80 38
long offsetPreHeal = 0x047ec80; //40 56 48 83 EC 30 48 8B 01 48 8B F1 FF 90 60 03
long offsetAbsorb = 0x047ffa0; //40 53 48 83 EC 20 48 8B 01 48 8B D9 FF 90 60 03 00 00 84 C0 74 08 33 C0 48 83 C4 20 5B C3 4C 8D 4C 24 30 4C 8D 05 ?? ?? ?? ?? BA 45 00 00 00 48
long offsetHealAbsorb = 0x047fff0; //40 53 48 83 EC 20 48 8B 01 48 8B D9 FF 90 60 03 00 00 84 C0 74 08 33 C0 48 83 C4 20 5B C3 4C 8D 4C 24 30 4C 8D 05 ?? ?? ?? ?? BA 2D 01 00 00 48
long offsetUnitCanAttack = 0x046cf40; //48 89 5C 24 20 57 48 83 EC 20 48 8B 41 10 45 0F
long offsetUnitCanAssist = 0x046ccf0; //48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 41 0F B6 F0 48 8B DA 48 8B F9 45 84 C9 75 1C 48 8B
long offsetHandleTerrainClick = 0x06bc4f0; //40 53 48 83 EC 20 83 79 1C 04 48 8B D9 74 09 83
long offsetGetPlayerFacing = 0x0644560; //40 53 48 83 EC 30 48 8B D9 E8 ?? ?? ?? ?? 48 85 C0 75 16 0F 57 C9 48 8B
long offsetLua_pushstring = 0x019a220; //48 85 D2 75 1C 48 8B 51 18 48 8B 05 ?? ?? ?? ?? 48 89 42 10 C7 42 08 00
long offsetLua_pushnumber = 0x019a200; //48 8B 51 18 48 8B 05 ?? ?? ?? ?? 48 89 42 10 C7 42 08 03 00 00 00 F2 0F
long offsetLua_pushboolean = 0x0199f40; //48 8B 05 ?? ?? ?? ?? 4C 8B 41 18 49 89 40 10 33
long offsetLua_pushnil = 0x019a1e0; //48 8B 51 18 48 8B 05 ?? ?? ?? ?? 48 89 42 10 C7 42 08 00 00 00 00 48 83 41 18 18 C3 CC
long offsetLua_checkstack = 0x0199c40; //48 83 EC 28 E8 57 F2 FF FF 48 8D 0D ?? ?? ?? ??
long offsetLua_tolstring = 0x019acd0; //48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 49 8B D8 8B F2 48 8B F9 E8 B4 E1 FF FF 4C 8B D0 83
long offsetLua_tointeger = 0x019ac50; //40 57 48 83 EC 40 48 8B F9 E8 42 E2 FF FF 83 78
long offsetLua_tonumber = 0x019ad70; //48 83 EC 48 E8 27 E1 FF FF 83 78 08 03 74 1A 48
long offsetLua_getTop = 0x0199900; //48 8B 51 18 48 B8 AB AA AA AA AA AA AA 2A 48 2B
long offsetLua_setTop = 0x019aba0; //85 D2 78 53 48 63 C2 48 8D 14 40 4C 8D 04 D5 00



void(*FrameScript_RegisterFunction)(const char *functionName, scriptFunction_t* fcn);
bool(*ClntObjMgrEnumVisibleObjects)(objCallback_t* callback, void* param);
long long(*GetObjectPtrByGUID)(long long * guid, int mask, char* fileName, int line);
bool(*GetGUIDByUnitId)(const char* unit, long long* guid_out, long unk1/* = 0*/, long unk2/* = 0*/);
char* (*GetNameByObjectPtr)(long long ObjectPtr, char ** realmName, long unk1/* = 1*/);
void(*GetGUIDStringByGUID)(long long* guid, char* str_out, long unk1/* = 0x80*/);
long(*GetGUIDByGUIDString)(long long* guid_out, const char* guidStr, long unk1/* = 0x80*/);
bool(*InteractByGUID)(long long* guid);
bool(*HandleTerrainClick)(float* positions);
bool(*UnitCanAttack)(long long s, long long d, long unk);
bool(*UnitCanAssist)(long long s, long long d, long unk1, long unk2);
long long(*GetWorldBase)();

LUA_API void(*lua_pushstring) (lua_State *L, const char *s);
LUA_API void(*lua_pushnumber) (lua_State *L, double n);
LUA_API void(*lua_pushboolean) (lua_State *L, bool b);

LUA_API int(*lua_checkstack) (lua_State *L, int sz);
LUA_API int(*lua_tointeger) (lua_State *L, int idx);
LUA_API const char *(*lua_tolstring) (lua_State *L, int idx, int *len);
LUA_API double(*lua_tonumber) (lua_State *L, int idx);


LUA_API int(*lua_gettop) (lua_State *L);
LUA_API void(*lua_settop) (lua_State *L, int idx);


LPCVOID wowBaseAddress;
long long base;
SIZE_T nBaseSize;

void init();
void registerFunctions();

void* ObjectBuffer[10240];

bool objectCallback(void* obj, void* param) {
	int* tmp = (int*)param;
	ObjectBuffer[*tmp] = obj;
	*tmp = *tmp + 1;
	return true;
}
// 
// std::map<long, bool> guids;
namespace CGObjectData {
	enum {
		Guid = 0x0, // Size: 0x4, Flags: 0x1
		Data = 0x10, // Size: 0x4, Flags: 0x1
		Type = 0x20, // Size: 0x1, Flags: 0x1
		EntryID = 0x24, // Size: 0x1, Flags: 0x80
		DynamicFlags = 0x28, // Size: 0x1, Flags: 0x280
		Scale = 0x2C, // Size: 0x1, Flags: 0x1
	};
}

namespace CGUnitData {
	enum {
		Charm = 0x30, // Size: 0x4, Flags: 0x1
		Summon = 0x40, // Size: 0x4, Flags: 0x1
		Critter = 0x50, // Size: 0x4, Flags: 0x2
		CharmedBy = 0x60, // Size: 0x4, Flags: 0x1
		SummonedBy = 0x70, // Size: 0x4, Flags: 0x1
		CreatedBy = 0x80, // Size: 0x4, Flags: 0x1
		DemonCreator = 0x90, // Size: 0x4, Flags: 0x1
		Target = 0xA0, // Size: 0x4, Flags: 0x1
		BattlePetCompanionGUID = 0xB0, // Size: 0x4, Flags: 0x1
		BattlePetDBID = 0xC0, // Size: 0x2, Flags: 0x1
		ChannelObject = 0xC8, // Size: 0x4, Flags: 0x201
		ChannelSpell = 0xD8, // Size: 0x1, Flags: 0x201
		ChannelSpellXSpellVisual = 0xDC, // Size: 0x1, Flags: 0x201
		SummonedByHomeRealm = 0xE0, // Size: 0x1, Flags: 0x1
		Sex = 0xE4, // Size: 0x1, Flags: 0x1
		DisplayPower = 0xE8, // Size: 0x1, Flags: 0x1
		OverrideDisplayPowerID = 0xEC, // Size: 0x1, Flags: 0x1
		Health = 0xF0, // Size: 0x2, Flags: 0x1
		Power = 0xF8, // Size: 0x6, Flags: 0x401
		MaxHealth = 0x110, // Size: 0x2, Flags: 0x1
		MaxPower = 0x118, // Size: 0x6, Flags: 0x1
		PowerRegenFlatModifier = 0x130, // Size: 0x6, Flags: 0x46
		PowerRegenInterruptedFlatModifier = 0x148, // Size: 0x6, Flags: 0x46
		Level = 0x160, // Size: 0x1, Flags: 0x1
		EffectiveLevel = 0x164, // Size: 0x1, Flags: 0x1
		ScalingLevelMin = 0x168, // Size: 0x1, Flags: 0x1
		ScalingLevelMax = 0x16C, // Size: 0x1, Flags: 0x1
		ScalingLevelDelta = 0x170, // Size: 0x1, Flags: 0x1
		FactionTemplate = 0x174, // Size: 0x1, Flags: 0x1
		VirtualItems = 0x178, // Size: 0x6, Flags: 0x1
		Flags = 0x190, // Size: 0x1, Flags: 0x201
		Flags2 = 0x194, // Size: 0x1, Flags: 0x201
		Flags3 = 0x198, // Size: 0x1, Flags: 0x201
		AuraState = 0x19C, // Size: 0x1, Flags: 0x1
		AttackRoundBaseTime = 0x1A0, // Size: 0x2, Flags: 0x1
		RangedAttackRoundBaseTime = 0x1A8, // Size: 0x1, Flags: 0x2
		BoundingRadius = 0x1AC, // Size: 0x1, Flags: 0x1
		CombatReach = 0x1B0, // Size: 0x1, Flags: 0x1
		DisplayID = 0x1B4, // Size: 0x1, Flags: 0x280
		NativeDisplayID = 0x1B8, // Size: 0x1, Flags: 0x201
		MountDisplayID = 0x1BC, // Size: 0x1, Flags: 0x201
		MinDamage = 0x1C0, // Size: 0x1, Flags: 0x16
		MaxDamage = 0x1C4, // Size: 0x1, Flags: 0x16
		MinOffHandDamage = 0x1C8, // Size: 0x1, Flags: 0x16
		MaxOffHandDamage = 0x1CC, // Size: 0x1, Flags: 0x16
		AnimTier = 0x1D0, // Size: 0x1, Flags: 0x1
		PetNumber = 0x1D4, // Size: 0x1, Flags: 0x1
		PetNameTimestamp = 0x1D8, // Size: 0x1, Flags: 0x1
		PetExperience = 0x1DC, // Size: 0x1, Flags: 0x4
		PetNextLevelExperience = 0x1E0, // Size: 0x1, Flags: 0x4
		ModCastingSpeed = 0x1E4, // Size: 0x1, Flags: 0x1
		ModSpellHaste = 0x1E8, // Size: 0x1, Flags: 0x1
		ModHaste = 0x1EC, // Size: 0x1, Flags: 0x1
		ModRangedHaste = 0x1F0, // Size: 0x1, Flags: 0x1
		ModHasteRegen = 0x1F4, // Size: 0x1, Flags: 0x1
		ModTimeRate = 0x1F8, // Size: 0x1, Flags: 0x1
		CreatedBySpell = 0x1FC, // Size: 0x1, Flags: 0x1
		NpcFlags = 0x200, // Size: 0x2, Flags: 0x81
		EmoteState = 0x208, // Size: 0x1, Flags: 0x1
		Stats = 0x20C, // Size: 0x4, Flags: 0x6
		StatPosBuff = 0x21C, // Size: 0x4, Flags: 0x6
		StatNegBuff = 0x22C, // Size: 0x4, Flags: 0x6
		Resistances = 0x23C, // Size: 0x7, Flags: 0x16
		ResistanceBuffModsPositive = 0x258, // Size: 0x7, Flags: 0x6
		ResistanceBuffModsNegative = 0x274, // Size: 0x7, Flags: 0x6
		ModBonusArmor = 0x290, // Size: 0x1, Flags: 0x6
		BaseMana = 0x294, // Size: 0x1, Flags: 0x1
		BaseHealth = 0x298, // Size: 0x1, Flags: 0x6
		ShapeshiftForm = 0x29C, // Size: 0x1, Flags: 0x1
		AttackPower = 0x2A0, // Size: 0x1, Flags: 0x6
		AttackPowerModPos = 0x2A4, // Size: 0x1, Flags: 0x6
		AttackPowerModNeg = 0x2A8, // Size: 0x1, Flags: 0x6
		AttackPowerMultiplier = 0x2AC, // Size: 0x1, Flags: 0x6
		RangedAttackPower = 0x2B0, // Size: 0x1, Flags: 0x6
		RangedAttackPowerModPos = 0x2B4, // Size: 0x1, Flags: 0x6
		RangedAttackPowerModNeg = 0x2B8, // Size: 0x1, Flags: 0x6
		RangedAttackPowerMultiplier = 0x2BC, // Size: 0x1, Flags: 0x6
		SetAttackSpeedAura = 0x2C0, // Size: 0x1, Flags: 0x6
		MinRangedDamage = 0x2C4, // Size: 0x1, Flags: 0x6
		MaxRangedDamage = 0x2C8, // Size: 0x1, Flags: 0x6
		PowerCostModifier = 0x2CC, // Size: 0x7, Flags: 0x6
		PowerCostMultiplier = 0x2E8, // Size: 0x7, Flags: 0x6
		MaxHealthModifier = 0x304, // Size: 0x1, Flags: 0x6
		HoverHeight = 0x308, // Size: 0x1, Flags: 0x1
		MinItemLevelCutoff = 0x30C, // Size: 0x1, Flags: 0x1
		MinItemLevel = 0x310, // Size: 0x1, Flags: 0x1
		MaxItemLevel = 0x314, // Size: 0x1, Flags: 0x1
		WildBattlePetLevel = 0x318, // Size: 0x1, Flags: 0x1
		BattlePetCompanionNameTimestamp = 0x31C, // Size: 0x1, Flags: 0x1
		InteractSpellID = 0x320, // Size: 0x1, Flags: 0x1
		StateSpellVisualID = 0x324, // Size: 0x1, Flags: 0x280
		StateAnimID = 0x328, // Size: 0x1, Flags: 0x280
		StateAnimKitID = 0x32C, // Size: 0x1, Flags: 0x280
		StateWorldEffectID = 0x330, // Size: 0x4, Flags: 0x280
		ScaleDuration = 0x340, // Size: 0x1, Flags: 0x1
		LooksLikeMountID = 0x344, // Size: 0x1, Flags: 0x1
		LooksLikeCreatureID = 0x348, // Size: 0x1, Flags: 0x1
		LookAtControllerID = 0x34C, // Size: 0x1, Flags: 0x1
		LookAtControllerTarget = 0x350, // Size: 0x4, Flags: 0x1
	};
}

namespace CGPlayerData {
	enum {
		DuelArbiter = 0x360, // Size: 0x4, Flags: 0x1
		WowAccount = 0x370, // Size: 0x4, Flags: 0x1
		LootTargetGUID = 0x380, // Size: 0x4, Flags: 0x1
		PlayerFlags = 0x390, // Size: 0x1, Flags: 0x1
		PlayerFlagsEx = 0x394, // Size: 0x1, Flags: 0x1
		GuildRankID = 0x398, // Size: 0x1, Flags: 0x1
		GuildDeleteDate = 0x39C, // Size: 0x1, Flags: 0x1
		GuildLevel = 0x3A0, // Size: 0x1, Flags: 0x1
		HairColorID = 0x3A4, // Size: 0x1, Flags: 0x1
		CustomDisplayOption = 0x3A8, // Size: 0x1, Flags: 0x1
		Inebriation = 0x3AC, // Size: 0x1, Flags: 0x1
		ArenaFaction = 0x3B0, // Size: 0x1, Flags: 0x1
		DuelTeam = 0x3B4, // Size: 0x1, Flags: 0x1
		GuildTimeStamp = 0x3B8, // Size: 0x1, Flags: 0x1
		QuestLog = 0x3BC, // Size: 0x320, Flags: 0x20
		VisibleItems = 0x103C, // Size: 0x26, Flags: 0x1
		PlayerTitle = 0x10D4, // Size: 0x1, Flags: 0x1
		FakeInebriation = 0x10D8, // Size: 0x1, Flags: 0x1
		VirtualPlayerRealm = 0x10DC, // Size: 0x1, Flags: 0x1
		CurrentSpecID = 0x10E0, // Size: 0x1, Flags: 0x1
		TaxiMountAnimKitID = 0x10E4, // Size: 0x1, Flags: 0x1
		AvgItemLevel = 0x10E8, // Size: 0x4, Flags: 0x1
		CurrentBattlePetBreedQuality = 0x10F8, // Size: 0x1, Flags: 0x1
		Prestige = 0x10FC, // Size: 0x1, Flags: 0x1
		HonorLevel = 0x1100, // Size: 0x1, Flags: 0x1
		InvSlots = 0x1104, // Size: 0x2EC, Flags: 0x2
		FarsightObject = 0x1CB4, // Size: 0x4, Flags: 0x2
		SummonedBattlePetGUID = 0x1CC4, // Size: 0x4, Flags: 0x2
		KnownTitles = 0x1CD4, // Size: 0xC, Flags: 0x2
		Coinage = 0x1D04, // Size: 0x2, Flags: 0x2
		XP = 0x1D0C, // Size: 0x1, Flags: 0x2
		NextLevelXP = 0x1D10, // Size: 0x1, Flags: 0x2
		Skill = 0x1D14, // Size: 0x1C0, Flags: 0x2
		CharacterPoints = 0x2414, // Size: 0x1, Flags: 0x2
		MaxTalentTiers = 0x2418, // Size: 0x1, Flags: 0x2
		TrackCreatureMask = 0x241C, // Size: 0x1, Flags: 0x2
		TrackResourceMask = 0x2420, // Size: 0x1, Flags: 0x2
		MainhandExpertise = 0x2424, // Size: 0x1, Flags: 0x2
		OffhandExpertise = 0x2428, // Size: 0x1, Flags: 0x2
		RangedExpertise = 0x242C, // Size: 0x1, Flags: 0x2
		CombatRatingExpertise = 0x2430, // Size: 0x1, Flags: 0x2
		BlockPercentage = 0x2434, // Size: 0x1, Flags: 0x2
		DodgePercentage = 0x2438, // Size: 0x1, Flags: 0x2
		ParryPercentage = 0x243C, // Size: 0x1, Flags: 0x2
		CritPercentage = 0x2440, // Size: 0x1, Flags: 0x2
		RangedCritPercentage = 0x2444, // Size: 0x1, Flags: 0x2
		OffhandCritPercentage = 0x2448, // Size: 0x1, Flags: 0x2
		SpellCritPercentage = 0x244C, // Size: 0x1, Flags: 0x2
		ShieldBlock = 0x2450, // Size: 0x1, Flags: 0x2
		ShieldBlockCritPercentage = 0x2454, // Size: 0x1, Flags: 0x2
		Mastery = 0x2458, // Size: 0x1, Flags: 0x2
		Speed = 0x245C, // Size: 0x1, Flags: 0x2
		Lifesteal = 0x2460, // Size: 0x1, Flags: 0x2
		Avoidance = 0x2464, // Size: 0x1, Flags: 0x2
		Sturdiness = 0x2468, // Size: 0x1, Flags: 0x2
		Versatility = 0x246C, // Size: 0x1, Flags: 0x2
		VersatilityBonus = 0x2470, // Size: 0x1, Flags: 0x2
		PvpPowerDamage = 0x2474, // Size: 0x1, Flags: 0x2
		PvpPowerHealing = 0x2478, // Size: 0x1, Flags: 0x2
		ExploredZones = 0x247C, // Size: 0x100, Flags: 0x2
		RestInfo = 0x287C, // Size: 0x4, Flags: 0x2
		ModDamageDonePos = 0x288C, // Size: 0x7, Flags: 0x2
		ModDamageDoneNeg = 0x28A8, // Size: 0x7, Flags: 0x2
		ModDamageDonePercent = 0x28C4, // Size: 0x7, Flags: 0x2
		ModHealingDonePos = 0x28E0, // Size: 0x1, Flags: 0x2
		ModHealingPercent = 0x28E4, // Size: 0x1, Flags: 0x2
		ModHealingDonePercent = 0x28E8, // Size: 0x1, Flags: 0x2
		ModPeriodicHealingDonePercent = 0x28EC, // Size: 0x1, Flags: 0x2
		WeaponDmgMultipliers = 0x28F0, // Size: 0x3, Flags: 0x2
		WeaponAtkSpeedMultipliers = 0x28FC, // Size: 0x3, Flags: 0x2
		ModSpellPowerPercent = 0x2908, // Size: 0x1, Flags: 0x2
		ModResiliencePercent = 0x290C, // Size: 0x1, Flags: 0x2
		OverrideSpellPowerByAPPercent = 0x2910, // Size: 0x1, Flags: 0x2
		OverrideAPBySpellPowerPercent = 0x2914, // Size: 0x1, Flags: 0x2
		ModTargetResistance = 0x2918, // Size: 0x1, Flags: 0x2
		ModTargetPhysicalResistance = 0x291C, // Size: 0x1, Flags: 0x2
		LocalFlags = 0x2920, // Size: 0x1, Flags: 0x2
		NumRespecs = 0x2924, // Size: 0x1, Flags: 0x2
		SelfResSpell = 0x2928, // Size: 0x1, Flags: 0x2
		PvpMedals = 0x292C, // Size: 0x1, Flags: 0x2
		BuybackPrice = 0x2930, // Size: 0xC, Flags: 0x2
		BuybackTimestamp = 0x2960, // Size: 0xC, Flags: 0x2
		YesterdayHonorableKills = 0x2990, // Size: 0x1, Flags: 0x2
		LifetimeHonorableKills = 0x2994, // Size: 0x1, Flags: 0x2
		WatchedFactionIndex = 0x2998, // Size: 0x1, Flags: 0x2
		CombatRatings = 0x299C, // Size: 0x20, Flags: 0x2
		PvpInfo = 0x2A1C, // Size: 0x24, Flags: 0x2
		MaxLevel = 0x2AAC, // Size: 0x1, Flags: 0x2
		ScalingPlayerLevelDelta = 0x2AB0, // Size: 0x1, Flags: 0x2
		MaxCreatureScalingLevel = 0x2AB4, // Size: 0x1, Flags: 0x2
		NoReagentCostMask = 0x2AB8, // Size: 0x4, Flags: 0x2
		PetSpellPower = 0x2AC8, // Size: 0x1, Flags: 0x2
		Researching = 0x2ACC, // Size: 0xA, Flags: 0x2
		ProfessionSkillLine = 0x2AF4, // Size: 0x2, Flags: 0x2
		UiHitModifier = 0x2AFC, // Size: 0x1, Flags: 0x2
		UiSpellHitModifier = 0x2B00, // Size: 0x1, Flags: 0x2
		HomeRealmTimeOffset = 0x2B04, // Size: 0x1, Flags: 0x2
		ModPetHaste = 0x2B08, // Size: 0x1, Flags: 0x2
		OverrideSpellsID = 0x2B0C, // Size: 0x1, Flags: 0x402
		LfgBonusFactionID = 0x2B10, // Size: 0x1, Flags: 0x2
		LootSpecID = 0x2B14, // Size: 0x1, Flags: 0x2
		OverrideZonePVPType = 0x2B18, // Size: 0x1, Flags: 0x402
		BagSlotFlags = 0x2B1C, // Size: 0x4, Flags: 0x2
		BankBagSlotFlags = 0x2B2C, // Size: 0x7, Flags: 0x2
		InsertItemsLeftToRight = 0x2B48, // Size: 0x1, Flags: 0x2
		QuestCompleted = 0x2B4C, // Size: 0x36B, Flags: 0x2
		Honor = 0x38F8, // Size: 0x1, Flags: 0x2
		HonorNextLevel = 0x38FC, // Size: 0x1, Flags: 0x2
	};
}

namespace CGGameObjectData {
	enum {
		CreatedBy = 0x30, // Size: 0x4, Flags: 0x1
		DisplayID = 0x40, // Size: 0x1, Flags: 0x280
		Flags = 0x44, // Size: 0x1, Flags: 0x201
		ParentRotation = 0x48, // Size: 0x4, Flags: 0x1
		FactionTemplate = 0x58, // Size: 0x1, Flags: 0x1
		Level = 0x5C, // Size: 0x1, Flags: 0x1
		PercentHealth = 0x60, // Size: 0x1, Flags: 0x201
		SpellVisualID = 0x64, // Size: 0x1, Flags: 0x281
		StateSpellVisualID = 0x68, // Size: 0x1, Flags: 0x280
		SpawnTrackingStateAnimID = 0x6C, // Size: 0x1, Flags: 0x280
		SpawnTrackingStateAnimKitID = 0x70, // Size: 0x1, Flags: 0x280
		StateWorldEffectID = 0x74, // Size: 0x4, Flags: 0x280
	};
}

namespace CGDynamicObjectData {
	enum {
		Caster = 0x30, // Size: 0x4, Flags: 0x1
		TypeAndVisualID = 0x40, // Size: 0x1, Flags: 0x80
		SpellID = 0x44, // Size: 0x1, Flags: 0x1
		Radius = 0x48, // Size: 0x1, Flags: 0x1
		CastTime = 0x4C, // Size: 0x1, Flags: 0x1
	};
}

namespace CGCorpseData {
	enum {
		Owner = 0x30, // Size: 0x4, Flags: 0x1
		PartyGUID = 0x40, // Size: 0x4, Flags: 0x1
		DisplayID = 0x50, // Size: 0x1, Flags: 0x1
		Items = 0x54, // Size: 0x13, Flags: 0x1
		SkinID = 0xA0, // Size: 0x1, Flags: 0x1
		FacialHairStyleID = 0xA4, // Size: 0x1, Flags: 0x1
		Flags = 0xA8, // Size: 0x1, Flags: 0x1
		DynamicFlags = 0xAC, // Size: 0x1, Flags: 0x80
		FactionTemplate = 0xB0, // Size: 0x1, Flags: 0x1
		CustomDisplayOption = 0xB4, // Size: 0x1, Flags: 0x1
	};
}

namespace CGAreaTriggerData {
	enum {
		OverrideScaleCurve = 0x30, // Size: 0x7, Flags: 0x201
		ExtraScaleCurve = 0x4C, // Size: 0x7, Flags: 0x201
		Caster = 0x68, // Size: 0x4, Flags: 0x1
		Duration = 0x78, // Size: 0x1, Flags: 0x1
		TimeToTarget = 0x7C, // Size: 0x1, Flags: 0x201
		TimeToTargetScale = 0x80, // Size: 0x1, Flags: 0x201
		TimeToTargetExtraScale = 0x84, // Size: 0x1, Flags: 0x201
		SpellID = 0x88, // Size: 0x1, Flags: 0x1
		SpellVisualID = 0x8C, // Size: 0x1, Flags: 0x80
		BoundsRadius2D = 0x90, // Size: 0x1, Flags: 0x280
		DecalPropertiesID = 0x94, // Size: 0x1, Flags: 0x1
	};
}

namespace CGSceneObjectData {
	enum {
		ScriptPackageID = 0x30, // Size: 0x1, Flags: 0x1
		RndSeedVal = 0x34, // Size: 0x1, Flags: 0x1
		CreatedBy = 0x38, // Size: 0x4, Flags: 0x1
		SceneType = 0x48, // Size: 0x1, Flags: 0x1
	};
}

namespace CGConversationData {
	enum {
		LastLineDuration = 0x30, // Size: 0x1, Flags: 0x80
	};
}


static int listObjects(lua_State* L) {

	int cnt = 0;
	int arg1 = 0;

	arg1 = lua_tointeger(L, 1);
	int arg2 = lua_tointeger(L, 2);


	//connect = 0x156FD70
	//first = 0x1A8
	//next = 0x70


	ClntObjMgrEnumVisibleObjects(objectCallback, &cnt);

	// fcnArray = obj -> 0

	// data = obj -> 0x10
	// guid = data -> 0

	// guid = obj -> 0x58  (long long)
	// target


	// health = obj -> 0x200 -> 0xc0 (long)
	// power = obj -> 0x200 -> 0xc8 (long)
	// maxhealth = obj -> 0x200 -> 0xe0 (long)

	// x = obj -> 0x210 -> 0x20 (float)  // unit
	// y = obj -> 0x210 -> 0x24 (float)
	// z = obj -> 0x210 -> 0x28 (float)

	// xyz = obj -> 0x228  // areaTrigger

	// facing = obj -> 0x210 -> 0x30 (float)   //obj -> 15B8h  //1D18
	// theta = obj -> 0x210 -> 0x30 (float)
	enum WoWObjectType : int
	{
		Object = 0,
		Item = 1,
		Container = 2,
		Unit = 3,
		Player = 4,
		GameObject = 5,
		DynamicObject = 6,
		Corpse = 7,
		AreaTrigger = 8,
		SceneObject = 9,
		NumClientObjectTypes = 0xA,
		None = 0x270f,
	};
	//F62258 unitFcns
	//F67C60 areaTriggerFcn

	// fcns
	// 10 getPostion
	// 1a0 getPostion
	// 1a8 getFacing
	float* (*getObjectPosition)(long long objectPtr, float* position);
	char buffer[128];
	int stackSize = 0;
	int matched = 0;
	for (int i = 0; i < cnt; i++)
	{
		long long obj = (long long)ObjectBuffer[i];
		int type = *(int*)(*(long long*)(obj + 0x10) + 0x20);
		long long guid[2] = { *(long long*)(obj + 0x58),*(long long*)(obj + 0x60) };
		if (!(type & 0x02))
		{
			matched++;
			getObjectPosition = (float* (*)(long long objectPtr, float* position))*(long long*)(*(long long*)(obj)+0x10);
			float position[4] = { 0.0 };
			getObjectPosition(obj, position);
			// 			sprintf_s(buffer,"obj:  %016llx  type:  %08x  guid:  %016llx%016llx",obj,type,guidH,guidL);

			char guidStr[256];
			GetGUIDStringByGUID(guid, guidStr, 0x80);
			sprintf_s(buffer, "obj:  %016llx  type:  %08x  guid:  %s", obj, type, guidStr);

			//sprintf_s(buffer,"obj:  %016llx  type:  %08x  guid:  %016llx%016llx",obj,type,guid[1],guid[0]);
			lua_pushstring(L, buffer);
			stackSize++;
		}
	}
	lua_pushnumber(L, matched);
	stackSize++;
	return stackSize;
	//return fcn(L);
}



static int unitGUID(lua_State* L) {
	const char* unit = lua_tolstring(L, 1, NULL);
	if (unit) {
		long long guid[2];
		if (GetGUIDByGUIDString(guid, unit, 0x80))
		{
			char guidStr[256];
			GetGUIDStringByGUID(guid, guidStr, 0x80);
			lua_pushstring(L, guidStr);
			return 1;
		}
	}
	return 0;
}

long objectSize = 0;

static int AirjUpdateObjects(lua_State* L) {
	objectSize = 0;
	ClntObjMgrEnumVisibleObjects(objectCallback, &objectSize);
	lua_pushnumber(L, objectSize);
	return 1;
}


static int AirjGetObjectGUID(lua_State* L) {
	if (lua_checkstack(L, 1)) {

		long index = lua_tointeger(L, 1);
		if (index >= 0 && index < objectSize) {

			long long obj = (long long)ObjectBuffer[index];

			long long guid[2] = { *(long long*)(obj + 0x58),*(long long*)(obj + 0x60) };
			char guidStr[256];
			GetGUIDStringByGUID(guid, guidStr, 0x80);
			lua_pushstring(L, guidStr);
			int type = *(int*)(*(long long*)(obj + 0x10) + 0x20);
			lua_pushnumber(L, type);
			// 			char buffer[1024];
			// 			sprintf_s(buffer,"%016llx",obj);
			// 			lua_pushstring(L,buffer);
			return 2;
		}
	}
	return 0;
}


static int AirjGetObjectGUIDByUnit(lua_State* L) {
	const char* unit = lua_tolstring(L, 1, NULL);
	if (unit) {
		long long guid[2];
		bool got;
		if (strcmp(unit, "mouseover") == 0) {
			guid[0] = *(long long*)(base + 0x1737B40);
			guid[1] = *(long long*)(base + 0x1737B40 + 8);
			got = !(guid[0] == 0 && guid[1] == 0);
		}
		else {
			got = GetGUIDByUnitId(unit, guid, 0, 0);
		}
		if (got)
		{
			char guidStr[256];
			GetGUIDStringByGUID(guid, guidStr, 0x80);
			lua_pushstring(L, guidStr);
			return 1;
		}
	}
	return 0;
}

static int AirjGetObjectType(lua_State* L) {
	const char* guidStr = lua_tolstring(L, 1, NULL);
	if (guidStr) {
		long long guid[2];
		if (GetGUIDByGUIDString(guid, guidStr, 0x80))
		{
			long long obj = GetObjectPtrByGUID(guid, 0x1, "", 0);
			if (obj) {
				int type = *(int*)(*(long long*)(obj + 0x10) + 0x20);
				lua_pushnumber(L, type);
				return 1;
			}
		}
	}
	return 0;
}

static int AirjGetObjectDataInt(lua_State* L) {
	const char* guidStr = lua_tolstring(L, 1, NULL);
	int offset = lua_tointeger(L, 2);
	if (guidStr) {
		long long guid[2];
		if (GetGUIDByGUIDString(guid, guidStr, 0x80))
		{
			long long obj = GetObjectPtrByGUID(guid, 0x01, "", 0);
			if (obj) {
				int value = *(int*)(*(long long*)(obj + 0x10) + offset);
				lua_pushnumber(L, value);
				return 1;
			}
		}
	}
	return 0;
}
static int AirjGetObjectDataFloat(lua_State* L) {
	const char* guidStr = lua_tolstring(L, 1, NULL);
	int offset = lua_tointeger(L, 2);
	if (guidStr) {
		long long guid[2];
		if (GetGUIDByGUIDString(guid, guidStr, 0x80))
		{
			long long obj = GetObjectPtrByGUID(guid, 0x01, "", 0);
			if (obj) {
				float value = *(float*)(*(long long*)(obj + 0x10) + offset);
				lua_pushnumber(L, value);
				return 1;
			}
		}
	}
	return 0;
}

float* getObjectPosition(long long obj, float* position) {
	float* (*getObjectPosition)(long long objectPtr, float* position);
	float(*getObjectFacing)(long long objectPt);
	getObjectPosition = (float* (*)(long long objectPtr, float* position))*(long long*)(*(long long*)(obj)+0x198);
	getObjectFacing = (float(*)(long long objectPt))*(long long*)(*(long long*)(obj)+0x1A8);
	getObjectPosition(obj, position);
	position[3] = getObjectFacing(obj);
	return position;
}

static int AirjGetObjectPosition(lua_State* L) {
	const char* guidStr = lua_tolstring(L, 1, NULL);
	if (guidStr) {
		long long guid[2];
		if (GetGUIDByGUIDString(guid, guidStr, 0x80))
		{
			long long obj = GetObjectPtrByGUID(guid, 0x1, "", 0);
			if (obj) {
				float position[4];
				getObjectPosition(obj, position);
				lua_pushnumber(L, position[0]);
				lua_pushnumber(L, position[1]);
				lua_pushnumber(L, position[2]);
				lua_pushnumber(L, position[3]);

				int type = *(int*)(*(long long*)(obj + 0x10) + 0x20);
				int toRet = 4;
				if (type & 0x8) {
					float value = *(float*)(*(long long*)(obj + 0x10) + 0x1b0);
					lua_pushnumber(L, value);
					toRet++;
				}
				else if (type & 0x100) {
					float value = *(float*)(*(long long*)(obj + 0x10) + 0x90);
					lua_pushnumber(L, value);
					toRet++;
				}
				return toRet;
			}
		}
	}
	return 0;
}

static int AirjGetHealth(lua_State* L) {
	const char* guidStr = lua_tolstring(L, 1, NULL);
	if (guidStr) {
		long long guid[2];
		if (GetGUIDByGUIDString(guid, guidStr, 0x80))
		{
			long long obj = GetObjectPtrByGUID(guid, 0x1, "", 0);
			if (obj) {
				int type = *(int*)(*(long long*)(obj + 0x10) + 0x20);
				if (type & 0x8) {

					int value = *(int*)(*(long long*)(obj + 0x10) + 0xf0);
					lua_pushnumber(L, value);
					value = *(int*)(*(long long*)(obj + 0x10) + 0x110);
					lua_pushnumber(L, value);
					int(*fcn)(long long objectPt);
					fcn = (int(*)(long long objectPt))(base + offsetPreHeal);
					lua_pushnumber(L, fcn(obj));
					fcn = (int(*)(long long objectPt))(base + offsetAbsorb);
					lua_pushnumber(L, fcn(obj));
					fcn = (int(*)(long long objectPt))(base + offsetHealAbsorb);
					lua_pushnumber(L, fcn(obj));

					fcn = (int(*)(long long objectPt))*(long long*)(*(long long*)(obj)+0x360);
					lua_pushboolean(L, (fcn(obj) & 0xff) != 0);

					return 6;
				}

			}
		}
	}
	return 0;
}

static int AirjTargetByGUID(lua_State* L) {

	const char* guidStr = lua_tolstring(L, 1, NULL);
	if (guidStr) {
		long long guid[2];
		if (GetGUIDByGUIDString(guid, guidStr, 0x80))
		{
			void(*target)(long long obj, long long * guid, long unk1/* = 0*/);
			long long obj = *(long long*)(base + 0x141FE30);

			target = (void(*)(long long obj, long long * guid, long unk1/* = 0*/))*(long long *)(*(long long *)obj + 0x20);
			target(obj, guid, 0);

		}
	}
	return 0;
}

static int AirjFocusByGUID(lua_State* L) {

	const char* guidStr = lua_tolstring(L, 1, NULL);
	if (guidStr) {
		long long guid[2];
		if (GetGUIDByGUIDString(guid, guidStr, 0x80))
		{
			void(*focus)(long long obj, long long * guid, long unk1/* = 0*/);
			long long obj = *(long long*)(base + 0x141FE30);

			focus = (void(*)(long long obj, long long * guid, long unk1/* = 0*/))*(long long *)(*(long long *)obj + 0xA0);
			focus(obj, guid, 0);

		}
	}
	return 0;
}

static int AirjInteractByGUID(lua_State* L) {

	const char* guidStr = lua_tolstring(L, 1, NULL);
	if (guidStr) {
		long long guid[2];
		if (GetGUIDByGUIDString(guid, guidStr, 0x80))
		{
			InteractByGUID(guid);
		}
	}
	return 0;
}

static int AirjGetCamera(lua_State* L) {
	long long worldBase = GetWorldBase();
	if (worldBase) {
		float r, f, t, x, y, z, h,facing,s,c;
		r = *(float*)(worldBase + 0x250);
		//f = *(float*)(worldBase + 0x13C);
		t = *(float*)(worldBase + 0x140);
		s = *(float*)(worldBase + 0x28);
		c = *(float*)(worldBase + 0x2C);
		f = acosf(c);
		if (s > 0) {
			f = -f;
		}
		facing = *(float*)(worldBase + 0x240);
		f -= facing;
		////24c r_set
		//int holding = *(int*)(worldBase + 0xC4);
		//if (holding) {
		//	long long guid[2];
		//	GetGUIDByUnitId("player", guid, 0, 0);
		//	long long obj = GetObjectPtrByGUID(guid, 0x1, "", 0);
		//	if (obj) {
		//		float position[4];
		//		getObjectPosition(obj, position);
		//		f -= position[3];
		//	}
		//}
		lua_pushnumber(L, r);
		lua_pushnumber(L, f);
		lua_pushnumber(L, t);

		x = *(float*)(worldBase + 0x10);
		y = *(float*)(worldBase + 0x14);
		z = *(float*)(worldBase + 0x18);

		h = *(float*)(worldBase + 0x280);

		lua_pushnumber(L, x);
		lua_pushnumber(L, y);
		lua_pushnumber(L, z);
		lua_pushnumber(L, h);
		return 7;
	}
	return 0;
}

static int AirjSetCameraDistance(lua_State* L) {
	long long worldBase = GetWorldBase();
	if (worldBase) {
		float range = lua_tonumber(L, 1);
		*(float*)(worldBase + 0x24C) = range;
		return 0;
	}
	return 0;
}

static int AirjHandleTerrainClick(lua_State* L) {
	float x = lua_tonumber(L, 1);
	float y = lua_tonumber(L, 2);
	float z = lua_tonumber(L, 3);
	float s[0x10] = { 0 };
	s[4] = x;
	s[5] = y;
	s[6] = z;
	*(int*)(s+7) = 1;
	HandleTerrainClick(s);
	return 0;
}
static int AirjUnitCanAttack(lua_State* L) {
	const char* guidStr = lua_tolstring(L, 2, NULL);
	const char* pguidStr = lua_tolstring(L, 1, NULL);
	if (guidStr && pguidStr) {
		long long guid[2];
		long long pguid[2];
		if (GetGUIDByGUIDString(guid, guidStr, 0x80) && GetGUIDByGUIDString(pguid, pguidStr, 0x80))
		{
			long long obj = GetObjectPtrByGUID(guid, 0x1, "", 0);
			long long pobj = GetObjectPtrByGUID(pguid, 0x1, "", 0);
			if (obj && pobj) {
				if (UnitCanAttack(pobj, obj, 0))
				{
					lua_pushboolean(L, true);
				}
				else
				{
					lua_pushboolean(L, false);
				}
				return 1;
			}
		}
	}
	return 0;
}
static int AirjUnitCanAssist(lua_State* L) {
	const char* guidStr = lua_tolstring(L, 2, NULL);
	const char* pguidStr = lua_tolstring(L, 1, NULL);
	if (guidStr && pguidStr) {
		long long guid[2];
		long long pguid[2];
		if (GetGUIDByGUIDString(guid, guidStr, 0x80) && GetGUIDByGUIDString(pguid, pguidStr, 0x80))
		{
			long long obj = GetObjectPtrByGUID(guid, 0x1, "", 0);
			long long pobj = GetObjectPtrByGUID(pguid, 0x1, "", 0);
			if (obj && pobj) {
				if (UnitCanAssist(pobj, obj, 0, 0))
				{
					lua_pushboolean(L, true);
				}
				else
				{
					lua_pushboolean(L, false);
				}
				return 1;
			}
		}
	}
	return 0;
}

static int AirjTest(lua_State* L) {
	const char* guidStr = lua_tolstring(L, 1, NULL);
	if (guidStr) {
		long long guid[2];
		if (GetGUIDByGUIDString(guid, guidStr, 0x80))
		{
			long long obj = GetObjectPtrByGUID(guid, 0x1, "", 0);
			if (obj) {
				char buffer[256];
				sprintf_s(buffer, "obj:  %016llx", obj);
				lua_pushstring(L, buffer);
				return 1;
			}
		}
	}
	return 0;
	/*long long worldBase = GetWorldBase();
	if (worldBase) {
		char buffer[256];
		sprintf_s(buffer, "obj:  %016llx", worldBase);
		lua_pushstring(L, buffer);
		return 1;
	}
	return 0;*/
}

functionPair_t *functionPairs = NULL;

void registerFunctions() {
	functionPair_t* next = functionPairs;
	while (next) {
		FrameScript_RegisterFunction(next->name, next->function);
		next = next->next;
	}
}

void addFunction(functionPair_t * item) {
	item->next = functionPairs;
	functionPairs = item;

}

functionPair_t* newFunction(char* name, scriptFunction_t* fcn) {
	functionPair_t* pair = (functionPair_t*)malloc(sizeof(functionPair_t));
	pair->name = name;
	pair->function = fcn;
	return pair;
}


scriptFunction_t* newJumper(scriptFunction_t* rawFunction, long offset = 0) {
	byte tmp[] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xFF, 0xE0 };
	byte* transmit;
	transmit = (byte*)(base + offset);
	memcpy(transmit, tmp, sizeof(tmp));
	void* fcnAddress = rawFunction;
	memcpy(transmit + 2, &fcnAddress, 8);
	return (scriptFunction_t*)transmit;
}

void lua_removefirst(lua_State* L) {
	long long lbase = *(long long*)((long long)L + 0x20);
	long long ltop = *(long long*)((long long)L + 0x18);
	long long lsize = (ltop - lbase) / 24;
	if (lsize > 1) {
		for (int i = 1; i < lsize; i++)
		{
			memcpy((void*)(lbase + (i - 1) * 24), (void*)(lbase + (i) * 24), 24);
		}

	}
	if (lsize > 0) {
		*(long long*)((long long)L + 0x18) = ltop - 24;
		//*(long*)((long long)L+0x20) = lbase+24;
	}
}

int AirjRouter(lua_State* L) {
	if (lua_checkstack(L, 1)) {
		const char* name = lua_tolstring(L, 1, NULL);
		functionPair_t* next = functionPairs;
		while (next) {
			if (strcmp(next->name, name) == 0)
			{
				lua_removefirst(L);
				return next->function(L);
			}
			next = next->next;
		}

	}
	else
	{
		long long guid[2];
		GetGUIDByUnitId("player", guid, 0, 0);
		long long obj = GetObjectPtrByGUID(guid, 0x1, "", 0);
		if (obj) {
			float position[4];
			getObjectPosition(obj, position);
			lua_pushnumber(L, position[3]);
			return 1;
		}
	}
	return 0;
}

void doHook2() {
	long long entry;
	// 	HANDLE hProcess = GetCurrentProcess();
	// 	entry = (long long)findMemory(hProcess,"40 53 48 83 EC 30 48 8B D9 E8 62 2C 0D 00 48 85",0);
	// 	if (entry == NULL){
	// 		entry = (long long)findMemory(hProcess,"48 B8 ?? ?? ?? ?? ?? ?? ?? ?? FF E0 0D 00 48 85",0);
	// 	}
	// 	CloseHandle(hProcess);
	entry = base + offsetGetPlayerFacing;
	if (entry) {
		byte jumpper[] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xFF, 0xE0 };
		void* fcn = AirjRouter;
		memcpy(jumpper + 2, &fcn, 8);
		memcpy((byte*)entry, jumpper, 12);
	}
}


void init() {
	GetBaseAddress(GetCurrentProcessId(), &wowBaseAddress, &nBaseSize);
	base = (long long)wowBaseAddress;

	// 
	// 	HANDLE hProcess = GetCurrentProcess();
	// 	long long start = 0x63A000;
	// 	for (int i = 0; i<20; i++)
	// 	{
	// 		start = (long long)findMemory(hProcess,"CC CC CC CC CC CC CC CC CC CC CC CC",start)-base;
	// 		offsetFreeSpaces[i] = start;
	// 		start += 0x10;
	// 	}
	// 	CloseHandle(hProcess);


	FrameScript_RegisterFunction = (void(*)(const char *functionName, scriptFunction_t* fcn))(base + offsetFrameScript_RegisterFunction);
	ClntObjMgrEnumVisibleObjects = (bool(*)(objCallback_t* callback, void* param))(base + offsetClntObjMgrEnumVisibleObjects);
	GetObjectPtrByGUID = (long long(*)(long long * guid, int mask, char* fileName, int line))(base + offsetGetObjectPtrByGUID);
	GetGUIDByUnitId = (bool(*)(const char* unit, long long* guid_out, long unk1/* = 0*/, long unk2/* = 0*/))(base + offsetGetGUIDByUnitId);
	GetNameByObjectPtr = (char* (*)(long long ObjectPtr, char ** realmName, long unk1/* = 1*/))(base + offsetGetNameByObjectPtr);
	GetGUIDStringByGUID = (void(*)(long long* guid, char* str_out, long unk1/* = 0x80*/))(base + offsetGetGUIDStringByGUID);
	GetGUIDByGUIDString = (long(*)(long long* guid_out, const char* guidStr, long unk1/* = 0x80*/))(base + offsetGetGUIDByGUIDString);
	InteractByGUID = (bool(*)(long long* guid))(base + offsetInteractByGUID);
	HandleTerrainClick = (bool(*)(float* positions))(base + offsetHandleTerrainClick);
	GetWorldBase = (long long(*)())(base + offsetGetWorldBase);
	UnitCanAttack = (bool(*)(long long s, long long d, long unk))(base + offsetUnitCanAttack);
	UnitCanAssist = (bool(*)(long long s, long long d, long unk1, long unk2))(base + offsetUnitCanAssist);

	lua_pushstring = (void(*)(lua_State *, const char *))(base + offsetLua_pushstring);
	lua_pushnumber = (void(*)(lua_State *, double))(base + offsetLua_pushnumber);
	lua_pushboolean = (void(*) (lua_State *L, bool b))(base + offsetLua_pushboolean);
	lua_tointeger = (int(*) (lua_State *, int))(base + offsetLua_tointeger);
	lua_tolstring = (const char* (*) (lua_State *, int, int*))(base + offsetLua_tolstring);

	lua_checkstack = (int(*)(lua_State *L, int sz))(base + offsetLua_checkstack);
	lua_tonumber = (double(*) (lua_State *L, int idx))(base + offsetLua_tonumber);

	lua_gettop = (int(*) (lua_State *L))(base + offsetLua_getTop);
	lua_settop = (void(*) (lua_State *L, int idx))(base + offsetLua_setTop);


	addFunction(newFunction("airj1", (listObjects)));
	addFunction(newFunction("AirjTest", (AirjTest)));
	addFunction(newFunction("AirjUpdateObjects", AirjUpdateObjects));
	addFunction(newFunction("AirjGetObjectGUID", AirjGetObjectGUID));
	addFunction(newFunction("AirjGetObjectGUIDByUnit", AirjGetObjectGUIDByUnit));
	addFunction(newFunction("AirjGetObjectType", AirjGetObjectType));
	addFunction(newFunction("AirjGetObjectDataInt", AirjGetObjectDataInt));
	addFunction(newFunction("AirjGetObjectDataFloat", AirjGetObjectDataFloat));
	addFunction(newFunction("AirjGetObjectPosition", AirjGetObjectPosition));
	addFunction(newFunction("AirjGetHealth", AirjGetHealth));
	addFunction(newFunction("AirjTargetByGUID", AirjTargetByGUID));
	addFunction(newFunction("AirjFocusByGUID", AirjFocusByGUID));
	addFunction(newFunction("AirjInteractByGUID", AirjInteractByGUID));
	addFunction(newFunction("AirjGetCamera", AirjGetCamera));
	addFunction(newFunction("AirjSetCameraDistance", AirjSetCameraDistance));
	addFunction(newFunction("AirjHandleTerrainClick", AirjHandleTerrainClick));
	addFunction(newFunction("AirjUnitCanAttack", AirjUnitCanAttack));
	addFunction(newFunction("AirjUnitCanAssist", AirjUnitCanAssist));
	
	// local guid, type, address = AirjGetObjectGUID(index)
	// local guid = AirjGetObjectGUIDByUnit(unit)
	// local type = AirjGetObjectType(guid)
	// local type = AirjGetObjectDataInt(guid,offset)
	// local x,y,z,facing = AirjGetObjectPosition(guid)
	// AirjTargetByGUID(guid)
	// AirjFocusByGUID(guid)
	// AirjInteractByGUID(guid)
	// local range, phi, theta, x, y, z = AirjGetCamera()

// 	addFunction(newFunction("airj1",newJumper(listObjects,offsetFreeSpaces[0])));
// 	addFunction(newFunction("AirjUpdateObjects",newJumper(AirjUpdateObjects,offsetFreeSpaces[1])));
// 	addFunction(newFunction("AirjGetObjectGUID",newJumper(AirjGetObjectGUID,offsetFreeSpaces[2])));
// 	addFunction(newFunction("AirjGetObjectGUIDByUnit",newJumper(AirjGetObjectGUIDByUnit,offsetFreeSpaces[3])));
// 	addFunction(newFunction("AirjGetObjectType",newJumper(AirjGetObjectType,offsetFreeSpaces[4])));
// 	addFunction(newFunction("AirjGetObjectDataInt",newJumper(AirjGetObjectDataInt,offsetFreeSpaces[5])));
// 	addFunction(newFunction("AirjGetObjectPosition",newJumper(AirjGetObjectPosition,offsetFreeSpaces[6])));
// 	addFunction(newFunction("AirjTargetByGUID",newJumper(AirjTargetByGUID,offsetFreeSpaces[7])));
// 	addFunction(newFunction("AirjFocusByGUID",newJumper(AirjFocusByGUID,offsetFreeSpaces[8])));
// 	addFunction(newFunction("AirjInteractByGUID",newJumper(AirjInteractByGUID,offsetFreeSpaces[9])));
// 	addFunction(newFunction("AirjGetCamera",newJumper(AirjGetCamera,offsetFreeSpaces[10])));
// 	addFunction(newFunction("AirjSetCameraDistance",newJumper(AirjSetCameraDistance,offsetFreeSpaces[11])));
}

void hook(void* from, void* fcn, int size = 12) {
	byte old[] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xFF, 0xE0 };
	byte step1[] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xFF, 0xD0 };
	byte step2[] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xFF, 0xE0 };
	byte tmp[128] = { 0 };
	memcpy(tmp, from, size);
	{
		int size1 = sizeof(step1), size2 = sizeof(step2);

		HANDLE hProcess = GetCurrentProcess();
		byte* transmit;
		//transmit = (byte*)malloc(size1 + size + size2);
		transmit = (byte*)VirtualAllocEx(hProcess, NULL, size1 + size + size2, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		memcpy(transmit, step1, size1);
		memcpy(transmit + size1, tmp, size);
		memcpy(transmit + size1 + size, step2, size2);

		memcpy(old + 2, &transmit, 8);
		memcpy(transmit + 2, &fcn, 8);
		void* back = (byte*)from + size;
		memcpy(transmit + size1 + size + 2, &back, 8);

		memcpy(from, old, size);
		CloseHandle(hProcess);
	}

}



void doHook() {
	hook((void*)(base + 0x31E1E0), registerFunctions, 12);
}


BOOL APIENTRY DllMain(HMODULE /* hModule */, DWORD ul_reason_for_call, LPVOID /* lpReserved */)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
		init();
		doHook2();
		// 		//doHook();
		// 		registerFunctions();
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}