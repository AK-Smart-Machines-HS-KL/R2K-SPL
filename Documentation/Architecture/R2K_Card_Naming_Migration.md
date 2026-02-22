# R2K Card Naming Migration (controlled)

This document tracks the controlled naming migration requested in the implementation plan.

## Canonical naming rule
- Card class names use the `*Card` suffix and are referenced in `gameplayCard.cfg` exactly by class name.
- Runtime activity naming remains represented by `BehaviorStatus::Activity` enums.

## Mapping table (legacy -> canonical)
- `OffenseFastGoalKick` -> `OffenseFastGoalKickCard`
- `OffenseForwardPass` -> `OffenseForwardPassCard`
- `OffenseReceivePass` -> `OffenseReceivePassCard`
- `GoalShot` -> `GoalShotCard`
- `DefenseLongShot` -> `DefenseLongShotCard`
- `GoalieLongShot` -> `GoalieLongShotCard`
- `ClearOwnHalf` -> `ClearOwnHalfCard`
- `ClearOwnHalfGoalie` -> `ClearOwnHalfGoalieCard`

## Validation strategy
- `Scripts/behavior/check_r2k_cards.py` validates card invariants.
- `Scripts/behavior/check_r2k_card_naming.py` validates that gameplay stack names resolve to existing `MAKE_CARD(...)` classes and warns on legacy aliases.

## Current status
- Configured normal-play card names in default scenario resolve to canonical card classes.
- Legacy aliases are kept here for documentation and migration traceability only.
