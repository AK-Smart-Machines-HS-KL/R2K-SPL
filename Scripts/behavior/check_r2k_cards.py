#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
CARDS = ROOT / "Src/Modules/BehaviorControl/BehaviorControl/Cards/R2K"

pre_re = re.compile(r"bool\s+preconditions\s*\(\)\s*const\s*override")
post_re = re.compile(r"bool\s+postconditions\s*\(\)\s*const\s*override")
requires_re = re.compile(r"REQUIRES\(([^)]+)\)")

issues = []

for path in sorted(CARDS.glob("*.cpp")):
    text = path.read_text(encoding="utf-8")

    if "CARD(" not in text and "TEAM_CARD(" not in text:
        continue

    has_pre = bool(pre_re.search(text))
    has_post = bool(post_re.search(text))

    if has_pre != has_post:
        issues.append((path, "pre/post mismatch", "Missing either preconditions() or postconditions()."))

    if "TODO" in text or "ToDo" in text:
        issues.append((path, "todo-marker", "Contains TODO/ToDo markers to review."))

    # Heuristic: postconditions should usually include !preconditions()
    if has_post and "!preconditions()" not in text:
        issues.append((path, "postcondition-nonstandard", "postconditions() does not include !preconditions()."))

    # Heuristic: REQUIRES symbols should usually appear as the<Representation> in the implementation body.
    for representation in requires_re.findall(text):
        symbol = f"the{representation.strip()}"
        if symbol not in text:
            issues.append((path, "requires-possibly-unused", f"{representation.strip()} declared but '{symbol}' not found."))

if not issues:
    print("R2K card check: OK")
    raise SystemExit(0)

print("R2K card check: issues found")
for p, code, msg in issues:
    rel = p.relative_to(ROOT)
    print(f"- {rel}: [{code}] {msg}")

# Non-zero to make it CI-friendly.
raise SystemExit(1)
