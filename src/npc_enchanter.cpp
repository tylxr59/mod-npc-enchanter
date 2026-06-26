/*

# Enchanter NPC #

_This module was created for [StygianCore](https://rebrand.ly/stygiancoreproject). A World of Warcraft 3.3.5a Solo/LAN repack by StygianTheBest | [GitHub](https://rebrand.ly/stygiangithub) | [Website](https://rebrand.ly/stygianthebest))_

### Data ###
------------------------------------------------------------------------------------------------------------------
- Type: NPC (ID: 601015)
- Script: npc_enchantment
- Config: Yes
- SQL: No


### Version ###
------------------------------------------------------------------------------------------------------------------
- v2019.04.15 - Ported to AC by gtao725 (https://github.com/gtao725/)
- v2019.02.21 - Add AI/Phrases/Emotes, Update Menu
- v2018.12.05 - Fix broken menu. Replace 'Enchant Weapon' function. Add creature AI and creature text.
- v2018.12.01 - Update function, Add icons, Fix typos, Add a little personality (Emotes don't always work)
- v2017.08.08 - Release


### CREDITS
------------------------------------------------------------------------------------------------------------------
![Styx](https://stygianthebest.github.io/assets/img/avatar/avatar-128.jpg "Styx")
![StygianCore](https://stygianthebest.github.io/assets/img/projects/stygiancore/StygianCore.png "StygianCore")

##### This module was created for [StygianCore](https://rebrand.ly/stygiancoreproject). A World of Warcraft 3.3.5a Solo/LAN repack by StygianTheBest | [GitHub](https://rebrand.ly/stygiangithub) | [Website](https://rebrand.ly/stygianthebest))

#### Additional Credits

- [Blizzard Entertainment](http://blizzard.com)
- [TrinityCore](https://github.com/TrinityCore/TrinityCore/blob/3.3.5/THANKS)
- [SunwellCore](http://www.azerothcore.org/pages/sunwell.pl/)
- [AzerothCore](https://github.com/AzerothCore/azerothcore-wotlk/graphs/contributors)
- [OregonCore](https://wiki.oregon-core.net/)
- [Wowhead.com](http://wowhead.com)
- [OwnedCore](http://ownedcore.com/)
- [ModCraft.io](http://modcraft.io/)
- [MMO Society](https://www.mmo-society.com/)
- [AoWoW](https://wotlk.evowow.com/)
- [More credits are cited in the sources](https://github.com/StygianTheBest)

### LICENSE
------------------------------------------------------------------------------------------------------------------
This code and content is released under the [GNU AGPL v3](https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3).

*/

#include "Cell.h"
#include "CellImpl.h"
#include "Chat.h"
#include "CombatAI.h"
#include "Configuration/Config.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "GameEventMgr.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "GameObject.h"
#include "InstanceScript.h"
#include "ObjectMgr.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Unit.h"


enum EnchantCategory : uint32
{
    CAT_MAIN_WEAPON = 1,
    CAT_2H_WEAPON = 2,
    CAT_SHIELD = 3,
    CAT_HEAD = 4,
    CAT_SHOULDERS = 5,
    CAT_CLOAK = 6,
    CAT_CHEST = 7,
    CAT_BRACERS = 8,
    CAT_GLOVES = 9,
    CAT_LEGS = 10,
    CAT_BOOTS = 11,
    CAT_RINGS = 12,
    CAT_OFFHAND_WEAPON = 13,
    CAT_STAFF = 14,
    CAT_BACK = 300,
};

struct EnchantCategoryInfo
{
    uint32 Id;
    char const* Icon;
    char const* Title;
    uint32 MenuId;
};

struct EnchantOption
{
    uint32 Action;
    uint32 Category;
    uint8 Slot;
    uint32 EnchantId;
    uint32 RequiredItemLevel;
    uint32 RequiredSkill;
    uint32 RequiredSkillRank;
    char const* Label;
};

static EnchantCategoryInfo const EnchantCategories[] =
{
    {CAT_MAIN_WEAPON, "Inv_mace_116", "Enchant Main Weapon", 100002},
    {CAT_OFFHAND_WEAPON, "Inv_mace_116", "Enchant Offhand Weapon", 100014},
    {CAT_2H_WEAPON, "Inv_axe_113", "Enchant 2H Weapon", 100003},
    {CAT_STAFF, "inv_staff_13", "Enchant Staff", 100015},
    {CAT_SHIELD, "Inv_shield_71", "Enchant Shield", 100004},
    {CAT_HEAD, "inv_helmet_29", "Enchant Head", 100005},
    {CAT_SHOULDERS, "inv_shoulder_23", "Enchant Shoulders", 100006},
    {CAT_CLOAK, "Inv_misc_cape_18", "Enchant Cloak", 100007},
    {CAT_CHEST, "inv_chest_cloth_04", "Enchant Chest", 100008},
    {CAT_BRACERS, "inv_bracer_14", "Enchant Bracers", 100009},
    {CAT_GLOVES, "inv_gauntlets_06", "Enchant Gloves", 100010},
    {CAT_LEGS, "inv_pants_11", "Enchant Legs", 100011},
    {CAT_BOOTS, "inv_boots_05", "Enchant Boots", 100012},
    {CAT_RINGS, "Inv_jewelry_ring_85", "Enchant Rings", 100013},
};

