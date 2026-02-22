# R2K SPL Refactor Blueprint (High-Impact)

## Session Information
- Prepared at (UTC): 2026-02-22T13:16:52Z
- Prepared by: Codex (GPT-5.2-Codex) in collaboration with R2K team
- Input context: 5v5 RoboCup SPL priority on stability (60-70%) over aggression, with planned future migration path to Booster K1 + ROS 2.

## Goals
1. Stabileres Gesamtspiel unter realen Störungen (Licht, Motion Blur, WLAN-Loss).
2. Schnellere und robustere Ballgewinne ohne Entscheidungsflattern.
3. Große, testbare Modularisierung für Zukunfts-Portierung (Booster K1/ROS 2).

## Suggested Target Architecture (SOTA-oriented, pragmatic)

### Layer A: Platform Abstraction Layer (PAL)
- Responsibilities: time, sensors, kinematics I/O, actuator commands, network sockets.
- Rule: no behavior logic in PAL.
- Benefit: clear porting seam from NAO stack to Booster K1/ROS 2 drivers.

### Layer B: Perception & State Estimation
- Perception split into:
  - low-level detectors (ball/line/robot hypotheses)
  - fusion layer (confidence-calibrated object tracks)
- State estimation split into:
  - self-localization
  - team-ball fusion
  - uncertainty manager (single confidence API for behavior)

### Layer C: Decision Layer
- Tactical layer: team mode, role policy, communication policy.
- Skill arbitration layer: shot/pass/dribble/search decisions with hysteresis and confidence gates.
- Rule: all role/striker assignment in reusable, unit-tested logic helpers.

### Layer D: Motion Execution
- Skills map to motion primitives with explicit "readiness" signals (ready to kick, unstable, recovering).
- Dribble/kick controllers expose quality metrics to Decision Layer.

### Layer E: Observability & Evaluation
- Structured event logs (JSON/CSV) for role changes, striker switches, ball-loss events, localization confidence drops.
- Replay-compatible KPI pipeline for automatic regression checks.

## Migration Path to ROS 2 (Booster K1)
1. Keep existing logic modules but route all hardware dependencies through PAL wrappers.
2. Define ROS 2 messages for core representations (ball, pose, obstacles, role state).
3. Build adapter nodes:
   - `r2k_perception_bridge`
   - `r2k_behavior_bridge`
   - `r2k_motion_bridge`
4. Run mixed mode (current runtime + ROS 2 mirror) before full switchover.

## Automatic Analysis / AI-assisted Optimization
- Add parameter sweeps over replay logs (ball gain time, role stability, shot execution success).
- Add search space for:
  - striker hysteresis
  - dribble-to-kick transition thresholds
  - ball confidence decay
- Use objective weighting: stability 0.65, scoring potential 0.35.

## Recommended KPI Set
- Time-to-first-controlled-ball (kickoff and live play)
- Ball-loss rate per 60s
- Captain/role switch frequency per minute
- Localization confidence drop events
- Dribble abort ratio
- Shots attempted vs. shots on target

## Implementation Phases

### Phase 1 (2-3 weeks): Stabilization Core
- Refactor role and striker logic into standalone tested helpers.
- Add hysteresis + confidence gating for ball ownership.
- Add telemetry events for role switches and ball-loss causes.

### Phase 2 (3-5 weeks): Perception/Localization Robustness
- Confidence-aware ball model handoff (own ball vs team ball).
- Location-adaptive parameter presets (light profile detection + fallback).
- Calibration watchdog for orientation drift.

### Phase 3 (4-8 weeks): Platform-ready Modularization
- Extract PAL interfaces.
- Isolate B-Human-specific dependencies from generic decision modules.
- Add ROS 2 bridge prototypes and replay validation.

## Notes on SOTA alignment
- Prioritize uncertainty-aware decisions and hysteresis to prevent oscillations.
- Keep optimization loop data-driven (replay metrics > subjective tuning).
- Preserve deterministic fallback behavior when communication quality degrades.
