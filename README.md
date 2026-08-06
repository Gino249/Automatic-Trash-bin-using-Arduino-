# Automatic-Trash-bin-using-Arduino-

## Wiring Steps

1. **Mount the ultrasonic sensor** on the front rim of the bin, facing outward/downward so it detects a hand or object approaching — not the lid itself.

2. **Wire the HC-SR04 sensor:**
   - VCC → Arduino **5V**
   - GND → Arduino **GND**
   - Trig → Arduino **pin 9**
   - Echo → Arduino **pin 10**

3. **Wire the servo motor:**
   - Red wire (power) → Arduino **5V** (or external 5V supply — see step 5)
   - Brown/Black wire (ground) → Arduino **GND**
   - Orange/Yellow wire (signal) → Arduino **pin 11**

4. **Mount the servo** near the lid hinge, and attach a servo arm/horn linked to the lid so that rotating from 0° to 90° lifts the lid open.

5. **Power considerations:** If using an external battery pack for the servo, make sure to **connect its GND to the Arduino's GND** as well (common ground) — otherwise the servo signal won't work correctly. Powering the servo from Arduino's 5V pin is fine for light testing, but a separate supply is safer to avoid brownouts that can reset the board.

6. **Connect Arduino to your computer** via USB and upload the sketch using the Arduino IDE (Tools → Board → select your Arduino model; Tools → Port → select the correct port).

7. **Test it:** Open the Serial Monitor (9600 baud) to watch live distance readings. Wave your hand within ~20 cm of the sensor — the servo should rotate to 90° (lid open), then return to 0° (lid closed) when nothing is detected.

8. **Adjust as needed:**
   - Change `MAX_DISTANCE` in the code if 20 cm is too close/far for your bin.
   - Adjust `servo.write(90)` if 90° doesn't fully open your specific lid mechanism.

 