static EnchantOption const EnchantOptions[] =
{
    {1000, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3788, 60, 0, 0, "[Accuracy] (60) +25 Hit Rating and +25 Critical Strike Rating"},
    {1001, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 2564, 0, 0, 0, "[Agility] +15 Agility"},
    {1002, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 2675, 35, 0, 0, "[Battlemaster] (35)"},
    {1003, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3789, 60, 0, 0, "[Berserking] (60)"},
    {1004, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3790, 60, 0, 0, "[Black Magic] (60)"},
    {1005, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3869, 75, 0, 0, "[Blade Ward] (75)"},
    {1006, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3870, 75, 0, 0, "[Blood Draining] (75)"},
    {1007, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 1900, 0, 0, 0, "[Crusader]"},
    {1008, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3273, 0, 0, 0, "[Deathfrost]"},
    {1009, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 912, 0, 0, 0, "[Demonslaying]"},
    {1010, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 1103, 60, 0, 0, "[Exceptional Agility] (60) +26 Agility"},
    {1011, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3830, 60, 0, 0, "[Exceptional Spellpower] (60) +50 Spell Power"},
    {1012, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3844, 60, 0, 0, "[Exceptional Spirit] (60) +45 Spirit"},
    {1013, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3225, 60, 0, 0, "[Executioner] (60)"},
    {1014, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 803, 0, 0, 0, "[Fiery Weapon]"},
    {1015, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3251, 60, 0, 0, "[Giant Slayer] (60) Giantslaying"},
    {1016, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3222, 35, 0, 0, "[Greater Agility] (35) +20 Agility"},
    {1017, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 1606, 60, 0, 0, "[Greater Potency] (60) +50 Attack Power"},
    {1018, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 805, 0, 0, 0, "[Greater Striking] +4 Weapon Damage"},
    {1019, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 2505, 0, 0, 0, "[Healing Power] +29 Spell Power"},
    {1020, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3239, 60, 0, 0, "[Icebreaker] (60) Icebreaker Weapon"},
    {1021, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 1894, 0, 0, 0, "[Icy Chill] Icy Weapon"},
    {1022, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 853, 0, 0, 0, "[Lesser Beastslayer] +6 Beastslaying"},
    {1023, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 854, 0, 0, 0, "[Lesser Elemental Slayer] +6 Elemental Slayer"},
    {1024, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 241, 0, 0, 0, "[Lesser Striking] +2 Weapon Damage"},
    {1025, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 1898, 0, 0, 0, "[Lifestealing]"},
    {1026, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3241, 60, 0, 0, "[Lifeward] (60)"},
    {1027, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3846, 35, 0, 0, "[Major Healing] (35) +40 Spell Power"},
    {1028, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 2666, 35, 0, 0, "[Major Intellect] (35) +30 Intellect"},
    {1029, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 2669, 35, 0, 0, "[Major Spellpower] (35) +40 Spell Power"},
    {1030, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 963, 35, 0, 0, "[Major Striking] (35) +7 Weapon Damage"},
    {1031, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 2568, 0, 0, 0, "[Mighty Intellect] +22 Intellect"},
    {1032, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3834, 60, 0, 0, "[Mighty Spellpower] (60) +63 Spell Power"},
    {1033, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 2567, 0, 0, 0, "[Mighty Spirit] +20 Spirit"},
    {1034, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 249, 0, 0, 0, "[Minor Beastslayer] +2 Beastslaying"},
    {1035, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 250, 0, 0, 0, "[Minor Striking] +1 Weapon Damage"},
    {1036, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 2673, 35, 0, 0, "[Mongoose] (35)"},
    {1037, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 2668, 35, 0, 0, "[Potency] (35) +20 Strength"},
    {1038, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 2672, 35, 0, 0, "[Soulfrost] (35) +54 Shadow and Frost Spell Power"},
    {1039, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 2504, 0, 0, 0, "[Spellpower] +30 Spell Power"},
    {1040, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 2674, 35, 0, 0, "[Spellsurge] (35)"},
    {1041, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 2563, 0, 0, 0, "[Strength] +15 Strength"},
    {1042, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 943, 0, 0, 0, "[Striking] +3 Weapon Damage"},
    {1043, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 2671, 35, 0, 0, "[Sunfire] (35) +50 Arcane and Fire Spell Power"},
    {1044, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3833, 60, 0, 0, "[Superior Potency] (60) +65 Attack Power"},
    {1045, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 1897, 0, 0, 0, "[Superior Striking] +5 Weapon Damage"},
    {1046, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3851, 60, 0, 0, "[Titanguard] (60) +50 Stamina"},
    {1047, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 1899, 0, 0, 0, "[Unholy Weapon]"},
    {1048, CAT_MAIN_WEAPON, EQUIPMENT_SLOT_MAINHAND, 2443, 0, 0, 0, "[Winter's Might] +7 Frost Spell Damage"},
    {1049, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 3788, 60, 0, 0, "[Accuracy] (60) +25 Hit Rating and +25 Critical Strike Rating"},
    {1050, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 2564, 0, 0, 0, "[Agility] +15 Agility"},
    {1051, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 2675, 35, 0, 0, "[Battlemaster] (35)"},
    {1052, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 3789, 60, 0, 0, "[Berserking] (60)"},
    {1053, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 3790, 60, 0, 0, "[Black Magic] (60)"},
    {1054, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 3869, 75, 0, 0, "[Blade Ward] (75)"},
    {1055, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 3870, 75, 0, 0, "[Blood Draining] (75)"},
    {1056, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 1900, 0, 0, 0, "[Crusader]"},
    {1057, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 3273, 0, 0, 0, "[Deathfrost]"},
    {1058, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 912, 0, 0, 0, "[Demonslaying]"},
    {1059, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 1103, 60, 0, 0, "[Exceptional Agility] (60) +26 Agility"},
    {1060, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 3830, 60, 0, 0, "[Exceptional Spellpower] (60) +50 Spell Power"},
    {1061, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 3844, 60, 0, 0, "[Exceptional Spirit] (60) +45 Spirit"},
    {1062, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 3225, 60, 0, 0, "[Executioner] (60)"},
    {1063, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 803, 0, 0, 0, "[Fiery Weapon]"},
    {1064, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 3251, 60, 0, 0, "[Giant Slayer] (60) Giantslaying"},
    {1065, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 3222, 35, 0, 0, "[Greater Agility] (35) +20 Agility"},
    {1066, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 1606, 60, 0, 0, "[Greater Potency] (60) +50 Attack Power"},
    {1067, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 805, 0, 0, 0, "[Greater Striking] +4 Weapon Damage"},
    {1068, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 2505, 0, 0, 0, "[Healing Power] +29 Spell Power"},
    {1069, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 3239, 60, 0, 0, "[Icebreaker] (60) Icebreaker Weapon"},
    {1070, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 1894, 0, 0, 0, "[Icy Chill] Icy Weapon"},
    {1071, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 853, 0, 0, 0, "[Lesser Beastslayer] +6 Beastslaying"},
    {1072, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 854, 0, 0, 0, "[Lesser Elemental Slayer] +6 Elemental Slayer"},
    {1073, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 241, 0, 0, 0, "[Lesser Striking] +2 Weapon Damage"},
    {1074, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 1898, 0, 0, 0, "[Lifestealing]"},
    {1075, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 3241, 60, 0, 0, "[Lifeward] (60)"},
    {1076, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 3846, 35, 0, 0, "[Major Healing] (35) +40 Spell Power"},
    {1077, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 2666, 35, 0, 0, "[Major Intellect] (35) +30 Intellect"},
    {1078, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 2669, 35, 0, 0, "[Major Spellpower] (35) +40 Spell Power"},
    {1079, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 963, 35, 0, 0, "[Major Striking] (35) +7 Weapon Damage"},
    {1080, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 2568, 0, 0, 0, "[Mighty Intellect] +22 Intellect"},
    {1081, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 3834, 60, 0, 0, "[Mighty Spellpower] (60) +63 Spell Power"},
    {1082, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 2567, 0, 0, 0, "[Mighty Spirit] +20 Spirit"},
    {1083, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 249, 0, 0, 0, "[Minor Beastslayer] +2 Beastslaying"},
    {1084, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 250, 0, 0, 0, "[Minor Striking] +1 Weapon Damage"},
    {1085, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 2673, 35, 0, 0, "[Mongoose] (35)"},
    {1086, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 2668, 35, 0, 0, "[Potency] (35) +20 Strength"},
    {1087, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 2672, 35, 0, 0, "[Soulfrost] (35) +54 Shadow and Frost Spell Power"},
    {1088, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 2504, 0, 0, 0, "[Spellpower] +30 Spell Power"},
    {1089, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 2674, 35, 0, 0, "[Spellsurge] (35)"},
    {1090, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 2563, 0, 0, 0, "[Strength] +15 Strength"},
    {1091, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 943, 0, 0, 0, "[Striking] +3 Weapon Damage"},
    {1092, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 2671, 35, 0, 0, "[Sunfire] (35) +50 Arcane and Fire Spell Power"},
    {1093, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 3833, 60, 0, 0, "[Superior Potency] (60) +65 Attack Power"},
    {1094, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 1897, 0, 0, 0, "[Superior Striking] +5 Weapon Damage"},
    {1095, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 3851, 60, 0, 0, "[Titanguard] (60) +50 Stamina"},
    {1096, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 1899, 0, 0, 0, "[Unholy Weapon]"},
    {1097, CAT_OFFHAND_WEAPON, EQUIPMENT_SLOT_OFFHAND, 2443, 0, 0, 0, "[Winter's Might] +7 Frost Spell Damage"},
    {1098, CAT_2H_WEAPON, EQUIPMENT_SLOT_MAINHAND, 2646, 0, 0, 0, "[Agility] +25 Agility"},
    {1099, CAT_2H_WEAPON, EQUIPMENT_SLOT_MAINHAND, 963, 0, 0, 0, "[Greater Impact] +7 Weapon Damage"},
    {1100, CAT_2H_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3828, 60, 0, 0, "[Greater Savagery] (60) +85 Attack Power"},
    {1101, CAT_2H_WEAPON, EQUIPMENT_SLOT_MAINHAND, 1897, 0, 0, 0, "[Impact] +5 Weapon Damage"},
    {1102, CAT_2H_WEAPON, EQUIPMENT_SLOT_MAINHAND, 943, 0, 0, 0, "[Lesser Impact] +3 Weapon Damage"},
    {1103, CAT_2H_WEAPON, EQUIPMENT_SLOT_MAINHAND, 723, 0, 0, 0, "[Lesser Intellect] +3 Intellect"},
    {1104, CAT_2H_WEAPON, EQUIPMENT_SLOT_MAINHAND, 255, 0, 0, 0, "[Lesser Spirit] +3 Spirit"},
    {1105, CAT_2H_WEAPON, EQUIPMENT_SLOT_MAINHAND, 2670, 35, 0, 0, "[Major Agility] (35) +35 Agility"},
    {1106, CAT_2H_WEAPON, EQUIPMENT_SLOT_MAINHAND, 1904, 0, 0, 0, "[Major Intellect] +9 Intellect"},
    {1107, CAT_2H_WEAPON, EQUIPMENT_SLOT_MAINHAND, 1903, 0, 0, 0, "[Major Spirit] +9 Spirit"},
    {1108, CAT_2H_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3827, 60, 0, 0, "[Massacre] (60) +110 Attack Power"},
    {1109, CAT_2H_WEAPON, EQUIPMENT_SLOT_MAINHAND, 241, 0, 0, 0, "[Minor Impact] +2 Weapon Damage"},
    {1110, CAT_2H_WEAPON, EQUIPMENT_SLOT_MAINHAND, 2667, 35, 0, 0, "[Savagery] (35) +70 Attack Power"},
    {1111, CAT_2H_WEAPON, EQUIPMENT_SLOT_MAINHAND, 3247, 60, 0, 0, "[Scourgebane] (60) +140 Attack Power versus Undead"},
    {1112, CAT_2H_WEAPON, EQUIPMENT_SLOT_MAINHAND, 1896, 0, 0, 0, "[Superior Impact] +9 Weapon Damage"},
    {1113, CAT_STAFF, EQUIPMENT_SLOT_MAINHAND, 3854, 60, 0, 0, "[Greater Spellpower] (60) +81 Spell Power"},
    {1114, CAT_STAFF, EQUIPMENT_SLOT_MAINHAND, 3855, 60, 0, 0, "[Spellpower] (60) +69 Spell Power"},
    {1115, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 1952, 60, 0, 0, "[Defense] (60) +20 Defense Rating"},
    {1116, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 926, 0, 0, 0, "[Frost Resistance] +8 Frost Resistance"},
    {1117, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 1128, 60, 0, 0, "[Greater Intellect] (60) +25 Intellect"},
    {1118, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 907, 0, 0, 0, "[Greater Spirit] +7 Spirit"},
    {1119, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 929, 0, 0, 0, "[Greater Stamina] +7 Stamina"},
    {1120, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 2654, 35, 0, 0, "[Intellect] (35) +12 Intellect"},
    {1121, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 863, 0, 0, 0, "[Lesser Block] +10 Shield Block Rating"},
    {1122, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 848, 0, 0, 0, "[Lesser Protection] +30 Armor"},
    {1123, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 255, 0, 0, 0, "[Lesser Spirit] +3 Spirit"},
    {1124, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 724, 0, 0, 0, "[Lesser Stamina] +3 Stamina"},
    {1125, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 1071, 35, 0, 0, "[Major Stamina] (35) +18 Stamina"},
    {1126, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 66, 0, 0, 0, "[Minor Stamina] +1 Stamina"},
    {1127, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 3229, 35, 0, 0, "[Resilience] (35) +12 Resilience Rating"},
    {1128, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 1888, 35, 0, 0, "[Resistance] (35) +5 All Resistances"},
    {1129, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 2655, 35, 0, 0, "[Shield Block] (35) +15 Shield Block Rating"},
    {1130, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 851, 0, 0, 0, "[Spirit] +5 Spirit"},
    {1131, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 852, 0, 0, 0, "[Stamina] +5 Stamina"},
    {1132, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 2653, 35, 0, 0, "[Tough Shield] (35) +36 Block Value"},
    {1133, CAT_SHIELD, EQUIPMENT_SLOT_OFFHAND, 1890, 0, 0, 0, "[Vitality] +4 Mana and Health every 5 sec"},
    {1134, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3006, 70, 0, 0, "[Arcanum of Arcane Warding] (70) +20 Arcane Resistance"},
    {1135, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3819, 80, 0, 0, "[Arcanum of Blissful Mending] (80) +30 Spell Power and 10 mana per 5 seconds."},
    {1136, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3820, 80, 0, 0, "[Arcanum of Burning Mysteries] (80) +30 Spell Power and 20 Critical strike rating."},
    {1137, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3797, 80, 0, 0, "[Arcanum of Dominance] (80) +29 Spell Power and +20 Resilience Rating"},
    {1138, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3003, 70, 0, 0, "[Arcanum of Ferocity] (70) +34 Attack Power and +16 Hit Rating"},
    {1139, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3007, 70, 0, 0, "[Arcanum of Fire Warding] (70) +20 Fire Resistance"},
    {1140, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 2544, 50, 0, 0, "[Arcanum of Focus] (50) +8 Spell Power"},
    {1141, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3008, 70, 0, 0, "[Arcanum of Frost Warding] (70) +20 Frost Resistance"},
    {1142, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3005, 70, 0, 0, "[Arcanum of Nature Warding] (70) +20 Nature Resistance"},
    {1143, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3002, 70, 0, 0, "[Arcanum of Power] (70) +22 Spell Power and +14 Hit Rating"},
    {1144, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 2545, 50, 0, 0, "[Arcanum of Protection] (50) +12 Dodge Rating"},
    {1145, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 2543, 50, 0, 0, "[Arcanum of Rapidity] (50) +10 Haste Rating"},
    {1146, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3001, 70, 0, 0, "[Arcanum of Renewal] (70) +19 Spell Power and +9 Mana every 5 seconds"},
    {1147, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3009, 70, 0, 0, "[Arcanum of Shadow Warding] (70) +20 Shadow Resistance"},
    {1148, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 2999, 70, 0, 0, "[Arcanum of the Defender] (70) +16 Defense Rating and +17 Dodge Rating"},
    {1149, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3815, 80, 0, 0, "[Arcanum of the Eclipsed Moon] (80) +25 Arcane Resistance and +30 Stamina"},
    {1150, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3816, 80, 0, 0, "[Arcanum of the Flame's Soul] (80) +25 Fire Resistance and +30 Stamina"},
    {1151, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3814, 80, 0, 0, "[Arcanum of the Fleeing Shadow] (80) +25 Shadow Resistance and +30 Stamina"},
    {1152, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3812, 80, 0, 0, "[Arcanum of the Frosty Soul] (80) +25 Frost Resistance and +30 Stamina"},
    {1153, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3004, 70, 0, 0, "[Arcanum of the Gladiator] (70) +18 Stamina and +20 Resilience Rating"},
    {1154, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3096, 70, 0, 0, "[Arcanum of the Outcast] (70) +17 Strength and +16 Intellect"},
    {1155, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3842, 80, 0, 0, "[Arcanum of the Savage Gladiator] (80) +30 Stamina and +25 Resilience Rating"},
    {1156, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3818, 80, 0, 0, "[Arcanum of the Stalwart Protector] (80) +37 Stamina and +20 Defense Rating"},
    {1157, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3817, 80, 0, 0, "[Arcanum of Torment] (80) +50 Attack Power and +20 Critical Strike Rating"},
    {1158, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3813, 80, 0, 0, "[Arcanum of Toxic Warding] (80) +25 Nature Resistance and +30 Stamina"},
    {1159, CAT_HEAD, EQUIPMENT_SLOT_HEAD, 3795, 80, 0, 0, "[Arcanum of Triumph] (80) +50 Attack Power and +20 Resilience Rating"},
    {1160, CAT_SHOULDERS, EQUIPMENT_SLOT_SHOULDERS, 2982, 70, 0, 0, "[Greater Inscription of Discipline] (70) +18 Spell Power and +10 Critical Strike Rating"},
    {1161, CAT_SHOULDERS, EQUIPMENT_SLOT_SHOULDERS, 2980, 70, 0, 0, "[Greater Inscription of Faith] (70) +18 Spell Power and +5 Mana every 5 seconds"},
    {1162, CAT_SHOULDERS, EQUIPMENT_SLOT_SHOULDERS, 3808, 80, 0, 0, "[Greater Inscription of the Axe] (80) +40 Attack Power and +15 Crit Rating"},
    {1163, CAT_SHOULDERS, EQUIPMENT_SLOT_SHOULDERS, 2997, 70, 0, 0, "[Greater Inscription of the Blade] (70) +15 Critical Strike Rating and +20 Attack Power"},
    {1164, CAT_SHOULDERS, EQUIPMENT_SLOT_SHOULDERS, 3809, 80, 0, 0, "[Greater Inscription of the Crag] (80) +24 Spell Power and +8 Mana per 5 sec"},
    {1165, CAT_SHOULDERS, EQUIPMENT_SLOT_SHOULDERS, 3852, 70, 0, 0, "[Greater Inscription of the Gladiator] (70) +30 Stamina and +15 Resilience Rating"},
    {1166, CAT_SHOULDERS, EQUIPMENT_SLOT_SHOULDERS, 2991, 70, 0, 0, "[Greater Inscription of the Knight] (70) +15 Defense Rating and +10 Dodge Rating"},
    {1167, CAT_SHOULDERS, EQUIPMENT_SLOT_SHOULDERS, 2993, 70, 0, 0, "[Greater Inscription of the Oracle] (70) +12 Spell Power and 8 Mana every 5 seconds"},
    {1168, CAT_SHOULDERS, EQUIPMENT_SLOT_SHOULDERS, 2995, 70, 0, 0, "[Greater Inscription of the Orb] (70) +15 Critical Strike Rating and +12 Spell Power"},
    {1169, CAT_SHOULDERS, EQUIPMENT_SLOT_SHOULDERS, 3811, 80, 0, 0, "[Greater Inscription of the Pinnacle] (80) +20 Dodge Rating and +15 Defense Rating"},
    {1170, CAT_SHOULDERS, EQUIPMENT_SLOT_SHOULDERS, 3810, 80, 0, 0, "[Greater Inscription of the Storm] (80) +24 Spell Power and +15 Critical Strike Rating"},
    {1171, CAT_SHOULDERS, EQUIPMENT_SLOT_SHOULDERS, 2986, 70, 0, 0, "[Greater Inscription of Vengeance] (70) +30 Attack Power and +10 Critical Strike Rating"},
    {1172, CAT_SHOULDERS, EQUIPMENT_SLOT_SHOULDERS, 2978, 70, 0, 0, "[Greater Inscription of Warding] (70) +15 Dodge Rating and +10 Defense Rating"},
    {1173, CAT_SHOULDERS, EQUIPMENT_SLOT_SHOULDERS, 3875, 80, 0, 0, "[Lesser Inscription of the Axe] (80) +30 Attack Power and +10 Critical Strike Rating"},
    {1174, CAT_SHOULDERS, EQUIPMENT_SLOT_SHOULDERS, 3807, 80, 0, 0, "[Lesser Inscription of the Crag] (80) +18 Spell Power and +5 Mana per 5 sec"},
    {1175, CAT_SHOULDERS, EQUIPMENT_SLOT_SHOULDERS, 3876, 80, 0, 0, "[Lesser Inscription of the Pinnacle] (80) +15 Dodge Rating and +10 Defense Rating"},
    {1176, CAT_SHOULDERS, EQUIPMENT_SLOT_SHOULDERS, 3806, 80, 0, 0, "[Lesser Inscription of the Storm] (80) +18 Spell Power and +10 Critical Strike Rating"},
    {1177, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 848, 0, 0, 0, "[Defense] +30 Armor"},
    {1178, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 2622, 35, 0, 0, "[Dodge] (35) +12 Dodge Rating"},
    {1179, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 2463, 0, 0, 0, "[Fire Resistance] +7 Fire Resistance"},
    {1180, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 368, 35, 0, 0, "[Greater Agility] (35) +12 Agility"},
    {1181, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 1257, 35, 0, 0, "[Greater Arcane Resistance] (35) +15 Arcane Resistance"},
    {1182, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 884, 0, 0, 0, "[Greater Defense] +50 Armor"},
    {1183, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 2619, 0, 0, 0, "[Greater Fire Resistance] +15 Fire Resistance"},
    {1184, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 2620, 0, 0, 0, "[Greater Nature Resistance] +15 Nature Resistance"},
    {1185, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 1888, 0, 0, 0, "[Greater Resistance] +5 All Resistances"},
    {1186, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 1441, 35, 0, 0, "[Greater Shadow Resistance] (35) +15 Shadow Resistance"},
    {1187, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 3831, 60, 0, 0, "[Greater Speed] (60) +23 Haste Rating"},
    {1188, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 849, 0, 0, 0, "[Lesser Agility] +3 Agility"},
    {1189, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 256, 0, 0, 0, "[Lesser Fire Resistance] +5 Fire Resistance"},
    {1190, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 744, 0, 0, 0, "[Lesser Protection] +20 Armor"},
    {1191, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 804, 0, 0, 0, "[Lesser Shadow Resistance] +10 Shadow Resistance"},
    {1192, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 1099, 60, 0, 0, "[Major Agility] (60) +22 Agility"},
    {1193, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 2662, 35, 0, 0, "[Major Armor] (35) +120 Armor"},
    {1194, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 2664, 35, 0, 0, "[Major Resistance] (35) +7 Resist All"},
    {1195, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 3294, 60, 0, 0, "[Mighty Armor] (60) +225 Armor"},
    {1196, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 247, 0, 0, 0, "[Minor Agility] +1 Agility"},
    {1197, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 783, 0, 0, 0, "[Minor Protection] +10 Armor"},
    {1198, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 65, 0, 0, 0, "[Minor Resistance] +1 All Resistances"},
    {1199, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 903, 0, 0, 0, "[Resistance] +3 All Resistances"},
    {1200, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 3256, 60, 0, 0, "[Shadow Armor] (60) Increased Stealth and +10 Agility"},
    {1201, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 3825, 60, 0, 0, "[Speed] (60) +15 Haste Rating"},
    {1202, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 2938, 35, 0, 0, "[Spell Penetration] (35) +20 Spell Penetration"},
    {1203, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 3243, 60, 0, 0, "[Spell Piercing] (60) +35 Spell Penetration"},
    {1204, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 910, 0, 0, 0, "[Stealth] Increased Stealth"},
    {1205, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 2648, 35, 0, 0, "[Steelweave] (35) +12 Defense Rating"},
    {1206, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 2621, 0, 0, 0, "[Subtlety] 2% Reduced Threat"},
    {1207, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 983, 60, 0, 0, "[Superior Agility] (60) +16 Agility"},
    {1208, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 1262, 60, 0, 0, "[Superior Arcane Resistance] (60) +20 Arcane Resistance"},
    {1209, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 1889, 0, 0, 0, "[Superior Defense] +70 Armor"},
    {1210, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 1354, 60, 0, 0, "[Superior Fire Resistance] (60) +20 Fire Resistance"},
    {1211, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 3230, 60, 0, 0, "[Superior Frost Resistance] (60) +20 Frost Resistance"},
    {1212, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 1400, 60, 0, 0, "[Superior Nature Resistance] (60) +20 Nature Resistance"},
    {1213, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 1446, 60, 0, 0, "[Superior Shadow Resistance] (60) +20 Shadow Resistance"},
    {1214, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 1951, 60, 0, 0, "[Titanweave] (60) +16 Defense Rating"},
    {1215, CAT_CLOAK, EQUIPMENT_SLOT_BACK, 3296, 60, 0, 0, "[Wisdom] (60) +10 Spirit and 2% Reduced Threat"},
    {1216, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 1951, 35, 0, 0, "[Defense] (35) +16 Defense Rating"},
    {1217, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 2659, 35, 0, 0, "[Exceptional Health] (35) +150 Health"},
    {1218, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 3233, 60, 0, 0, "[Exceptional Mana] (60) +250 Mana"},
    {1219, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 3245, 60, 0, 0, "[Exceptional Resilience] (60) +20 Resilience Rating"},
    {1220, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 2661, 35, 0, 0, "[Exceptional Stats] (35) +6 All Stats"},
    {1221, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 1953, 60, 0, 0, "[Greater Defense] (60) +22 Defense Rating"},
    {1222, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 850, 0, 0, 0, "[Greater Health] +35 Health"},
    {1223, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 857, 0, 0, 0, "[Greater Mana] +50 Mana"},
    {1224, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 2381, 60, 0, 0, "[Greater Mana Restoration] (60) +10 mana every 5 sec."},
    {1225, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 1891, 0, 0, 0, "[Greater Stats] +4 All Stats"},
    {1226, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 254, 0, 0, 0, "[Health] +25 Health"},
    {1227, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 63, 0, 0, 0, "[Lesser Absorption] Absorption (25)"},
    {1228, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 242, 0, 0, 0, "[Lesser Health] +15 Health"},
    {1229, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 246, 0, 0, 0, "[Lesser Mana] +20 Mana"},
    {1230, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 866, 0, 0, 0, "[Lesser Stats] +2 All Stats"},
    {1231, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 1892, 0, 0, 0, "[Major Health] +100 Health"},
    {1232, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 1893, 0, 0, 0, "[Major Mana] +100 Mana"},
    {1233, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 2933, 35, 0, 0, "[Major Resilience] (35) +15 Resilience Rating"},
    {1234, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 1144, 35, 0, 0, "[Major Spirit] (35) +15 Spirit"},
    {1235, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 843, 0, 0, 0, "[Mana] +30 Mana"},
    {1236, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 3236, 60, 0, 0, "[Mighty Health] (60) +200 Health"},
    {1237, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 44, 0, 0, 0, "[Minor Absorption] Absorption (10)"},
    {1238, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 41, 0, 0, 0, "[Minor Health] +5 Health"},
    {1239, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 24, 0, 0, 0, "[Minor Mana] +5 Mana"},
    {1240, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 847, 0, 0, 0, "[Minor Stats] +1 All Stats"},
    {1241, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 3832, 60, 0, 0, "[Powerful Stats] (60) +10 All Stats"},
    {1242, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 3150, 35, 0, 0, "[Restore Mana Prime] (35) +7 mana every 5 sec."},
    {1243, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 928, 0, 0, 0, "[Stats] +3 All Stats"},
    {1244, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 3297, 60, 0, 0, "[Super Health] (60) +275 Health"},
    {1245, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 3252, 60, 0, 0, "[Super Stats] (60) +8 All Stats"},
    {1246, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 908, 0, 0, 0, "[Superior Health] +50 Health"},
    {1247, CAT_CHEST, EQUIPMENT_SLOT_CHEST, 913, 0, 0, 0, "[Superior Mana] +65 Mana"},
    {1248, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 3756, 0, 165, 400, "[Fur Lining - Attack Power] +130 Attack Power"},
    {1249, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 1593, 35, 0, 0, "[Assault] (35) +24 Attack Power"},
    {1250, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 2647, 35, 0, 0, "[Brawn] (35) +12 Strength"},
    {1251, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 923, 0, 0, 0, "[Deflection] +5 Defense Rating"},
    {1252, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 3231, 60, 0, 0, "[Expertise] (60) +15 Expertise Rating"},
    {1253, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 2649, 35, 0, 0, "[Fortitude] (35) +12 Stamina"},
    {1254, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 1883, 0, 0, 0, "[Greater Intellect] +7 Intellect"},
    {1255, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 907, 0, 0, 0, "[Greater Spirit] +7 Spirit"},
    {1256, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 929, 0, 0, 0, "[Greater Stamina] +7 Stamina"},
    {1257, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 927, 0, 0, 0, "[Greater Strength] +7 Strength"},
    {1258, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 2650, 0, 0, 0, "[Healing Power] +15 Spell Power"},
    {1259, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 905, 0, 0, 0, "[Intellect] +5 Intellect"},
    {1260, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 925, 0, 0, 0, "[Lesser Deflection] +3 Defense Rating"},
    {1261, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 723, 0, 0, 0, "[Lesser Intellect] +3 Intellect"},
    {1262, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 255, 0, 0, 0, "[Lesser Spirit] +3 Spirit"},
    {1263, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 724, 0, 0, 0, "[Lesser Stamina] +3 Stamina"},
    {1264, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 823, 0, 0, 0, "[Lesser Strength] +3 Strength"},
    {1265, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 2648, 35, 0, 0, "[Major Defense] (35) +12 Defense Rating"},
    {1266, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 369, 35, 0, 0, "[Major Intellect] (35) +12 Intellect"},
    {1267, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 3850, 60, 0, 0, "[Major Stamina] (60) +40 Stamina"},
    {1268, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 2565, 0, 0, 0, "[Mana Regeneration] Mana Regen 5 per 5 sec."},
    {1269, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 247, 0, 0, 0, "[Minor Agility] +1 Agility"},
    {1270, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 924, 0, 0, 0, "[Minor Deflection] +2 Defense Rating"},
    {1271, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 41, 0, 0, 0, "[Minor Health] +5 Health"},
    {1272, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 243, 0, 0, 0, "[Minor Spirit] +1 Spirit"},
    {1273, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 66, 0, 0, 0, "[Minor Stamina] +1 Stamina"},
    {1274, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 248, 0, 0, 0, "[Minor Strength] +1 Strength"},
    {1275, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 2679, 35, 0, 0, "[Restore Mana Prime] (35) 8 Mana per 5 Sec."},
    {1276, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 2650, 35, 0, 0, "[Spellpower] (35) +15 Spell Power"},
    {1277, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 851, 0, 0, 0, "[Spirit] +5 Spirit"},
    {1278, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 852, 0, 0, 0, "[Stamina] +5 Stamina"},
    {1279, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 1891, 35, 0, 0, "[Stats] (35) +4 All Stats"},
    {1280, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 856, 0, 0, 0, "[Strength] +5 Strength"},
    {1281, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 2650, 35, 0, 0, "[Superior Healing] (35) +15 Spell Power"},
    {1282, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 2332, 60, 0, 0, "[Superior Spellpower] (60) +30 Spell Power"},
    {1283, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 1884, 0, 0, 0, "[Superior Spirit] +9 Spirit"},
    {1284, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 1886, 0, 0, 0, "[Superior Stamina] +9 Stamina"},
    {1285, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 1885, 0, 0, 0, "[Superior Strength] +9 Strength"},
    {1286, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 1119, 60, 0, 0, "[Exceptional Intellect] (60) +16 Intellect"},
    {1287, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 3845, 60, 0, 0, "[Greater Assault] (60) +50 Attack Power"},
    {1288, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 2326, 60, 0, 0, "[Greater Spellpower] (60) +23 Spell Power"},
    {1289, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 2661, 60, 0, 0, "[Greater Stats] (60) +6 All Stats"},
    {1290, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 1147, 60, 0, 0, "[Major Spirit] (60) +18 Spirit"},
    {1291, CAT_BRACERS, EQUIPMENT_SLOT_WRISTS, 1600, 60, 0, 0, "[Striking] (60) +38 Attack Power"},
    {1292, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 3603, 0, 202, 400, "[Hand-Mounted Pyro Rocket]"},
    {1293, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 3604, 0, 202, 400, "[Hyperspeed Accelerators]"},
    {1294, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 909, 0, 0, 0, "[Advanced Herbalism] +5 Herbalism"},
    {1295, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 906, 0, 0, 0, "[Advanced Mining] +5 Mining"},
    {1296, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 904, 0, 0, 0, "[Agility] +5 Agility"},
    {1297, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 846, 0, 0, 0, "[Angler] +5 Fishing"},
    {1298, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 3253, 60, 0, 0, "[Armsman] (60) +2% Threat and 10 Parry Rating"},
    {1299, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 1594, 35, 0, 0, "[Assault] (35) +26 Attack Power"},
    {1300, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 2934, 35, 0, 0, "[Blasting] (35) +10 Critical Strike Rating"},
    {1301, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 1603, 60, 0, 0, "[Crusher] (60) +44 Attack Power"},
    {1302, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 3246, 60, 0, 0, "[Exceptional Spellpower] (60) +28 Spell Power"},
    {1303, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 3231, 60, 0, 0, "[Expertise] (60) +15 Expertise Rating"},
    {1304, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 2616, 0, 0, 0, "[Fire Power] +20 Fire Spell Power"},
    {1305, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 2603, 0, 0, 0, "[Fishing] +2 Fishing"},
    {1306, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 2615, 0, 0, 0, "[Frost Power] +20 Frost Spell Power"},
    {1307, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 3238, 60, 0, 0, "[Gatherer] (60)"},
    {1308, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 1887, 0, 0, 0, "[Greater Agility] +7 Agility"},
    {1309, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 3829, 60, 0, 0, "[Greater Assault] (60) +35 Attack Power"},
    {1310, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 3249, 60, 0, 0, "[Greater Blasting] (60) +16 Critical Strike Rating"},
    {1311, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 927, 0, 0, 0, "[Greater Strength] +7 Strength"},
    {1312, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 2617, 0, 0, 0, "[Healing Power] +16 Spell Power"},
    {1313, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 845, 0, 0, 0, "[Herbalism] +2 Herbalism"},
    {1314, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 3222, 60, 0, 0, "[Major Agility] (60) +20 Agility"},
    {1315, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 2322, 35, 0, 0, "[Major Healing] (35) +19 Spell Power"},
    {1316, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 2937, 35, 0, 0, "[Major Spellpower] (35) +20 Spell Power"},
    {1317, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 684, 35, 0, 0, "[Major Strength] (35) +15 Strength"},
    {1318, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 844, 0, 0, 0, "[Mining] +2 Mining"},
    {1319, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 931, 0, 0, 0, "[Minor Haste] +10 Haste Rating"},
    {1320, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 2935, 35, 0, 0, "[Precise Strikes] (35) +15 Hit Rating"},
    {1321, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 3234, 60, 0, 0, "[Precision] (60) +20 Hit Rating"},
    {1322, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 930, 0, 0, 0, "[Riding Skill] +2% Mount Speed"},
    {1323, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 2614, 0, 0, 0, "[Shadow Power] +20 Shadow Spell Power"},
    {1324, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 865, 0, 0, 0, "[Skinning] +5 Skinning"},
    {1325, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 856, 0, 0, 0, "[Strength] +5 Strength"},
    {1326, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 2564, 0, 0, 0, "[Superior Agility] +15 Agility"},
    {1327, CAT_GLOVES, EQUIPMENT_SLOT_HANDS, 2613, 0, 0, 0, "[Threat] +2% Threat"},
    {1328, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 3720, 70, 0, 0, "[Azure Spellthread] (70) +35 Spell Power and +20 Stamina"},
    {1329, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 3719, 70, 0, 0, "[Brilliant Spellthread] (70) +50 Spell Power and +20 Spirit"},
    {1330, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 3011, 50, 0, 0, "[Clefthide Leg Armor] (50) +30 Stamina and +10 Agility"},
    {1331, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 3010, 50, 0, 0, "[Cobrahide Leg Armor] (50) +40 Attack Power and +10 Critical Strike Rating"},
    {1332, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 3331, 70, 0, 0, "[Dragonscale Leg Armor] (70) +72 Stamina and +35 Agility"},
    {1333, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 3853, 80, 0, 0, "[Earthen Leg Armor] (80) +40 Resilience Rating and +28 Stamina"},
    {1334, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 3822, 80, 0, 0, "[Frosthide Leg Armor] (80) +55 Stamina and +22 Agility"},
    {1335, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 2746, 60, 0, 0, "[Golden Spellthread] (60) +35 Spell Power and +20 Stamina"},
    {1336, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 3823, 80, 0, 0, "[Icescale Leg Armor] (80) +75 Attack Power and +22 Critical Strike Rating"},
    {1337, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 3325, 70, 0, 0, "[Jormungar Leg Armor] (70) +45 Stamina and +15 Agility"},
    {1338, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 2747, 50, 0, 0, "[Mystic Spellthread] (50) +25 Spell Power and +15 Stamina"},
    {1339, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 3326, 70, 0, 0, "[Nerubian Leg Armor] (70) +55 Attack Power and +15 Critical Strike Rating"},
    {1340, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 3013, 60, 0, 0, "[Nethercleft Leg Armor] (60) +40 Stamina and +12 Agility"},
    {1341, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 3012, 60, 0, 0, "[Nethercobra Leg Armor] (60) +50 Attack Power and +12 Critical Strike Rating"},
    {1342, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 2748, 60, 0, 0, "[Runic Spellthread] (60) +35 Spell Power and +20 Stamina"},
    {1343, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 3721, 70, 0, 0, "[Sapphire Spellthread] (70) +50 Spell Power and +30 Stamina"},
    {1344, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 3718, 70, 0, 0, "[Shining Spellthread] (70) +35 Spell Power and +12 Spirit"},
    {1345, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 2745, 50, 0, 0, "[Silver Spellthread] (50) +25 Spell Power and +15 Stamina"},
    {1346, CAT_LEGS, EQUIPMENT_SLOT_LEGS, 3332, 70, 0, 0, "[Wyrmscale Leg Armor] (70) +100 Attack Power and +36 Critical Strike Rating"},
    {1347, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 3606, 0, 202, 400, "[Nitro Boosts] +24 Critical Strike Rating"},
    {1348, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 904, 0, 0, 0, "[Agility] +5 Agility"},
    {1349, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 3824, 60, 0, 0, "[Assault] (60) +24 Attack Power"},
    {1350, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 2940, 35, 0, 0, "[Boar's Speed] (35) Minor Speed and +9 Stamina"},
    {1351, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 2939, 35, 0, 0, "[Cat's Swiftness] (35) Minor Speed and +6 Agility"},
    {1352, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 2657, 35, 0, 0, "[Dexterity] (35) +12 Agility"},
    {1353, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 2649, 35, 0, 0, "[Fortitude] (35) +12 Stamina"},
    {1354, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 1887, 0, 0, 0, "[Greater Agility] +7 Agility"},
    {1355, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 1597, 60, 0, 0, "[Greater Assault] (60) +32 Attack Power"},
    {1356, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 1075, 60, 0, 0, "[Greater Fortitude] (60) +22 Stamina"},
    {1357, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 1147, 60, 0, 0, "[Greater Spirit] (60) +18 Spirit"},
    {1358, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 929, 0, 0, 0, "[Greater Stamina] +7 Stamina"},
    {1359, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 3244, 60, 0, 0, "[Greater Vitality] (60) +7 Health and Mana every 5 sec"},
    {1360, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 3826, 60, 0, 0, "[Icewalker] (60) +12 Hit Rating and +12 Critical Strike Rating"},
    {1361, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 3858, 0, 0, 0, "[Lesser Accuracy] +5 Hit Rating"},
    {1362, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 849, 0, 0, 0, "[Lesser Agility] +3 Agility"},
    {1363, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 255, 0, 0, 0, "[Lesser Spirit] +3 Spirit"},
    {1364, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 724, 0, 0, 0, "[Lesser Stamina] +3 Stamina"},
    {1365, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 247, 0, 0, 0, "[Minor Agility] +1 Agility"},
    {1366, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 911, 0, 0, 0, "[Minor Speed] Minor Speed Increase"},
    {1367, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 66, 0, 0, 0, "[Minor Stamina] +1 Stamina"},
    {1368, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 851, 0, 0, 0, "[Spirit] +5 Spirit"},
    {1369, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 852, 0, 0, 0, "[Stamina] +5 Stamina"},
    {1370, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 983, 60, 0, 0, "[Superior Agility] (60) +16 Agility"},
    {1371, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 2658, 35, 0, 0, "[Surefooted] (35) +10 Hit Rating and +10 Critical Strike Rating"},
    {1372, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 3232, 0, 0, 0, "[Tuskarr's Vitality] +15 Stamina and Minor Speed Increase"},
    {1373, CAT_BOOTS, EQUIPMENT_SLOT_FEET, 2656, 35, 0, 0, "[Vitality] (35) +5 Health and Mana every 5 sec"},
    {1374, CAT_RINGS, EQUIPMENT_SLOT_FINGER1, 3839, 0, 333, 450, "[Assault] +40 Attack Power"},
    {1375, CAT_RINGS, EQUIPMENT_SLOT_FINGER1, 3840, 0, 333, 450, "[Greater Spellpower] +23 Spell Power"},
    {1376, CAT_RINGS, EQUIPMENT_SLOT_FINGER1, 3791, 0, 333, 450, "[Stamina] +30 Stamina"},
};

