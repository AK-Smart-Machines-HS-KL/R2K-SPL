#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
CARDS_DIR = ROOT / "Src/Modules/BehaviorControl/BehaviorControl/Cards/R2K"
GAMEPLAY_CFG = ROOT / "Config/Scenarios/Default/BehaviorControl/gameplayCard.cfg"

legacy_aliases = {
    "OffenseFastGoalKick": "OffenseFastGoalKickCard",
    "OffenseForwardPass": "OffenseForwardPassCard",
    "OffenseReceivePass": "OffenseReceivePassCard",
    "GoalShot": "GoalShotCard",
    "DefenseLongShot": "DefenseLongShotCard",
    "GoalieLongShot": "GoalieLongShotCard",
    "ClearOwnHalf": "ClearOwnHalfCard",
    "ClearOwnHalfGoalie": "ClearOwnHalfGoalieCard",
}

make_card_re = re.compile(r"MAKE_CARD\(([^)]+)\)")
word_re = re.compile(r"\b([A-Za-z0-9_]+)\b")

implemented_cards = set()
for path in CARDS_DIR.glob("*.cpp"):
    text = path.read_text(encoding="utf-8")
    for match in make_card_re.findall(text):
        implemented_cards.add(match.strip())

cfg_text = GAMEPLAY_CFG.read_text(encoding="utf-8")
words = set(word_re.findall(cfg_text))

errors = []
warnings = []

for canonical in sorted(set(legacy_aliases.values())):
    if canonical not in implemented_cards:
        errors.append(f"Canonical R2K card missing implementation: {canonical}")

for legacy, canonical in legacy_aliases.items():
    if legacy in words:
        warnings.append(f"Legacy alias '{legacy}' found in cfg. Use '{canonical}'.")

if warnings:
    print("R2K naming check: warnings")
    for warning in warnings:
        print(f"- {warning}")

if errors:
    print("R2K naming check: errors")
    for error in errors:
        print(f"- {error}")
    raise SystemExit(1)

print("R2K naming check: OK")
