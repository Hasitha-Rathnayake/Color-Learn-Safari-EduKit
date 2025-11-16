// --- Library Imports ---
#include <Wire.h> // Enables I2C communication (used by LCD)
#include <LiquidCrystal_I2C.h> // Library for I2C-based LCD screens
#include <SPI.h> // Enables SPI communication (used by RFID reader)
#include <MFRC522.h> // Library for MFRC522 RFID module
#include <SoftwareSerial.h> // Allows serial communication on other digital pins
#include <DFRobotDFPlayerMini.h> // Library for DFPlayer Mini MP3 module

// --- LCD Setup ---
LiquidCrystal_I2C lcd(0x27, 16, 2); // Creates LCD object with I2C address 0x27 and 16x2 size

// --- DFPlayer Setup ---
SoftwareSerial mySerial(2, 3); // Creates software serial on pins 2 (RX) and 3 (TX)
DFRobotDFPlayerMini player; // Creates DFPlayer object

// --- RFID Setup ---
#define SS_PIN 10 // Slave Select pin for RFID
#define RST_PIN 9 // Reset pin for RFID
MFRC522 rfid(SS_PIN, RST_PIN); // Creates RFID object with defined pins

// --- Pin Definitions ---
const int potPin = A0; // Potentiometer input
const int redPin = A1; // Red LED pin
const int greenPin = A2; // Green LED pin
const int bluePin = A3; // Blue LED pin
const int modeButtonPin = 4; // Mode toggle button
const int songButtonPin = 5; // Alphabet song button

// --- State Variables and flags ---
bool colorMode = true; // Tracks current mode (true = color, false = alphabet)
bool lastButtonState = HIGH; // Stores previous button state for debounce
bool modeSelected = false; // Tracks if mode has been selected
int lastColor = -1; // Tracks last selected color
int initialPotValue = 0; // Stores initial potentiometer value
bool wheelMoved = false; // Tracks if wheel has been moved
bool songButtonPressed = false; // Tracks if song button is pressed
bool songPlaying = false; // Tracks if Alphabet Song is currently playing
bool songLockout = false; // Prevents song trigger during mode switch
unsigned long startupTime = 0; // Tracks when device powered on
unsigned long songLockoutStart = 0; // Records when lockout begins
unsigned long modeSwitchTime = 0; // Tracks time of last mode switch

// --- UID Tracking for Rfid key cards ---
byte lastUID[4] = {0, 0, 0, 0}; // Stores last scanned RFID UID by using array 
                                // As well card eke uid wala 16 base digits recognize krnw
// --- UID Comparison Function ---
bool isNewUID(byte *uid) {
  // Compares current UID with last scanned UID
  // Returns true if any byte is different (as an ex, it's a new card)
  for (int i = 0; i < 4; i++) 
    if (uid[i] != lastUID[i]) 
      return true;
  return false; // If all bytes match, it's the same card
}

// --- UID Copy Function ---
void copyUID(byte *uid) {
  // Copies current UID into lastUID[] for future comparison
  for (int i = 0; i < 4; i++) 
    lastUID[i] = uid[i];
}

// --- UID to Letter Mapping ---
String getLetterFromUID(byte *uid, byte size) {
  // Matches specific UID byte patterns to letters A–F
  //this isnt a array this is a hardcoded pattern matching by using if statements
  if (uid[0]==0xA3 && uid[1]==0x92 && uid[2]==0xDF && uid[3]==0x95) return "A";
  if (uid[0]==0xE6 && uid[1]==0xBF && uid[2]==0x26 && uid[3]==0x96) return "B";
  if (uid[0]==0x63 && uid[1]==0xD2 && uid[2]==0x78 && uid[3]==0x96) return "C";
  if (uid[0]==0x13 && uid[1]==0xE6 && uid[2]==0xE3 && uid[3]==0x95) return "D";
  if (uid[0]==0x43 && uid[1]==0x7A && uid[2]==0x57 && uid[3]==0xA9) return "E";
  if (uid[0]==0xA3 && uid[1]==0x17 && uid[2]==0xA4 && uid[3]==0x95) return "F";
  return ""; // If no match, return empty string
}