class EnchanterConfig : public WorldScript
{
public:
    EnchanterConfig() : WorldScript("EnchanterConfig_conf", {
        WORLDHOOK_ON_BEFORE_CONFIG_LOAD
    }) { }

    void OnBeforeConfigLoad(bool reload) override
    {
        if (!reload) {
            EnchanterEnableModule = sConfigMgr->GetOption<bool>("Enchanter.Enable", 1);
            EnchanterAnnounceModule = sConfigMgr->GetOption<bool>("Enchanter.Announce", 1);
            EnchanterNumPhrases = sConfigMgr->GetOption<uint32>("Enchanter.NumPhrases", 3);
            EnchanterMessageTimer = sConfigMgr->GetOption<uint32>("Enchanter.MessageTimer", 60000);
            EnchanterEmoteSpell = sConfigMgr->GetOption<uint32>("Enchanter.EmoteSpell", 44940);
            EnchanterEmoteCommand = sConfigMgr->GetOption<uint32>("Enchanter.EmoteCommand", 3);

            // Enforce Min/Max Time
            if (EnchanterMessageTimer != 0)
                if (EnchanterMessageTimer < 60000 || EnchanterMessageTimer > 300000)
                    EnchanterMessageTimer = 60000;
        }
    }
};

class EnchanterAnnounce : public PlayerScript
{

public:

