# ![logo](https://raw.githubusercontent.com/azerothcore/azerothcore.github.io/master/images/logo-github.png) AzerothCore

## Enchanter NPC

- Latest upstream build status: [![Build Status](https://github.com/azerothcore/mod-npc-enchanter/workflows/core-build/badge.svg?branch=master&event=push)](https://github.com/azerothcore/mod-npc-enchanter)

This module adds Beauregard Boneglitter, an AzerothCore NPC that applies permanent gear enchantments through gossip menus.

_This module was created for [StygianCore](https://rebrand.ly/stygiancoreproject), a World of Warcraft 3.3.5a Solo/LAN repack by StygianTheBest | [GitHub](https://rebrand.ly/stygiangithub) | [Website](https://rebrand.ly/stygianthebest))_

_Ported to AzerothCore by gtao725 (https://github.com/gtao725/)_

### Data

- Type: NPC
- NPC ID: 601015
- NPC name: Beauregard Boneglitter
- Script: npc_enchantment
- Config: Yes
- SQL: Yes

### Features

- Applies permanent enchantments directly to equipped gear.
- Supports main-hand weapons, offhand weapons, 2H weapons, staffs, shields, head, shoulders, cloak, chest, bracers, gloves, legs, boots, and rings.
- Uses data-driven enchant/category tables in `src/npc_enchanter.cpp`, making menu entries easier to maintain.
- Paginates large menus at 8 entries per page with Previous/Next navigation.
- Shows only categories and enchants the player can currently use.
- Requires Dual Wield before showing offhand weapon enchants.
- Requires Enchanting 450 before showing ring enchants.
- Requires the appropriate profession skill for profession-only enchants, such as Leatherworking, Engineering, and Enchanting bonuses.
- Validates equipped item slot, weapon type, staff type, shield type, and required item level before applying an enchant.
- Applies ring enchants to both equipped rings.
- Keeps the NPC phrase, emote, login announcement, and enable/disable configuration options.

### Installation

1. Clone this module into your AzerothCore `modules/` directory.
2. Re-run CMake and rebuild the core.
3. Copy `conf/npc_enchanter.conf.dist` to your server config directory as `npc_enchanter.conf` if your setup does not do this automatically.
4. Import the world SQL if your module setup does not auto-import it.

The module registers its SQL path through `conf/conf.sh.dist`:

```bash
data/sql/db-world/
```

The included SQL creates NPC entry `601015`. Spawn it in-game with:

```text
.npc add 601015
```

### Configuration

Configuration lives in `conf/npc_enchanter.conf.dist`.

```ini
Enchanter.Enable = 1
Enchanter.Announce = 1
Enchanter.MessageTimer = 60000
Enchanter.NumPhrases = 3
EC.P1 = "I can infuse your weapons with magical energy."
EC.P2 = "My enchantments are known across all of Azeroth!"
EC.P3 = "Greetings traveller. Care to see my wares?"
Enchanter.EmoteSpell = 0
Enchanter.EmoteCommand = 3
```

- `Enchanter.Enable`: Enables or disables the module.
- `Enchanter.Announce`: Announces the module when players log in.
- `Enchanter.MessageTimer`: Minimum NPC phrase/emote interval in milliseconds. Use `0` to disable. Non-zero values outside 60000 to 300000 are clamped back to 60000.
- `Enchanter.NumPhrases`: Number of configured `EC.P#` phrases to randomly choose from. Use `0` to disable spoken phrases.
- `EC.P#`: NPC phrase text.
- `Enchanter.EmoteSpell`: Optional spell the NPC casts on itself when speaking. Default is disabled.
- `Enchanter.EmoteCommand`: Optional emote command the NPC performs when speaking. Default is Wave.

### Developer Tooling

`tools/export_enchants.py` can export potential enchant candidates from an AzerothCore world database and optionally resolve permanent enchant IDs from Wowhead WotLK spell pages.

Example:

```bash
AC_DB_PASSWORD='password' uv run --with mysql-connector-python tools/export_enchants.py --resolve-wowhead
```

Supported environment variables:

- `AC_DB_HOST`, default `127.0.0.1`
- `AC_DB_PORT`, default `3306`
- `AC_DB_USER`, default `root`
- `AC_DB_PASSWORD`, required
- `AC_DB_NAME`, default `acore_world`

### Version

- v2026.06.27 - Fork update: refactored enchant menus into data-driven tables, expanded enchant coverage, added paginated gossip menus, added offhand/staff/ring handling, added skill and item-level visibility/validation, fixed formatter-style chat notifications, declared config defaults, corrected config comments/defaults, registered the SQL install path, and added the enchant export helper.
- v2019.04.15 - Ported to AzerothCore by gtao725 (https://github.com/gtao725/)
- v2019.02.21 - Add AI/Phrases/Emotes, Update Menu
- v2018.12.05 - Fix broken menu. Replace 'Enchant Weapon' function. Add creature AI and creature text.
- v2018.12.01 - Update function, Add icons, Fix typos, Add a little personality (Emotes don't always work)
- v2017.08.08 - Release

### Credits

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

### License

This code and content is released under the [GNU AGPL v3](https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3).