// --- Letter to Phrase Mapping ---
String getPhraseForLetter(String letter) {
  // Returns phrase based on letter
  // In here we used Hardcoded conditional mapping except using arrays we can use arrays as well             
  if (letter=="A") return "A for Apple";
  if (letter=="B") return "B for Bat";
  if (letter=="C") return "C for Cat";
  if (letter=="D") return "D for Dog";
  if (letter=="E") return "E for Egg";
  if (letter=="F") return "F for Fish";
  return "Unknown"; // If letter is not recognized
}

// --- LED Fade-Out Function ---
// In here we used For Loop with PWM Dimming method
// The praocess is Iterative analog control using PWM (Pulse Width Modulation)
//| Ideal For | Mode switching, power-down effects, calming transitions |
void fadeOutLED() {
  // Gradually dims all RGB LEDs from full brightness to off
  for (int i = 255; i >= 0; i -= 5) {
    analogWrite(redPin, i);    // Reduce red intensity
    analogWrite(greenPin, i);  // Reduce green intensity
    analogWrite(bluePin, i);   // Reduce blue intensity
    delay(10);                 // Short delay for smooth fading
  }
}

// --- Setup Function ---
void setup() {
  delay(1000); // Wait 1 second before initializing everything

  Serial.begin(9600);           // Start serial monitor for debugging messages
  mySerial.begin(9600);         // Start software serial for DFPlayer communication

  // --- Set RGB LED pins as output ---
  pinMode(redPin, OUTPUT);      // Set red LED pin as output
  pinMode(greenPin, OUTPUT);    // Set green LED pin as output
  pinMode(bluePin, OUTPUT);     // Set blue LED pin as output

  // --- Set button pins as input with internal pull-up resistors ---
  pinMode(modeButtonPin, INPUT_PULLUP);  // Mode selection button (active LOW)
  pinMode(songButtonPin, INPUT_PULLUP);  // Alphabet song button (active LOW)

  lcd.init();                   // Initialize the I2C LCD display
  lcd.backlight();              // Turn on LCD backlight

  // --- Initialize DFPlayer Mini ---
  if (!player.begin(mySerial)) {         // Try to start DFPlayer using software serial
    lcd.setCursor(0, 0);                 // If it fails, show error on LCD
    lcd.print("DFPlayer error");         // Display error message
    while (true);                        // Halt the program (infinite loop)
  }

  player.volume(15);           // Set DFPlayer volume level (range: 0–25)
  delay(100);                  // Short delay to allow DFPlayer to stabilize
  player.stop();               // Stop any auto-play that might have started

  SPI.begin();                 // Start SPI communication for RFID module
  rfid.PCD_Init();             // Initialize RFID reader

  initialPotValue = analogRead(potPin);  // Assign the initialPotValue variable & Read and store initial potentiometer value

  // --- Guided Startup Sequence ---
  lcd.clear();                               // Clear LCD screen
  lcd.setCursor(0, 0); lcd.print("Hello Welcome"); // Display welcome message
  player.play(1);                            // Play welcome voice: "Hello there, welcome to Color and Learn Safari"
  delay(5000);                               // Wait for audio to finish

  lcd.clear();                               // Clear LCD again
  lcd.setCursor(0, 0); lcd.print("Press mode button"); // Prompt user to press mode button
  player.play(2);                            // Play instruction voice: "To select mode, press the mode button"
  delay(5000);                               // Wait for audio to finish

  startupTime = millis();                    // Record the time when setup finishes
  //millis() is a built-in Arduino function
  //It returns the number of milliseconds since the Arduino board was powered on or reset
  //So if your board has been running for 5 seconds, millis() will return 5000
  //This is like a stopwatch that starts at zero when the device boots
}

