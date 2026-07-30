# RoboArm — 2-Axis Robotic Arm with Servo-Controlled Claw
 
A joystick-controlled robotic arm built on an Arduino microcontroller. Two analog joysticks drive smooth, velocity-based movement across two axes, while a third analog input controls a servo-driven claw. The mechanical base and structure were designed in CAD.
 
<p align="center">
  <img src="resources/roboarm.gif" width="45%" alt="RoboArm Demo" /> 
  <img src="resources/claw.gif" width="20%" alt="Claw Demo" />
</p>
## Overview
 
Most beginner joystick-servo projects map joystick position directly to servo angle. RoboArm instead uses **velocity-based control**: joystick deflection sets how *fast* the arm moves, not where it points. Combined with dead-zone filtering and a rolling average filter on the analog inputs, this produces smoother, drift-free motion instead of jittery, twitchy movement.
 
## Features
 
- **Velocity-based joystick control** — joystick deflection from center maps to servo *speed*, not absolute position, giving finer, more natural control over arm movement
- **Dead-zone filtering** — a threshold band around the joystick's resting value (~510–512) prevents drift from analog noise when the joystick is idle
- **Rolling average (circular buffer) smoothing** — a 4-sample circular buffer with a running total filters noisy analog input before it reaches the control loop, without recomputing the sum on every read
- **Non-blocking task scheduling** — three independent timing loops (analog sampling, servo updates, debug logging) run off `micros()` comparisons instead of `delay()`, so the loop never blocks
- **Direct proportional claw control** — a third servo (claw) is driven by direct 1:1 mapping from a potentiometer/joystick axis for immediate, responsive open/close control
- **CAD-designed base and structure**
## Hardware
 
| Component | Pin | Notes |
|---|---|---|
| X-axis servo SG90 | D2 | 800–2000µs pulse range |
| Y-axis servo SG90 | D3 | 800–2000µs pulse range |
| Claw servo | D6 | Direct proportional mapping |
| X-axis joystick | A4 | |
| Y-axis joystick | A5 | |
| Claw input (pot/joystick axis) | A3 | |
 
- Arduino Uno/Nano (or compatible)
- 2x standard hobby servos (arm axes)
- 1x standard hobby servo (claw)
- 2-axis analog joystick module (or 2x potentiometers)
- 1x potentiometer or third joystick axis (claw control)
- CAD-designed base/frame (see `/cad` for design files, if included)
- External 5V power supply recommended for servos under load
## How It Works
 
1. **Sampling**: Every 5ms, the raw analog joystick values for X and Y are read, inverted, and pushed into a 4-sample circular buffer per axis. The buffer maintains a running total so the average is always O(1) to compute.
2. **Control loop**: Every 20ms, the buffered average is compared against a dead-zone band. Outside the dead zone, the distance from the threshold is scaled down into a small speed value, which is added to the servo's current position each cycle — this is what makes it velocity control rather than position control. The result is clamped to the servo's safe pulse range (800–2000µs).
3. **Claw**: Read directly and mapped linearly (0–1023 → 0–180°) each loop iteration — no smoothing, since immediate response is more useful for gripping.
4. **Debug output**: Every 100ms, current averages, servo pulse widths, and speed values are printed over serial for tuning/debugging.
## CAD

The base and arm structure were designed in [Onshape]. Renders below; raw files are informal/unversioned and not included, 
but available on request.
## Possible Improvements
 
- Add smoothing to the claw input to reduce jitter on noisy potentiometers
- Add a soft-start ramp so the arm doesn't jump on power-up
- Externalize tuning constants (thresholds, speed constant) to allow runtime calibration
- Add current-position feedback (potentiometer-based servos) for closed-loop control
## Files
 
- `RoboArm.ino` — main Arduino sketch
## Known Issues / Lessons Learned

- The claw mechanism was never successfully attached to the arm. The servo(s) 
  driving it failed repeatedly under load, consistent with a mismatch between 
  the servo's rated torque and the actual grip force required — likely needing 
  either a higher-torque servo or mechanical gear reduction to compensate.
- Diagnosing this pointed to a clear next step (resize the servo by required 
  torque rather than by what was on hand), but I paused further iteration here 
  to prioritize school, so it remains an open item rather than a solved one.
- The claw's control code (direct proportional mapping) is written and tested 
  in isolation, and the arm/joystick control loop above is fully functional 
  independent of the claw.