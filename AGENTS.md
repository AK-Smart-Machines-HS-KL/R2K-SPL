# Build Notes For Agents

## Scope
- These instructions are for building this repository from the terminal.
- Prefer terminal-driven CMake builds.
- Do not use Visual Studio, Xcode, or solution/project files unless a user explicitly asks for that.
- In this repository, "develop" usually means the build configuration `Develop`, not the Git branch `develop`.

## Working Directory
- Run commands from the repository root, i.e. the directory that contains `Build`, `Config`, `Make`, `Src`, and `Util`.
- Do not assume a fixed absolute checkout path. Use repo-relative paths in commands and instructions.

## Common Build Configurations
- `Debug`
- `Develop`
- `Release`

## Common Targets
- `SimRobot`
- `SimulatedNao`
- `Nao`
- `bush`
- `ALL_BUILD` on multi-config generators such as Visual Studio

## Optional Targets
- `Tests`
- `deploy`
- `Controller`
- `CompiledNN`

## Windows
### Preferred Generate Step
- Regenerate the Windows CMake project with:
  - `cmake -G "Visual Studio 17 2022" -A x64 -S Make\Common -B Build\Windows\CMake`

### Full Build
- Build all Windows targets in `Develop` with:
  - `cmake --build Build\Windows\CMake --config Develop --target ALL_BUILD -- /m`

### Single-Target Build
- Examples:
  - `cmake --build Build\Windows\CMake --config Develop --target SimRobot -- /m`
  - `cmake --build Build\Windows\CMake --config Develop --target Tests -- /m`
  - `cmake --build Build\Windows\CMake --config Develop --target bush -- /m`

### Expected Windows Build Artifacts
- After a successful Windows `Develop` build, these are the main output files that should typically exist:
- `Build\Windows\SimRobot\Develop\SimRobot.exe`
- `Build\Windows\SimRobot\Develop\SimulatedNao.dll`
- `Build\Windows\bush\Develop\bush.exe`
- Optional, only if that target was built:
- `Build\Windows\Tests\Develop\Tests.exe`

### Windows Notes
- `Make\VS2022\generate` also tries to generate Linux Ninja build trees. That is not required for the Windows terminal build.
- On this machine, `ninja` was not in `PATH`, so direct Windows generation via the CMake command above is the safer path.
- `ALL_BUILD` also triggers the `Nao` and `deploy` custom steps defined by CMake. Seeing `ninja: no work to do.` there is acceptable if those sub-builds are already up to date.

## Linux
### Preferred Generate Step
- Generate a single Linux config directly:
  - `cmake -DCMAKE_BUILD_TYPE=Develop -G Ninja -S Make\Linux -B Build\Linux\CMake\Develop`
- For all Linux configs:
  - `cmake -DCMAKE_BUILD_TYPE=Debug -G Ninja -S Make\Linux -B Build\Linux\CMake\Debug`
  - `cmake -DCMAKE_BUILD_TYPE=Develop -G Ninja -S Make\Linux -B Build\Linux\CMake\Develop`
  - `cmake -DCMAKE_BUILD_TYPE=Release -G Ninja -S Make\Linux -B Build\Linux\CMake\Release`

### Alternative Repo Script
- The repo already provides:
  - `bash Make/Linux/generate`
- Note: without `NO_CLION=1`, that script also updates CLion project files.
- If only CMake build trees are wanted, prefer:
  - `bash -lc "cd Make/Linux && NO_CLION=1 ./generate"`

### Full Or Single-Target Build
- Direct CMake examples:
  - `cmake --build Build\Linux\CMake\Develop --target SimRobot --config Develop`
  - `cmake --build Build\Linux\CMake\Develop --target Nao --config Develop`
  - `cmake --build Build\Linux\CMake\Develop --target bush --config Develop`
  - `cmake --build Build\Linux\CMake\Develop --target Tests --config Develop`
