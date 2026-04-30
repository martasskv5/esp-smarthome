#!/usr/bin/env python3
import RPi.GPIO as GPIO
import time
import paho.mqtt.client as mqtt
import json

ROW_PINS = [21, 20, 16, 12]
COL_PINS = [26, 19, 13, 6]

KEYPAD = [
    ['1', '2', '3', 'A'],
    ['4', '5', '6', 'B'],
    ['7', '8', '9', 'C'],
    ['*', '0', '#', 'D'],
]

MQTT_BROKER = "10.195.236.205"
MQTT_PORT = 1883
LOCK_STATE_TOPIC = "stat/door/LOCK"
LOCK_CODE = "1234"
buffer = []

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
    
def on_connect(client, userdata, flags, reason_code, properties):
    print(f"Connected to MQTT broker with code: {reason_code}")
    
# MQTT Client Setup
client = mqtt.Client(callback_api_version=mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.connect(MQTT_BROKER, MQTT_PORT, 60)
client.loop_start()

# ============ KEYPAD SCANNING ============
def get_key():
    """Scan keypad and return pressed key (with debounce)"""
    key = None
    
    for row_num, row_pin in enumerate(ROW_PINS):
        # Set current row LOW, others HIGH
        for r in ROW_PINS:
            GPIO.output(r, GPIO.HIGH)
        GPIO.output(row_pin, GPIO.LOW)

        # Small delay for signal to settle
        time.sleep(0.001)

        # Check columns
        for col_num, col_pin in enumerate(COL_PINS):
            if GPIO.input(col_pin) == GPIO.LOW:
                key = KEYPAD[row_num][col_num]
                # Debounce/wait for release
                while GPIO.input(col_pin) == GPIO.LOW:
                    time.sleep(0.05)
                break

        # Reset row
        GPIO.output(row_pin, GPIO.HIGH)
        if key is not None:
            break

    # Keep rows in idle state between scans.
    for r in ROW_PINS:
        GPIO.output(r, GPIO.HIGH)
    
    return key

# ============ MAIN LOOP ============
print("Keypad MQTT bridge started. Press Ctrl+C to exit.")
print("Enter the lock code on the keypad, then press # to unlock.")

try:
    while True:
        pressed = get_key()
        
        if pressed is not None:
            if pressed == '#':
                command = ''.join(str(k) for k in buffer)
                if command == LOCK_CODE:
                    client.publish(LOCK_STATE_TOPIC, "0", qos=1)
                    print(f"Lock code entered: {command}. Published 0 to {LOCK_STATE_TOPIC}")
                elif command:
                    print(f"Invalid lock code: {command}")
                buffer = []
                    
            elif pressed == '*':
                buffer = []
                print("Buffer cleared")
                
            else:
                buffer.append(pressed)
                print(f"Buffer: {buffer}")
                
            time.sleep(0.2)  # Debounce between presses
            
        time.sleep(0.05)

except KeyboardInterrupt:
    print("\nShutting down...")
    
finally:
    client.loop_stop()
    client.disconnect()
    GPIO.cleanup()