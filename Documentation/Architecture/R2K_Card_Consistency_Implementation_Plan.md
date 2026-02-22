# R2K Card Consistency & Dribbling Implementation Plan

## Scope
This plan addresses four requested tracks in iterative order:
1. Reliable dribbling behavior integration (ball-forward progression).
2. Redundant include / REQUIRES cleanup.
3. Unified card naming strategy.
4. pre/postcondition consistency hardening.

## Phase 1 (implemented in this iteration)
- Introduce `R2KDribbleLogic` decision helper with explicit dribble modes:
  - `advance`, `cautiousAdvance`, `recover`
- Integrate into:
  - `OffenseChaseBallCard`
  - `DefenseChaseBallCard`
- Add `dribble_state_change` structured logging event.
- Remove local redundancies in pass/chase cards (`REQUIRES`, includes, dead helpers).
- Fix critical postcondition inconsistency in `GoalieDefaultCard` (`post = !pre`).

## Phase 2 (next)
- Add scripted linting entrypoint: `Scripts/behavior/check_r2k_cards.py`.
- Systematic include/REQUIRES cleanup for all R2K cards with a per-card matrix:
  - declared vs. referenced symbols
  - remove unused dependencies
- Add CI/focused test compile script to prevent regressions.

## Phase 3 (next)
- Card naming migration policy (non-breaking, config-first):
  - define canonical scheme: `<Domain><Intent><Action>Card`
  - add mapping table from old to new names
  - migrate gameplay cfg stacks in one controlled change

## Phase 4 (next)
- pre/postcondition lint policy:
  - default rule: `postconditions() == !preconditions()`
  - exceptions require comment and rationale
- add static checks for condition pattern drift.

## Validation targets
- Focused behavior unit tests pass.
- No cfg stack regressions (cards still resolvable).
- Structured logs remain low-noise and parseable.