- Wrapper script examples:
  - `bash Make/Linux/compile SimRobot Develop`
  - `bash Make/Linux/compile Nao Develop`
  - `bash Make/Linux/compile bush Develop`
  - `bash Make/Linux/compile Tests Develop`

### Typical Linux Outputs
- `Build/Linux/SimRobot/Develop/SimRobot`
- `Build/Linux/Nao/Develop/bhuman`
- `Build/Linux/bush/Develop/bush`

## macOS
### Preferred Generate Step
- Generate the macOS Xcode-backed CMake project with:
  - `bash Make/macOS/generate`
- On Apple Silicon, to generate for Rosetta 2:
  - `bash Make/macOS/generate -r`

### Build From Terminal
- Examples:
  - `cmake --build Build/macOS --config Develop --target SimRobot`
  - `cmake --build Build/macOS --config Develop --target bush`
  - `cmake --build Build/macOS --config Develop --target Tests`
  - `cmake --build Build/macOS --config Develop --target Nao`

### macOS Notes
- The macOS generate script performs additional project preparation beyond a plain CMake call.
- Prefer the repo script over manually re-creating its steps.

## Fast Rebuilds
- If no relevant source or CMake files changed, rerun only the build command for the affected platform and target.
- Rebuilds should be much faster than the first full build because they are incremental.
- Examples:
  - Windows: `cmake --build Build\Windows\CMake --config Develop --target ALL_BUILD -- /m`
  - Linux: `cmake --build Build\Linux\CMake\Develop --target SimRobot --config Develop`
  - macOS: `cmake --build Build/macOS --config Develop --target SimRobot`

## Handling Ambiguous Build Requests
- Requests like `build projekt`, `build it`, or `kannst du das bauen` are underspecified.
- Do not immediately ask broad open-ended questions if a reasonable default exists.
- First infer as much as possible from the current machine, the recent conversation, changed files, and existing build trees.

### Default Assumptions
- Default configuration: `Develop`
- Default build style: incremental rebuild
- Default scope: common targets only
- Default platform: the platform that matches the current environment or the one explicitly discussed most recently

### When To Ask Follow-Up Questions
- Ask only if the build request is still materially ambiguous after checking local context.
- Keep questions short and decision-oriented.
- Prefer asking about:
  - platform
  - configuration
  - target scope
  - whether regeneration is required

### Example Clarifications
- `Soll ich Windows, Linux oder macOS bauen?`
- `Meinst du Develop, Debug oder Release?`
- `Soll ich die normalen Build-Ziele bauen, also z. B. SimRobot, SimulatedNao, Nao und bush, oder zusätzlich auch Tests?`
- `Reicht ein inkrementeller Rebuild oder soll ich vorher neu generieren?`

### If No Clarification Is Needed
- Proceed with the inferred defaults.
- In the status update or final report, state the assumptions explicitly.
- Example:
  - `Ich habe einen inkrementellen Windows-Build in Develop für die Haupttargets ausgeführt.`

## When To Regenerate
- Regenerate if:
  - CMake files changed
  - a target was added or removed
  - include paths changed
  - linked libraries changed
  - the generator changed
  - the build tool reports stale or missing generated project data

## Windows-Specific Fix Already Applied
- `Make\CMake\Tests.cmake` was adjusted so `Tests` also builds on Windows.
- The target now includes:
  - `Util/Buildchain/Windows/include`
  - Windows system libs `winmm` and `ws2_32`
- If `Tests` fails again with `dirent.h` or unresolved Windows symbols like `PlaySoundA`, `timeGetTime`, `gethostbyname`, `gethostname`, or `inet_ntoa`, inspect `Make\CMake\Tests.cmake` first.

## What To Report Back
- State the platform, configuration, and targets that were built.
- State whether the requested build succeeded.
- If it failed, name the failing target and include the first actionable compiler or linker error.
- If only a single target was rebuilt, say that explicitly instead of implying the whole build tree is green.