void loop() {

  // --- Wait for Initial Mode Selection ---
  // Until modeSelected is true, wait for mode button press
  if (!modeSelected) {
    bool buttonState = digitalRead(modeButtonPin); // Read mode button state
    if (buttonState == LOW && lastButtonState == HIGH) { // Detect button press (falling edge)
      modeSelected = true;           // Mark that mode has been selected
      colorMode = true;              // Start in Color Mode
      songButtonPressed = false;     // Reset song button state
      delay(300);                    // Debounce delay
      modeSwitchTime = millis();     // Record time of mode switch/Time Tracking with millis
      
      // Display and play Color Mode welcome
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Color Mode Activated");
      player.play(3);                // Play "Welcome to Color Mode"
      delay(2000);

      // Display and play wheel/knob control instruction
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Turn the wheel");
      player.play(4);                // Play "Turn the wheel slowly to start"
      delay(3000);
      player.pause();                // Stop DFPlayer to prevent auto-trigger
    }
    lastButtonState = buttonState;   // Update button state for next loop
    return;                          // Skip rest of loop until mode is selected
  }

  // --- Mode Toggle Logic ---
  // Detects mode button press to switch between Color and Alphabet modes
  bool buttonState = digitalRead(modeButtonPin); // Read mode button again
  if (buttonState == LOW && lastButtonState == HIGH) { // Detect press
    modeSwitchTime = millis();     // Reset cooldown timer
    colorMode = !colorMode;        // Toggle between Color and Alphabet Mode
    lcd.clear();

    if (colorMode) {
      // Enter Color Mode
      lcd.setCursor(0, 0); lcd.print("Color Mode Activated");
      player.play(3);              // Play welcome voice
      delay(2000);

      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Turn the wheel");
      player.play(4);              // Play wheel instruction audio to start
      wheelMoved = false;          // Reset wheel movement tracking
      delay(3000);
    } else {
      // Enter Alphabet Mode
      songLockout = true;          // Temporarily block song button
      songLockoutStart = millis(); // Record lockout start time
      fadeOutLED();                // Fade out RGB LED
      lcd.setCursor(0, 0); lcd.print("Alphabet Mode Activated");//display the mode we selected
      player.play(5);              // Play Alphabet Mode welcome audio part
      delay(5000);

      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Scan a card");//display the instruction
      player.play(6);              // Play audio to scan RFID card
      delay(2000);

      // Reinitialize RFID reader/manual reinitialization of the RFID reader, and it uses a combination of SPI bus control and library-level reset methods
      SPI.end(); SPI.begin(); rfid.PCD_Init(); rfid.PICC_HaltA();
      /*SPI.end();               // Stop SPI communication
        SPI.begin();             // Restart SPI communication
        rfid.PCD_Init();         // Reinitialize the RFID reader (MFRC522)
        rfid.PICC_HaltA();       // Halt any active card communication*/
    }

    lastColor = -1;                // Reset last color index
    delay(300);                    // Debounce delay
  }
  lastButtonState = buttonState;   // Update button state

  // --- Color Mode Logic ---
  if (modeSelected && colorMode && millis() - modeSwitchTime > 100) {
    int potValue = analogRead(potPin); // Read potentiometer value
    Serial.println("🎨 Potentiometer logic active");//Serial debugging to check potentiometer behaviour

    // Detect if wheel has moved significantly/Threshold detection using absolute difference
    //Purpose: Detect if the wheel has moved significantly from its initial position
    //Prevents false triggers from small noise
    if (!wheelMoved && abs(potValue - initialPotValue) > 20) {
      wheelMoved = true;
    }

    if (wheelMoved) {
      // Determine color based on pot value
      //Variable initialization:Purpose: Prepare variables to store color data and audio index
      int colorIndex = -1;
      int R=0, G=0, B=0;
      String colorName = "";

      // Range-based mapping:Purpose: Map potentiometer value to a specific color
      if (potValue < 170) { R=255; G=0; B=0; colorIndex=7; colorName="Red"; }
      else if (potValue < 340) { R=0; G=255; B=0; colorIndex=8; colorName="Green"; }
      else if (potValue < 510) { R=0; G=0; B=255; colorIndex=9; colorName="Blue"; }
      else if (potValue < 680) { R=255; G=255; B=0; colorIndex=10; colorName="Yellow"; }
      else if (potValue < 850) { R=0; G=255; B=255; colorIndex=11; colorName="Cyan"; }
      else { R=255; G=0; B=255; colorIndex=12; colorName="Magenta"; }

      // Set RGB LED to selected color using analog brightness values
      analogWrite(redPin, R);
      analogWrite(greenPin, G);
      analogWrite(bluePin, B);

      // If color changed, update LCD and play voice
      if (colorIndex != lastColor) {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("You chose:");
        lcd.setCursor(0, 1); lcd.print(colorName);//LCD display update
        player.play(colorIndex); // Play color name audio for the selected color
        lastColor = colorIndex;//State tracking for store the last color index to detect future changes
        delay(1000);//Blocking delay to avoid rapid re triggering
      }
    }
  }

  // --- Alphabet Mode Logic ---
  else {
    // Check for new RFID card
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())//Rfid card detection
     {
      if (isNewUID(rfid.uid.uidByte))//UID comparison to Check if the scanned UID is different from the last one
       {
        copyUID(rfid.uid.uidByte); // Save UID for comparison
                                   //This is UID storage method
        // Map UID to letter mapping
        String letter = getLetterFromUID(rfid.uid.uidByte, rfid.uid.size);
        //Letter to phrase mapping:Purpose: Converts the letter into a phrase like “A for Apple”
        String phrase = getPhraseForLetter(letter);

        // Display letter and phrase on LCD
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("Letter "); lcd.print(letter);
        lcd.setCursor(0, 1); lcd.print(phrase);

        // Play corresponding audio file
        mySerial.listen(); // Ensure DFPlayer is listening
        int audioFile = 13 + (letter[0] - 'A'); 
        // Calculates which MP3 file to play based on the letter by using ASCII-based file indexing
        player.play(audioFile); // Play letter audio

        Serial.print("Playing file: ");//Serial debug output
        Serial.println(audioFile);

        //UID logging: Print UID to Serial Monitor in HEX format
        Serial.print("✅ UID: ");
        for (byte i = 0; i < rfid.uid.size; i++) {
          Serial.print(rfid.uid.uidByte[i], HEX);
          Serial.print(" ");
        }
        Serial.println();

        delay(1500); // Wait before next scan
      }
      // RFID protocol cleanup
      // Halt RFID communication with the card and stops encryption
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
  }

  // --- D5 Button: Toggle Alphabet Song ---
  bool songButtonState = digitalRead(songButtonPin); // Read the current song button state

  // Multi-condition gating to Check all conditions before allowing song playback
  if (modeSelected &&
      !songLockout &&
      millis() - startupTime > 3000 &&
      millis() - modeSwitchTime > 1000 &&
      songButtonState == LOW &&
      !songButtonPressed &&
      colorMode == false) { // ✅ Only allow in Alphabet Mode
    
    //Button release detection
    if (digitalRead(songButtonPin) == HIGH) {
      songButtonPressed = false; // Reset the press flag if button released
    }

    songButtonPressed = true; // Mark button as pressed

    // Turn off RGB LED during song
    analogWrite(redPin, 0);
    analogWrite(greenPin, 0);
    analogWrite(bluePin, 0);

    if (!songPlaying) {
      // Playback state check&Display and play intro voice
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Alphabet Song Playing");
      player.play(19); // blank audio file to by pass the df player auto triggering
      delay(1000);

      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Alphabet Song Playing");
      player.play(20); // plays intro "Let's hear the Alphabet Song"
      delay(5000);

      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Alphabet Song Playing");
      player.play(21); // plays the Alphabet Song
      songPlaying = true; // Mark song as playing/state update

    } else {
      // If already playing, stop the song
      player.pause(); // Stop playback
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Song Stopped");
      songPlaying = false; // Reset flag
    }
  }

  // Reset song button state when released
  //Ensures the button can be pressed again later
  if (digitalRead(songButtonPin) == HIGH) {
    songButtonPressed = false;
  }

  // Time-based unlock to Unlocks the song button 3 seconds after switching modes
  if (songLockout && millis() - modeSwitchTime > 3000) {
    songLockout = false; // ✅ Unlock song button after 3 seconds
  }
}
