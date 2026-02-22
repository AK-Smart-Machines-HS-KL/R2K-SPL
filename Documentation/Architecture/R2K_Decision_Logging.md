# R2K Decision Logging Concept (Agent-Ready)

## Goal
Provide a compact, machine-readable event stream for offline coding-agent analysis and behavior optimization.

Design principle: **few but high-value messages**.

## Message Format
All messages are emitted as annotations with a strict prefix:

`R2KLOG|event=<event>|k1=v1|k2=v2|...`

This allows simple regex/CSV parsing from logs.

## Event Catalog (current integration)

### 1) `shot_gate`
When the striker decision gate blocks or downgrades a shot.

Fields:
- `card` (e.g. `OffenseFastGoalKick`)
- `mode` (`hold` or `fallback`)
- `reason` (`ballLost`, `localizationPoor`, `farFromGoal`)
- `player`
- `distGoal` (fallback case)

Optimization value:
- separates shot failures caused by perception vs. localization vs. geometry.

### 2) `ball_loss_transition`
When an offense chase state drops into search due to missing ball.

Fields:
- `card`
- `to`
- `reason`
- `player`

Optimization value:
- directly measures unstable ball handling loops (approach -> lose -> search).

### 3) `relocalize_state`
Entry markers of relocalization recovery FSM states.

Fields:
- `card`
- `state` (`scan`, `turn`)
- `player`

Optimization value:
- quantifies field disorientation and recovery durations.

### 4) `team_mode_change`
Team tactical mode changed.

Fields:
- `from`
- `to`
- `player`
- `ownActive`
- `oppActive`

Optimization value:
- correlates tactical switching with game context and instability.

### 5) `captain_change`
Captain/ball-player reassignment.

Fields:
- `from`
- `to`
- `player`
- `ownDist`

Optimization value:
- captures role churn and potential decision flapping.


### 6) `pass_intent`
When a passer locks a pass target.

Fields:
- `card`
- `passer`
- `target`
- `targetX`
- `targetY`

Optimization value:
- links successful/failed passes to targeting and teammate selection.

### 7) `pass_ack`
When a receiver acknowledges pass context.

Fields:
- `card`
- `receiver`
- `passer`

Optimization value:
- allows measuring pass synchronization and communication lag effects.

### 8) `intercept_decision`
When a defensive interception/dribble-intercept action is selected.

Fields:
- `card`
- `player`
- `action`
- `ballSource`

Optimization value:
- highlights defensive intervention timing and dependence on own/team ball source.


### 9) `dribble_state_change`
Dribble mode transition/selection for a ball-advance card.

Fields:
- `card`
- `player`
- `mode` (`advance`, `cautiousAdvance`, `recover`)
- `reason` (`stable`, `localizationPoor`, `ballUncertain`)

Optimization value:
- separates dribble instability caused by perception, localization, or ball-motion uncertainty.


### 10) `ball_prediction_source`
Marks decisions where forecast/end-position ball information is explicitly preferred.

Fields:
- `card`
- `player`
- `source` (`forecast`)

Optimization value:
- identifies where behavior depends on predicted ball stop/trajectory instead of instantaneous ball position.

## Why this is intentionally small
We avoid broad debug spam and only log transition/decision points that:
1. have tactical consequence,
2. are actionable for tuning,
3. can be statistically aggregated.

## Suggested post-processing pipeline
1. Parse lines containing `R2KLOG|`.
2. Split by `|`, convert key-value pairs.
3. Aggregate by `event` and `reason`.
4. Build KPI series:
   - `shot_gate` fallback/hold rate
   - `ball_loss_transition` per minute
   - `captain_change` frequency
   - `relocalize_state` cycle count and duration proxy
   - `pass_intent` to `pass_ack` coupling rate
   - `intercept_decision` count by ball source
   - `dribble_state_change` mode distribution and abort trend
   - `ball_prediction_source` frequency by card

## Next expansion candidates (if needed)
