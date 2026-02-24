# Dribbling-Verbesserungen R2K-SPL
**Datum:** 24. Februar 2026 | **Status:** Implementiert

## Problem
- Roboter kickte Ball nur einmal und blieb stehen
- `isDone()` meldete Skill sofort als fertig
- Strenge Ball-Velocity-Limits (50mm/s) verhinderten Re-Kicks
- `alignPrecisely = true` zwang Roboter anzuhalten

## Lösung: Kontinuierlicher Kick-Lauf-Zyklus
Kick → Lauf → Kick → Lauf (150ms Cooldown, 300mm/s Ball-Limit, 0.25-0.35 Power)

## Code-Änderungen

### 1. Dribble.cpp
`Src/Modules/BehaviorControl/BehaviorControl/Skills/Output/MotionRequest/Dribble.cpp`
```cpp
// NEU: Kontinuierliches Dribbling
bool isDone(const Dribble&) const override { return false; }

// ALT: return theMotionInfo.executedPhase == MotionPhase::walk;
```

### 2. GoToBallAndDribble.cpp
`Src/Modules/BehaviorControl/BehaviorControl/Skills/Ball/GoToBallAndDribble.cpp`
```cpp
DEFINES_PARAMETERS({
    (float)(0.25f) minKickPower,  // Sanft
    (float)(0.35f) maxKickPower,  // Kontrolliert
})

// Adaptive Kick-Power: Ballentfernung 150mm=0.25, 300mm=0.35
const float t = (ballDistance - 150.f) / (300.f - 150.f);
const float adaptiveKickPower = minKickPower + t * (maxKickPower - minKickPower);

// alignPrecisely = false (kein Stoppen)
theDribbleSkill(..., false, finalKickPower, ...);
```

### 3. DribbleEngine.cpp
`Src/Modules/MotionControl/WalkingEngine/DribbleEngine.cpp`
```cpp
// NEU: Gelockerte Checks für kontinuierliches Dribbling
const unsigned timeSinceLastKick = theFrameInfo.time - theMotionInfo.lastKickTimestamp;
const bool ballSeenRecently = ... || timeSinceLastKick > 150; // NEU: 150ms Cooldown
const bool ballNotTooFast = velocity.norm() < 300.f;          // NEU: 300mm/s (war 50mm/s)
```

### 4. DribbleEngine.h
`Src/Modules/MotionControl/WalkingEngine/DribbleEngine.h`
```cpp
REQUIRES(FrameInfo),  // Für Cooldown-Timer
```

### 5. Obstacle Avoidance
- PathPlanner.plan() - Automatisch im FarRange
- LibWalk.calcObstacleAvoidance() - Automatisch im CloseRange
- Nutzen ObstacleModel intern

## Parameter
| Parameter | Wert | Zweck |
|-----------|------|-------|
| minKickPower | 0.25f | Sanfte Kicks |
| maxKickPower | 0.35f | Max Power |
| kickCooldown | 150ms | Kick-Frequenz |
| ballVelocityLimit | 300mm/s | Max Ball-Speed |
| alignPrecisely | false | Kein Stoppen |

## Testing
**Scene:** `Config/Scenes/1vs3DribblingTest.con`
- Robot1 @ x=750mm (Mittelfeldkreis)
- 3 Dummies @ x=-2m

**Kompilieren & Starten:**
```powershell
cd build
msbuild B-Human.sln /p:Configuration=Debug /t:SimulatedNao
.\build\Windows\SimRobot\Debug\SimRobot.exe Config\Scenes\1vs3DribblingTest.con
```

**Aktivieren:** `Behavior` → `OffenseChaseBallCard` → `gc playing`

**Erwartetes Verhalten:**
- Mehrfache Kicks (nicht nur einer)
- Keine Stops zwischen Kicks
- Ball bleibt nah am Roboter
- Automatische Gegner-Ausweichung

## Parameter-Tuning
```cpp
// GoToBallAndDribble.cpp
minKickPower / maxKickPower  // Kick-Stärke anpassen

// DribbleEngine.cpp
timeSinceLastKick > 150      // Cooldown anpassen
< 300.f                       // Velocity-Limit anpassen
```

## Verwendung
- `OffenseChaseBallCard`: `theGoToBallAndDribbleSkill(calcAngleToGoal(), true);`
- `DefenseChaseBallCard`: `theGoToBallAndDribbleSkill(calcAngleToGoal(), true);`

## Ergebnis
- Ball bleibt sehr nah am Roboter (0.25-0.35 Power)
- Fluide Bewegung ohne Stops (alignPrecisely=false) -> wenn tippel problem gelöst, wieder true setzen
- Häufige sanfte Kicks (150ms Cooldown)
- Automatische Gegner-Ausweichung
