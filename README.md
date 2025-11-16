# 🎨🔤 Color-Alpha-EduKit

An Arduino-based interactive learning tool designed to help children explore colors and letters through RFID cards, audio feedback, and LED visuals.

---

## 📖 Overview
Color-Alpha-EduKit is a modular learning system that combines RFID card scanning, potentiometer-based color selection, and audio playback to create an engaging experience for early learners. It helps children associate letters and colors with sounds and visuals, reinforcing memory and recognition.

---

## ❗ Problem Statement
Traditional learning tools often lack interactivity and multisensory engagement. This project addresses that gap by providing a hands-on, voice-enabled system that supports both color and alphabet learning in a playful way.

---
## 🔍 Key Learning Goals Addressed

- Letter recognition through RFID cards  
- Color identification via potentiometer and RGB LED  
- Auditory reinforcement using DFPlayer Mini  
- Independent song playback for memory and rhythm  
- Modular design for classroom and STEM demos

---
## 🌟 Features

- **Alphabet Mode**: Scan RFID cards to hear phrases like “A for Apple”  
- **Color Mode**: Select and hear color names using a potentiometer  
- **Alphabet Song Button**: Play/stop the song independently of mode  
- **LCD Display**: Shows current mode, scanned letter, and feedback  
- **Debounce Logic**: Prevents accidental triggers and ensures smooth operation

---
## 🧩 Components Used

| Component              | Quantity | Purpose                             |
|------------------------|----------|-------------------------------------|
| Arduino Uno            | 1        | Main controller                     |
| MFRC522 RFID Reader    | 1        | Detects RFID cards                  |
| DFPlayer Mini          | 1        | Plays MP3 audio                     |
| RGB LED                | 1        | Displays selected color             |
| Potentiometer          | 1        | Selects color input                 |
| 16x2 LCD Display       | 1        | Shows feedback                      |
| Push Buttons           | 2        | Mode switch and song trigger        |
| SD Card (with MP3s)    | 1        | Stores audio files                  |

---
## 🧠 System Architecture
- RFID reader detects card UID
- LCD displays letter and phrase
- DFPlayer Mini plays audio
- Potentiometer selects color
- RGB LED lights up selected color
- Button toggles Alphabet Song

---

## 🔧 Hardware Used
- Arduino Uno  
- MFRC522 RFID Reader  
- RGB LED  
- 16x2 LCD Display  
- DFPlayer Mini MP3 Module  
- Potentiometer  
- Push Buttons  

---

## 💻 Software Used
- Arduino IDE  
- MFRC522 Library  
- LiquidCrystal Library  
- DFPlayerMini Library  

---

## 🌟 Features
- Alphabet Mode: Scan cards to hear phrases like “A for Apple”
- Color Mode: Select and hear color names
- Alphabet Song: Play/stop with a dedicated button
- Modular code structure for easy expansion

---

## 🚀 How to Run
1. Upload `ColorAlphaEduKit.ino` to Arduino Uno  
2. Connect components as per wiring diagram  
3. Insert MP3 files into DFPlayer SD card  
4. Power up and interact!

---

## 👥 Team Members
- Hasitha Rathnayake (SLIIT)

---

## 🔗 References
- [Arduino Libraries](https://www.arduino.cc/en/Reference/Libraries)
- [DFPlayer Mini Guide](https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299)
- [MFRC522 RFID Setup](https://randomnerdtutorials.com/security-access-using-rfid-arduino/)