    EnchanterAnnounce() : PlayerScript("EnchanterAnnounce", {
        PLAYERHOOK_ON_LOGIN
    }) {}

    void OnPlayerLogin(Player* player)
    {
        // Announce Module
        if (EnchanterAnnounceModule)
            ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00EnchanterNPC |rmodule.");
    }
};


class npc_enchantment : public CreatureScript
{
public:

    npc_enchantment() : CreatureScript("npc_enchantment") { }

    static std::string PickPhrase()
    {
        std::string phrase = "";
        uint32 PhraseNum = urand(1, EnchanterNumPhrases);
        phrase = "EC.P" + std::to_string(PhraseNum);

        if (phrase == "")
            phrase = "ERROR! NPC Emote Text Not Found! Check the npc_enchanter.conf!";

        std::string randMsg = sConfigMgr->GetOption<std::string>(phrase.c_str(), "");
        return randMsg.c_str();
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (!EnchanterEnableModule)
            return false;

        AddMainMenu(player);
        player->PlayerTalkClass->SendGossipMenu(601015, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (!EnchanterEnableModule)
            return false;

        player->PlayerTalkClass->ClearMenus();

        if (action == CAT_BACK)
        {
            AddMainMenu(player);
            player->PlayerTalkClass->SendGossipMenu(601015, creature->GetGUID());
            return true;
        }

        if (EnchantCategoryInfo const* category = GetCategory(action))
        {
            ShowEnchantMenu(player, creature, category);
            return true;
        }

        if (EnchantOption const* option = GetEnchantOption(action))
        {
            ApplyEnchant(player, creature, option);
            return true;
        }

        player->PlayerTalkClass->SendCloseGossip();
        return true;
    }

    static EnchantCategoryInfo const* GetCategory(uint32 categoryId)
    {
        for (EnchantCategoryInfo const& category : EnchantCategories)
            if (category.Id == categoryId)
                return &category;

        return nullptr;
    }

    static EnchantOption const* GetEnchantOption(uint32 action)
    {
        for (EnchantOption const& option : EnchantOptions)
            if (option.Action == action)
                return &option;

        return nullptr;
    }

    static bool HasRequiredSkill(Player* player, uint32 skill, uint32 rank)
    {
        return !skill || (player->HasSkill(skill) && player->GetSkillValue(skill) >= rank);
    }

    static bool CanSeeCategory(Player* player, uint32 category)
    {
        if (category == CAT_OFFHAND_WEAPON && !player->HasSpell(674))
            return false;

        if (category == CAT_RINGS && !HasRequiredSkill(player, SKILL_ENCHANTING, 450))
            return false;

        for (EnchantOption const& option : EnchantOptions)
            if (option.Category == category && HasRequiredSkill(player, option.RequiredSkill, option.RequiredSkillRank))
                return true;

        return false;
    }

    static void AddMainMenu(Player* player)
    {
        for (EnchantCategoryInfo const& category : EnchantCategories)
        {
            if (!CanSeeCategory(player, category.Id))
                continue;

            std::string label = "|TInterface/ICONS/";
            label += category.Icon;
            label += ":24:24:-18|t[";
            label += category.Title;
            label += "]";
            AddGossipItemFor(player, 1, label, GOSSIP_SENDER_MAIN, category.Id);
        }
    }

    static void ShowEnchantMenu(Player* player, Creature* creature, EnchantCategoryInfo const* category)
    {
        bool added = false;
        for (EnchantOption const& option : EnchantOptions)
        {
            if (option.Category != category->Id || !HasRequiredSkill(player, option.RequiredSkill, option.RequiredSkillRank))
                continue;

            AddGossipItemFor(player, 1, option.Label, GOSSIP_SENDER_MAIN, option.Action);
            added = true;
        }

        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Back", GOSSIP_SENDER_MAIN, CAT_BACK);
        player->PlayerTalkClass->SendGossipMenu(added ? category->MenuId : 601015, creature->GetGUID());
    }

    static bool IsWeapon(Item* item)
    {
        if (!item)
            return false;

        switch (item->GetTemplate()->InventoryType)
        {
            case INVTYPE_WEAPON:
            case INVTYPE_2HWEAPON:
            case INVTYPE_WEAPONMAINHAND:
            case INVTYPE_WEAPONOFFHAND:
                return true;
            default:
                return false;
        }
    }

    static bool ValidateItem(Player* player, Creature* creature, EnchantOption const* option, Item* item)
    {
        if (!item)
        {
            creature->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
            creature->Whisper("Please equip the item you would like to enchant!", LANG_UNIVERSAL, player);
            return false;
        }

        if (option->RequiredItemLevel && item->GetTemplate()->ItemLevel < option->RequiredItemLevel)
        {
            creature->Whisper("That item is not high enough level for this enchantment.", LANG_UNIVERSAL, player);
            return false;
        }

        switch (option->Category)
        {
            case CAT_MAIN_WEAPON:
                if (!IsWeapon(item))
                {
                    creature->Whisper("This enchant requires a weapon to be equipped.", LANG_UNIVERSAL, player);
                    return false;
                }
                break;
            case CAT_OFFHAND_WEAPON:
                if (item->GetTemplate()->InventoryType != INVTYPE_WEAPON && item->GetTemplate()->InventoryType != INVTYPE_WEAPONOFFHAND)
                {
                    creature->Whisper("This enchant requires a one-handed weapon to be equipped in your offhand.", LANG_UNIVERSAL, player);
                    return false;
                }
                break;
            case CAT_2H_WEAPON:
                if (item->GetTemplate()->InventoryType != INVTYPE_2HWEAPON)
                {
                    creature->Whisper("This enchant requires a 2H weapon to be equipped.", LANG_UNIVERSAL, player);
                    return false;
                }
                break;
            case CAT_STAFF:
                if (item->GetTemplate()->InventoryType != INVTYPE_2HWEAPON || item->GetTemplate()->Class != ITEM_CLASS_WEAPON || item->GetTemplate()->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
                {
                    creature->Whisper("This enchant requires a staff to be equipped.", LANG_UNIVERSAL, player);
                    return false;
                }
                break;
            case CAT_SHIELD:
                if (item->GetTemplate()->InventoryType != INVTYPE_SHIELD)
                {
                    creature->Whisper("This enchant requires a shield to be equipped.", LANG_UNIVERSAL, player);
                    return false;
                }
                break;
            default:
                break;
        }

        return true;
    }

    static void ApplyEnchant(Player* player, Creature* creature, EnchantOption const* option)
    {
        if (!HasRequiredSkill(player, option->RequiredSkill, option->RequiredSkillRank))
        {
            creature->Whisper("You do not have the profession skill required for this enchantment.", LANG_UNIVERSAL, player);
            player->PlayerTalkClass->SendCloseGossip();
            return;
        }

        if (option->Category == CAT_RINGS)
        {
            Enchant(player, creature, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER1), option->EnchantId, option);
            Enchant(player, creature, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER2), option->EnchantId, option);
            player->PlayerTalkClass->SendCloseGossip();
            return;
        }

        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, option->Slot);
        if (!ValidateItem(player, creature, option, item))
        {
            player->PlayerTalkClass->SendCloseGossip();
            return;
        }

