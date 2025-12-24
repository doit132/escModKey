# Modifier Key Auto-Fix Tool - User Guide

## What is this tool?

This tool automatically fixes "stuck" modifier keys (Ctrl, Shift, Alt, Win) that remain in a pressed state even after you've physically released them. This commonly happens when using automation software like AutoHotkey or Quicker.

## The Problem

When using keyboard automation or key mapping software, modifier keys can sometimes get "stuck" in the virtual pressed state:

- You run an AutoHotkey script that uses `Alt+X`
- The script finishes, but the system still thinks Alt is pressed
- When you press `X` again, it triggers `Alt+X` instead of just `X`
- You have to manually press the physical Alt key to "unstick" it

## The Solution

This tool monitors both physical and virtual key states in real-time:

1. **Detects** when a modifier key is stuck (virtual pressed, physical released)
2. **Waits** for a threshold period (1000ms by default) to avoid false positives
3. **Auto-fixes** when you press any key, by sending a release event for stuck keys

## Features

- ✅ Real-time monitoring of all modifier keys
- ✅ Distinguishes between left and right keys
- ✅ Smart detection (avoids interfering with key mapping tools)
- ✅ Non-intrusive (only fixes when needed)
- ✅ Statistics tracking
- ✅ Configurable threshold

## Requirements

- Windows OS
- Interception driver installed
- Administrator privileges

## Installation

1. Install the Interception driver
2. Build the project: `xmake`
3. Run with administrator privileges: `.\run.ps1`

## Usage

### Basic Usage

1. Run the program with administrator privileges
2. The program runs in the background, monitoring your keys
3. When a modifier key gets stuck:
   - Wait for it to be marked as [STUCK!] (after 1000ms)
   - Press any key to trigger auto-fix
   - The stuck key will be automatically released

### Understanding the Display

```
Left Ctrl   : Physical[RELEASED] Virtual[PRESSED] <-- MISMATCH (1523ms) [STUCK!]
```

- **Physical**: Actual hardware key state
- **Virtual**: System's view of the key state
- **MISMATCH**: States don't match
- **Duration**: How long the mismatch has lasted
- **[STUCK!]**: Key is considered stuck (duration > threshold)

### Status Messages

- `All keys normal. Monitoring...` - Everything is working correctly
- `Stuck keys detected! Press any key to auto-fix.` - One or more keys are stuck
- `[Auto-Fix Triggered]` - Fix process started
- `[Fixed] Left Ctrl` - Specific key was fixed

## Configuration

Edit `src/main.cpp` to customize:

```cpp
const int MISMATCH_THRESHOLD_MS = 1000;  // Time before considering stuck
const bool SHOW_FIX_MESSAGES = true;     // Show/hide fix messages
```

- **MISMATCH_THRESHOLD_MS**: Increase if you get false positives, decrease for faster fixes
- **SHOW_FIX_MESSAGES**: Set to `false` for silent operation

## How It Works

### Detection Logic

1. **Physical Detection**: Uses Interception driver to monitor actual key presses
2. **Virtual Detection**: Uses Windows API to check system key states
3. **Mismatch Detection**: Compares physical and virtual states
4. **Time Tracking**: Records how long a mismatch has lasted

### Fix Trigger Conditions

The tool only fixes when ALL conditions are met:

1. ✅ A modifier key is stuck (mismatch > threshold)
2. ✅ You press any key (physical key down event)
3. ✅ No other physical keys are pressed (avoids interfering with key mapping)

### Smart Protection

The tool avoids interfering with:

- **Normal key presses**: Short mismatches (< 1000ms) are ignored
- **Key mapping tools**: If you're using Space as Ctrl, it won't interfere
- **Automation scripts**: Only fixes after scripts finish (threshold delay)

## Troubleshooting

### "Failed to create Interception context"

- Ensure Interception driver is installed
- Run with administrator privileges
- Check if another program is using Interception

### Keys still stuck after fix

- Increase the threshold if fixes happen too early
- Check if your automation software is continuously sending keys
- Try manually pressing the stuck key

### False positives (fixing when not needed)

- Increase `MISMATCH_THRESHOLD_MS` to 2000 or higher
- Check if you're holding keys for extended periods

### Fix doesn't trigger

- Make sure the key shows [STUCK!] marker
- Try pressing a non-modifier key (like a letter)
- Check that no other physical keys are pressed

## Statistics

When you exit (press ESC), the tool shows:

- Total number of fixes performed
- Breakdown by key (which keys got stuck most often)

This helps identify problematic scripts or patterns.

## Tips

1. **Run at startup**: Add to Windows startup for continuous protection
2. **Monitor patterns**: Check statistics to identify problematic scripts
3. **Adjust threshold**: Fine-tune based on your usage patterns
4. **Silent mode**: Set `SHOW_FIX_MESSAGES = false` for background operation

## Known Limitations

1. **Requires administrator privileges**: Needed for Interception driver
2. **Windows only**: Uses Windows-specific APIs
3. **Threshold delay**: 1 second delay before fixing (by design)
4. **Remote desktop**: May not work correctly in RDP sessions

## Support

If you encounter issues:

1. Check that Interception driver is properly installed
2. Verify you're running with administrator privileges
3. Try adjusting the threshold value
4. Check the statistics to understand what's being fixed

## License

See project LICENSE file.

