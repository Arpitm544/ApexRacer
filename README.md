# Apex Racer 3D — Unreal Engine 5 Car Racing System

A high-performance 3D Car Racing Game implementation built on Unreal Engine 5 with Chaos Vehicle Physics, anti-cheat sequential checkpoint validation, millisecond-accurate lap timing, and real-time HUD telemetry.

---

## Architecture & Project Structure

```
Game2/
├── ApexRacer.uproject             # UE5 project descriptor (ChaosVehicles & EnhancedInput enabled)
├── Config/
│   ├── DefaultEngine.ini         # Physics sub-stepping (6 substeps @ 60Hz), default GameMode & maps
│   ├── DefaultGame.ini           # Project identity & metadata
│   └── DefaultInput.ini          # Fallback action/axis mappings (WASD, Spacebar, Gamepad)
├── Source/
│   ├── ApexRacer.Target.cs
│   ├── ApexRacerEditor.Target.cs
│   └── ApexRacer/
│       ├── ApexRacer.Build.cs    # Dependencies: ChaosVehicles, EnhancedInput, UMG, Slate
│       ├── ApexRacer.h / .cpp    # Primary module & LogApexRacer category
│       ├── Vehicle/
│       │   ├── RacingWheelFront.h / .cpp   # Front steering & suspension configuration
│       │   ├── RacingWheelRear.h / .cpp    # Rear drive axle & handbrake setup
│       │   └── RacingVehiclePawn.h / .cpp  # Wheeled vehicle pawn, chase cam, speed FOV, reset
│       ├── Track/
│       │   ├── CheckpointActor.h / .cpp    # Gate triggers with forward direction validation
│       │   └── TrackSplineActor.h / .cpp   # Track circuit spline & distance calculation
│       ├── Race/
│       │   ├── LapTimingComponent.h / .cpp # Sequential checkpoint logic, sector splits, best lap
│       │   ├── RacingGameState.h / .cpp    # Countdown sequence, track catalog, race states
│       │   ├── RacingGameMode.h / .cpp     # Game rules, player spawning, restart flow
│       │   └── RacingPlayerController.h / .cpp # Input enablement & vehicle recovery
│       └── UI/
│           └── RacingHUD.h / .cpp          # Built-in Canvas HUD (speedometer, tachometer, lap board)
└── Content/
```

---

## Key Features

### 1. Chaos Vehicle Physics & Handling
- **`ARacingVehiclePawn`**:
  - Inherits from `AWheeledVehiclePawn` with `UChaosWheeledVehicleMovementComponent`.
  - Realistic Rear-Wheel Drive (RWD) with configurable torque curves and automatic transmission.
  - Dynamic Chase Camera with speed-based spring arm lag and automatic FOV widening (88° idle up to 108° at high speed).
  - Quick Cockpit/Hood camera toggle.
  - Upright recovery system (`ResetVehicle`) to instantly recover from rollovers.
  - Lateral slip and wheel skid detection for particle smoke and audio triggers.
- **Wheel Configurations (`URacingWheelFront` & `URacingWheelRear`)**:
  - Front axle: 38° max steer angle, responsive suspension damping, non-driven.
  - Rear axle: Wider tire contact patch, driven axle with handbrake lockup.

### 2. Checkpoints & Anti-Shortcut Validation
- **`ACheckpointActor`**:
  - Overlap gate triggers with visual bounds.
  - **Direction Check**: Validates vehicle velocity vector against gate forward vector (`DotProduct > 0.1`) to prevent reverse driving exploits.
  - Index-based progression: Player must hit gates in sequence (`0 -> 1 -> 2 ... -> N -> 0`).
  - Skipping a checkpoint invalidates the lap and warns the driver.

### 3. Millisecond Lap Timing & Sector Splits
- **`ULapTimingComponent`**:
  - Frame-accurate lap timing (`MM:SS.mmm`).
  - Sector splits calculated at each checkpoint with real-time delta vs. best lap time.
  - Tracks current lap, total laps, last lap, and personal best.
  - Flashing **WRONG WAY** state detection when driving against track direction.

### 4. Immediate Out-of-the-Box HUD
- **`ARacingHUD`**:
  - Built-in C++ Canvas drawing provides instant UI without requiring pre-compiled UMG assets:
    - Digital Speedometer (KM/H & MPH)
    - Gear Indicator (`R`, `N`, `1` - `6`)
    - Colored Tachometer / RPM bar with redline warning (> 6500 RPM)
    - Lap Board: Current Lap, Current Time, Best Time, Last Time
    - Animated Pre-race countdown (`3... 2... 1... GO!`)
    - Wrong Way warning banner
    - Post-race results overlay with restart shortcut (`R`)

---

## How to Open & Run

### macOS (Xcode):
1. Right-click `ApexRacer.uproject` and select **Services > Generate Xcode Project** (or run `UnrealVersionSelector`).
2. Open `ApexRacer.xcworkspace` in Xcode.
3. Build the `ApexRacerEditor` scheme for `My Mac`.
4. Launch the project in Unreal Editor 5.

### Windows (Visual Studio 2022):
1. Right-click `ApexRacer.uproject` and select **Generate Visual Studio project files**.
2. Open `ApexRacer.sln` in Visual Studio.
3. Set build configuration to `Development Editor` / `Win64`.
4. Press `F5` to build and launch Unreal Editor.

---

## Setting Up Your First Track in Unreal Editor

1. **Place Checkpoints**:
   - Drag `ACheckpointActor` into your level along the track.
   - For the Start/Finish line, check `bIsFinishLine = true` and set `CheckpointIndex = 0`.
   - Set subsequent checkpoints along the track with indices `1, 2, 3, ... N`.
   - Ensure the green arrow on each checkpoint points in the forward driving direction!
2. **Place Player Start**:
   - Place a standard `PlayerStart` behind the Start/Finish line facing forward.
3. **Hit Play (PIE)**:
   - The countdown (`3, 2, 1, GO!`) begins automatically with movement frozen until `GO!`.
   - Drive with `W / S / A / D` or Gamepad.
   - Press `Spacebar` for Handbrake, `C` to toggle camera, `R` to reset vehicle.