        Enchant(player, creature, item, option->EnchantId, option);
        player->PlayerTalkClass->SendCloseGossip();
    }

    static void Enchant(Player* player, Creature* creature, Item* item, uint32 enchantid, EnchantOption const* option)
    {
        if (!ValidateItem(player, creature, option, item))
            return;

        if (!enchantid)
        {
            ChatHandler(player->GetSession()).SendNotification("Something went wrong in the code. It has been logged for developers and will be looked into, sorry for the inconvenience.");
            creature->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
            return;
        }

        roll = urand(1, 100);

        item->ClearEnchantment(PERM_ENCHANTMENT_SLOT);
        item->SetEnchantment(PERM_ENCHANTMENT_SLOT, enchantid, 0, 0);

        if (roll > 0 && roll < 33)
            ChatHandler(player->GetSession()).SendNotification("|cff00ff00Beauregard's bony finger crackles with energy when he touches |cffDA70D6{}|cff00ff00!", item->GetTemplate()->Name1);
        else if (roll > 33 && roll < 75)
            ChatHandler(player->GetSession()).SendNotification("|cff00ff00Beauregard holds |cffDA70D6{} |cff00ff00up in the air and utters a strange incantation!", item->GetTemplate()->Name1);
        else
            ChatHandler(player->GetSession()).SendNotification("|cff00ff00Beauregard concentrates deeply while waving his wand over |cffDA70D6{}|cff00ff00!", item->GetTemplate()->Name1);

        creature->CastSpell(player, 12512);
    }

    struct NPC_PassiveAI : public ScriptedAI
    {
        NPC_PassiveAI(Creature * creature) : ScriptedAI(creature) { }

        uint32 MessageTimer = 0;

        void Reset() override
        {
            if (EnchanterMessageTimer != 0)
                MessageTimer = urand(EnchanterMessageTimer, 300000);
        }

        void UpdateAI(const uint32 diff) override
        {
            if (EnchanterEnableModule && EnchanterMessageTimer != 0)
            {
                if (MessageTimer <= diff)
                {
                    if (EnchanterNumPhrases > 0)
                    {
                        std::string Message = PickPhrase();
                        me->Say(Message.c_str(), LANG_UNIVERSAL);
                    }

                    if (EnchanterEmoteCommand != 0)
                        me->HandleEmoteCommand(EnchanterEmoteCommand);

                    if (EnchanterEmoteSpell != 0)
                        me->CastSpell(me, EnchanterEmoteSpell);

                    MessageTimer = urand(EnchanterMessageTimer, 300000);
                }
                else { MessageTimer -= diff; }
            }
        }
    };

    CreatureAI * GetAI(Creature * creature) const override
    {
        return new NPC_PassiveAI(creature);
    }
};

void AddNPCEnchanterScripts()
{
    new EnchanterConfig();
    new EnchanterAnnounce();
    new npc_enchantment();
}
