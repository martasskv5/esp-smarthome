#!/usr/bin/env python3
import RPi.GPIO as GPIO
import time
import sys

# ============ CONFIGURATION ============

# 4x4 keypad configuration (BCM numbering).
# If you wired from a WiringPi chart, these are the BCM equivalents:
# WPI rows [29, 28, 27, 26] -> BCM [21, 20, 16, 12]
# WPI cols [25, 24, 23, 22] -> BCM [26, 19, 13, 6]
ROW_PINS = [21, 20, 16, 12]
COL_PINS = [26, 19, 13, 6]

KEYPAD = [
    ['1', '2', '3', 'A'],
    ['4', '5', '6', 'B'],
    ['7', '8', '9', 'C'],
    ['*', '0', '#', 'D'],
]

# ============ SETUP ============

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

# Setup rows as outputs, keep them HIGH while idle.
# A row is pulled LOW only during its scan window.
for pin in ROW_PINS:
    GPIO.setup(pin, GPIO.OUT)
    GPIO.output(pin, GPIO.HIGH)

# Setup columns as inputs with pull-up resistors
for pin in COL_PINS:
    GPIO.setup(pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)

print("=" * 50)
print("   MATRIX KEYPAD TEST")
print("=" * 50)
print(f"Row pins (outputs):    {ROW_PINS}")
print(f"Column pins (inputs):  {COL_PINS}")
print(f"Keypad layout:         4x4 (16 keys)")
print("-" * 50)
print("Press keys on the keypad. Press Ctrl+C to exit.")
print("=" * 50)

# ============ KEYPAD SCANNING ============

def scan_keypad():
    """Scan the keypad matrix and return pressed key, or None"""
    for row_idx, row_pin in enumerate(ROW_PINS):
        # Set current row LOW, others HIGH
        for rp in ROW_PINS:
            GPIO.output(rp, GPIO.HIGH)
        GPIO.output(row_pin, GPIO.LOW)
        
        # Small delay for signal to settle
        time.sleep(0.001)
        
        # Check each column
        for col_idx, col_pin in enumerate(COL_PINS):
            if GPIO.input(col_pin) == GPIO.LOW:
                # Key pressed! Wait for release with timeout
                key = KEYPAD[row_idx][col_idx]
                
                # Debounce: wait for key release
                timeout = 0
                while GPIO.input(col_pin) == GPIO.LOW and timeout < 50:
                    time.sleep(0.01)
                    timeout += 1
                
                # Return to idle state after a key event.
                for rp in ROW_PINS:
                    GPIO.output(rp, GPIO.HIGH)
                
                return key
        
        # Reset row before moving to next
        GPIO.output(row_pin, GPIO.HIGH)
    
    # Keep rows in idle state between scans.
    for rp in ROW_PINS:
        GPIO.output(rp, GPIO.HIGH)
    
    return None

# ============ RAW DIAGNOSTIC MODE ============

def diagnostic_mode():
    """Show raw pin states for troubleshooting"""
    print("\n--- DIAGNOSTIC MODE ---")
    print("Press any key to see raw pin states (Ctrl+C to stop)")
    try:
        while True:
            # Scan each row and show column states
            for row_idx, row_pin in enumerate(ROW_PINS):
                for rp in ROW_PINS:
                    GPIO.output(rp, GPIO.HIGH)
                GPIO.output(row_pin, GPIO.LOW)
                time.sleep(0.001)
                
                col_states = []
                for col_pin in COL_PINS:
                    state = GPIO.input(col_pin)
                    col_states.append(f"GPIO{col_pin}={'L' if state == GPIO.LOW else 'H'}")
                
                print(f"Row GPIO{row_pin} LOW -> {', '.join(col_states)}", end="\r")
                GPIO.output(row_pin, GPIO.HIGH)
            
            time.sleep(0.05)
            
    except KeyboardInterrupt:
        print("\nDiagnostic mode ended.")
        for rp in ROW_PINS:
            GPIO.output(rp, GPIO.HIGH)

# ============ MAIN LOOP ============

def main():
    try:
        while True:
            key = scan_keypad()
            if key is not None:
                print(f"  [KEY PRESSED: {key}]")
                time.sleep(0.15)  # Debounce delay between reads
            time.sleep(0.02)
            
    except KeyboardInterrupt:
        print("\n\nTest stopped by user.")
        
    finally:
        GPIO.cleanup()
        print("GPIO cleaned up. Goodbye!")

# Run diagnostic first if requested
if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--diag":
        diagnostic_mode()
    else:
        main()