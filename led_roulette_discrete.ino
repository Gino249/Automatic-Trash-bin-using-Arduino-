/*
  LED ROULETTE GAME - Arduino Nano (DISCRETE LED VERSION, no NeoPixel)
  ----------------------------------------------------------------------
  - Press button -> ring spins counter-clockwise, slows down, stops on a
    random color (red/green/blue/yellow).
  - The matching discrete indicator LED then blinks to show the result.
  - Press the SAME button again -> resets and spins again.

  This version drives 16 separate single-color LEDs (not addressable RGB)
  using two chained 74HC595 shift registers, so only 3 Nano pins are used
  to control all 16 ring LEDs.

  Hardware:
    D2  - Push button (other leg to GND, uses INPUT_PULLUP)
    D3  - Buzzer
    D9  - 74HC595 DS   (Data)   - shared by both chained chips
    D10 - 74HC595 SH_CP (Clock) - shared by both chained chips
    D11 - 74HC595 ST_CP (Latch) - shared by both chained chips
    D5  - Red indicator LED
    D6  - Green indicator LED
    D7  - Blue indicator LED
    D8  - Yellow indicator LED
    A0  - left unconnected (random seed noise source)

  Ring LED physical color placement (must match your wiring):
    Ring LEDs 0-3   -> Yellow
    Ring LEDs 4-7   -> Blue
    Ring LEDs 8-11  -> Green
    Ring LEDs 12-15 -> Red

  No external library needed - uses built-in shiftOut().
*/

// ---------- CONFIG ----------
#define NUM_LEDS 16 // Ring LED count. Must be a multiple of 4.
                     // 16 = 2 chained 74HC595 chips (8 outputs each).
                     // To go higher (24, 32...), add more 74HC595 chips
                     // chained the same way, and add matching shiftOut()
                     // calls inside setRingLED() / clearRing() below.

#define BUTTON_PIN  2
#define BUZZER_PIN  3
#define DATA_PIN    9   // 74HC595 DS
#define CLOCK_PIN   10  // 74HC595 SH_CP
#define LATCH_PIN   11  // 74HC595 ST_CP

#define RED_LED     5
#define GREEN_LED   6
#define BLUE_LED    7
#define YELLOW_LED  8

// If your physical ring's wiring order runs CLOCKWISE instead of
// counter-clockwise, set this to true to flip the spin direction.
#define REVERSE_DIRECTION false

enum Color { RED, GREEN, BLUE, YELLOW };

bool gameRunning = false;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);

  clearRing();
  clearIndicators();

  // seed randomness from an unconnected analog pin (floating noise)
  randomSeed(analogRead(A0));
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(30); // debounce
    if (digitalRead(BUTTON_PIN) == LOW) {
      while (digitalRead(BUTTON_PIN) == LOW) { /* wait for release */ }
      if (!gameRunning) {
        spinWheel();
      }
    }
  }
}

// ---------- helpers ----------

// Which physical color LED lives at this ring position (quadrant layout)
Color colorAt(int index) {
  int quadrantSize = NUM_LEDS / 4;
  int quadrant = index / quadrantSize;

  switch (quadrant) {
    case 0: return YELLOW;
    case 1: return BLUE;
    case 2: return GREEN;
    case 3: return RED;
  }
  return RED; // fallback, shouldn't be reached
}

int indicatorPin(Color c) {
  switch (c) {
    case RED:    return RED_LED;
    case GREEN:  return GREEN_LED;
    case BLUE:   return BLUE_LED;
    case YELLOW: return YELLOW_LED;
  }
  return -1;
}

// Lights exactly ONE ring LED (by index) and turns all others off.
// Pass -1 to turn the whole ring off.
void setRingLED(int index) {
  uint16_t bits = 0;
  if (index >= 0 && index < NUM_LEDS) {
    bits = (uint16_t)1 << index;
  }

  uint8_t lowByte  = bits & 0xFF;        // Chip A (LEDs 0-7)
  uint8_t highByte = (bits >> 8) & 0xFF; // Chip B (LEDs 8-15)

  digitalWrite(LATCH_PIN, LOW);
  // Send the FARTHEST chip's byte first (Chip B), then the nearest (Chip A),
  // since data shifts further down the chain with each subsequent byte.
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, highByte);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, lowByte);
  digitalWrite(LATCH_PIN, HIGH);
}

void clearRing() {
  setRingLED(-1);
}

void clearIndicators() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  noTone(BUZZER_PIN);
}

// ---------- main game logic ----------

void spinWheel() {
  gameRunning = true;
  clearRing();
  clearIndicators();

  // Reseed with live entropy each spin so results aren't predictable/repeating
  randomSeed(analogRead(A0) + micros());

  int winnerIndex    = random(0, NUM_LEDS);  // truly random stopping LED
  int extraRotations = random(3, 6);          // 3-5 full loops before landing
  int totalSteps      = extraRotations * NUM_LEDS + winnerIndex;

  for (int step = 0; step <= totalSteps; step++) {
    int idx = step % NUM_LEDS;
    if (REVERSE_DIRECTION) idx = (NUM_LEDS - idx) % NUM_LEDS;

    setRingLED(idx);
    tone(BUZZER_PIN, 1000, 15); // roulette "tick" sound

    // Ease-out: fast at first, slows down near the end (cubic curve)
    float progress = (float)step / (float)totalSteps;
    int delayTime = 35 + (int)(pow(progress, 3) * 280); // 35ms -> ~315ms
    delay(delayTime);
  }

  // Land exactly on the winner
  int finalIdx = winnerIndex;
  if (REVERSE_DIRECTION) finalIdx = (NUM_LEDS - winnerIndex) % NUM_LEDS;
  setRingLED(finalIdx);

  Color winner = colorAt(winnerIndex);
  announceResult(winner);

  gameRunning = false; // pressing the button now restarts the game
}

void announceResult(Color winner) {
  // little "win" fanfare
  int melody[] = {1200, 1500, 1800};
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, melody[i], 150);
    delay(180);
  }
  noTone(BUZZER_PIN);

  // blink the matching indicator LED
  int pin = indicatorPin(winner);
  for (int i = 0; i < 6; i++) {
    digitalWrite(pin, HIGH);
    delay(200);
    digitalWrite(pin, LOW);
    delay(200);
  }
  // winning ring LED stays lit as a reminder of the result
  // until the button is pressed again to spin a new round
}
