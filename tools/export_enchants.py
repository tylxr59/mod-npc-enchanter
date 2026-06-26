#!/usr/bin/env python3
"""Export AzerothCore WotLK enchant candidates as JSON.

The AzerothCore world DB stores item templates for enchant scrolls and
consumable kits, but the stock DBC enchant rows may not be loaded into SQL.
This helper therefore reads spell IDs from item_template and can resolve the
permanent enchant IDs from Wowhead WotLK spell pages.

Example:
  AC_DB_PASSWORD='...' uv run --with mysql-connector-python tools/export_enchants.py --resolve-wowhead
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
import time
import urllib.request
from urllib.error import HTTPError, URLError
from dataclasses import asdict, dataclass

import mysql.connector


WOWHEAD_SPELL_URL = "https://www.wowhead.com/wotlk/spell={spell_id}"
ENCHANT_RE = re.compile(r"Effect\s+Enchant Item:\s*([^<(]+).*?\((\d+)\)", re.S)
REQUIRES_ITEM_LEVEL_RE = re.compile(r"Requires a level\s+(\d+)\s+or higher item", re.I)


@dataclass(frozen=True)
class EnchantCandidate:
    source: str
    item_id: int
    item_name: str
    spell_id: int
    category: str
    label: str
    enchant_id: int | None = None
    enchant_name: str | None = None
    effect_text: str | None = None
    required_item_level: int | None = None


def env(name: str, default: str) -> str:
    return os.environ.get(name, default)


def connect():
    password = os.environ.get("AC_DB_PASSWORD")
    if not password:
        raise SystemExit("Set AC_DB_PASSWORD before running this helper.")

    return mysql.connector.connect(
        host=env("AC_DB_HOST", "127.0.0.1"),
        port=int(env("AC_DB_PORT", "3306")),
        user=env("AC_DB_USER", "root"),
        password=password,
        database=env("AC_DB_NAME", "acore_world"),
        connection_timeout=10,
    )


def category_from_name(name: str) -> str | None:
    replacements = {
        "2H Weapon": "2H Weapon",
        "Weapon": "Weapon",
        "Shield": "Shield",
        "Bracer": "Bracers",
        "Bracers": "Bracers",
        "Cloak": "Cloak",
        "Chest": "Chest",
        "Boots": "Boots",
        "Gloves": "Gloves",
        "Ring": "Rings",
        "Staff": "Staff",
        "Fishing Pole": "Fishing Pole",
    }

    for token, category in replacements.items():
        if f"Enchant {token} -" in name:
            return category

    if "Leg Armor" in name or "Spellthread" in name:
        return "Legs"
    if name.startswith("Arcanum of "):
        return "Head"
    if "Inscription of " in name:
        return "Shoulders"
    if name.startswith("Fur Lining - "):
        return "Bracers"
    if name in {"Darkglow Embroidery", "Lightweave Embroidery", "Swordguard Embroidery", "Springy Arachnoweave"}:
        return "Cloak"
    if name in {"Hyperspeed Accelerators", "Hand-Mounted Pyro Rocket"}:
        return "Gloves"
    if name in {"Nitro Boosts", "Reticulated Armor Webbing"}:
        return "Boots"

    return None


def label_from_name(name: str, category: str) -> str:
    prefixes = [
        f"Scroll of Enchant {category} - ",
        "Scroll of Enchant 2H Weapon - ",
        "Scroll of Enchant Weapon - ",
        "Scroll of Enchant Bracer - ",
        "Scroll of Enchant Cloak - ",
        "Scroll of Enchant Chest - ",
        "Scroll of Enchant Boots - ",
        "Scroll of Enchant Gloves - ",
        "Scroll of Enchant Shield - ",
        "Scroll of Enchant Ring - ",
    ]
    for prefix in prefixes:
        if name.startswith(prefix):
            return name[len(prefix) :]
    return name


def query_candidates() -> list[EnchantCandidate]:
    sql = """
        SELECT entry, name, spellid_1
        FROM item_template
        WHERE spellid_1 > 0
          AND name NOT LIKE 'Formula: %%'
          AND name NOT LIKE 'Pattern: %%'
          AND name NOT LIKE 'Plans: %%'
          AND name NOT LIKE 'Schematic: %%'
          AND (
            name LIKE 'Scroll of Enchant %%'
            OR name LIKE '%% Leg Armor'
            OR name LIKE '%% Spellthread'
            OR name LIKE 'Arcanum of %%'
            OR name LIKE '%% Inscription of %%'
            OR name LIKE 'Fur Lining - %%'
            OR name IN (
              'Darkglow Embroidery',
              'Lightweave Embroidery',
              'Swordguard Embroidery',
              'Springy Arachnoweave',
              'Hyperspeed Accelerators',
              'Hand-Mounted Pyro Rocket',
              'Nitro Boosts',
              'Reticulated Armor Webbing'
            )
          )
        ORDER BY name, entry
    """
    conn = connect()
    try:
        cur = conn.cursor(dictionary=True)
        cur.execute(sql)
        rows = cur.fetchall()
    finally:
        conn.close()

    candidates: list[EnchantCandidate] = []
    seen: set[tuple[int, str]] = set()
    for row in rows:
        name = row["name"]
        category = category_from_name(name)
        if not category:
            continue
        key = (row["spellid_1"], category)
        if key in seen:
            continue
        seen.add(key)
        candidates.append(
            EnchantCandidate(
                source="item_template",
                item_id=row["entry"],
                item_name=name,
                spell_id=row["spellid_1"],
                category=category,
                label=label_from_name(name, category),
            )
        )
    return candidates


def resolve_wowhead(spell_id: int, delay: float) -> tuple[int | None, str | None, str | None, int | None]:
    request = urllib.request.Request(
        WOWHEAD_SPELL_URL.format(spell_id=spell_id),
        headers={
            "User-Agent": (
                "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
                "(KHTML, like Gecko) Chrome/126.0 Safari/537.36"
            ),
            "Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
        },
    )
    with urllib.request.urlopen(request, timeout=15) as response:
        html = response.read().decode("utf-8", "replace")
    match = ENCHANT_RE.search(html)
    required_level_match = REQUIRES_ITEM_LEVEL_RE.search(html)
    if delay:
        time.sleep(delay)
    if not match:
        return None, None, None, int(required_level_match.group(1)) if required_level_match else None
    enchant_name = " ".join(match.group(1).split())
    return (
        int(match.group(2)),
        enchant_name,
        enchant_name,
        int(required_level_match.group(1)) if required_level_match else None,
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--resolve-wowhead", action="store_true")
    parser.add_argument("--wowhead-delay", type=float, default=0.1)
    args = parser.parse_args()

    result = []
    for candidate in query_candidates():
        data = asdict(candidate)
        if args.resolve_wowhead:
            try:
                enchant_id, enchant_name, effect_text, required_level = resolve_wowhead(candidate.spell_id, args.wowhead_delay)
            except (HTTPError, URLError, TimeoutError) as exc:
                print(f"warning: could not resolve spell {candidate.spell_id}: {exc}", file=sys.stderr)
                enchant_id, enchant_name, effect_text, required_level = None, None, None, None
            data["enchant_id"] = enchant_id
            data["enchant_name"] = enchant_name
            data["effect_text"] = effect_text
            data["required_item_level"] = required_level
        result.append(data)

    json.dump(result, sys.stdout, indent=2, sort_keys=True)
    sys.stdout.write("\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
