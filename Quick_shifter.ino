const int switchPin = 12;       // Gear shift sensor (Active LOW with PULLUP)
const int relay1 = A0;          // Fuel injector relay
const int relay2 = A1;          // Ignition coil relay
const int ledEnabled = 3;      // Quick shifter enabled indicator
const int ledDisabled = 4;     // Quick shifter disabled indicator

bool quickShifterEnabled = true;
unsigned long switchOffTime = 0;  // Track when switch turned OFF
unsigned long shiftStartTime = 0; // Track shift start time
bool shifting = false;            // Track if shifting is in progress
bool switchWasLow = false;        // Tracks if switch was LOW before
bool switchWasHigh = true;        // Tracks if switch was HIGH before
bool shiftInProgress = false;     // Indicates if shift delay is active

void setup() {
    pinMode(switchPin, INPUT_PULLUP);
    pinMode(relay1, OUTPUT);
    pinMode(relay2, OUTPUT);
    pinMode(ledEnabled, OUTPUT);
    pinMode(ledDisabled, OUTPUT);

    digitalWrite(ledEnabled, HIGH);  // Quick shifter enabled initially
    digitalWrite(ledDisabled, LOW);

    delay(500); // Allow startup stabilization

    // **Disable quick shifter at startup if switch is OFF**
    if (digitalRead(switchPin) == HIGH) {  
        disableQuickShifter();
    }
}

void loop() {
    if (!quickShifterEnabled) return; // Stop if disabled

    int switchState = digitalRead(switchPin);
    unsigned long currentTime = millis();

    // **Gear switch is LOW → Keep relays ON (No delay)**
    if (switchState == LOW) {
        digitalWrite(relay1, LOW);
        digitalWrite(relay2, LOW);
        shifting = false;
        switchOffTime = 0;  // Reset failsafe timer
        switchWasLow = true;
        switchWasHigh = false;
        shiftInProgress = false; // Reset shift flag
    } 

    // **Gear switch goes from LOW to HIGH (Start shifting process)**
    else if (switchState == HIGH && switchWasLow) {  
        shifting = true;
        switchOffTime = millis();  // Record when switch turned OFF
        switchWasLow = false;
        switchWasHigh = true;

        // **Start shift process (Do NOT block)**
        shiftStartTime = millis();  
        shiftInProgress = true; 

        // **Turn OFF the relays immediately**
        digitalWrite(relay1, HIGH);
        digitalWrite(relay2, HIGH);
    }

    // **Check if shift delay is complete (Non-blocking delay)**
    if (shiftInProgress && (millis() - shiftStartTime >= 100)) {
        // **Turn ON the relays back after delay**
        digitalWrite(relay1, LOW);
        digitalWrite(relay2, LOW);
        shiftInProgress = false; // Reset flag
    }

    // **Failsafe: If switch is OFF for more than 3500ms, disable Quick Shifter**
    if (switchOffTime > 0 && (millis() - switchOffTime >= 3500)) {
        disableQuickShifter();
    }
}

void disableQuickShifter() {
    quickShifterEnabled = false;
    digitalWrite(ledEnabled, LOW);
    digitalWrite(ledDisabled, HIGH);
    digitalWrite(relay1, LOW); // Ensure fuel stays ON (safe mode)
    digitalWrite(relay2, LOW);
}
