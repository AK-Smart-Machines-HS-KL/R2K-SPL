# R2K Card Naming Migration (Non-Breaking Plan)

## Goal
Unify R2K card names into a stable schema to improve readability, tooling, and long-term portability.

## Canonical Scheme
`<Domain><Intent><Action>Card`

Examples:
- `OffenseBallAdvanceCard`
- `DefenseBallInterceptCard`
- `GoalieGoalLineBlockCard`
- `SystemRelocalizationRecoveryCard`

## Current -> Target Mapping (Phase-wise)
- `OffenseChaseBallCard` -> `OffenseBallAdvanceCard`
- `DefenseChaseBallCard` -> `DefenseBallInterceptCard`
- `OffenseFastGoalKickCard` -> `OffenseFastFinishCard`
- `OffenseForwardPassCard` -> `OffensePassInitiateCard`
- `OffenseReceivePassCard` -> `OffensePassReceiveCard`
- `RelocalizeRecoveryCard` -> `SystemRelocalizationRecoveryCard`
- `GoalShotCard` -> `OffenseGoalShotCard`
- `GoalieDefaultCard` -> `GoalieGoalLineBlockCard`

## Migration Strategy
1. Keep old names in code for now, only add mapping table.
2. Introduce aliases in config stacks (where framework allows).
3. Rename cards incrementally with one behavior group per PR.
4. Remove legacy names after all configs/scripts are switched.

## Safety Rules
- Never mix semantic dimensions in one token (e.g. avoid combining role + implementation detail).
- Keep role-independent utility cards under `System*` domain.
- Any renamed card must keep pre/postconditions and behavior equivalent in the rename PR.
