#include <Wire.h>
#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x27, 16, 2);


int speakerPin = 2;                       // transistor base connected to 8 ohm speaker 
// 6 = red, 7 = white, 8 = yellow, 9 = green, 10 = blue
const int ledPins[5] = {6, 7, 8, 9, 10};


const int START_SEC = 50;                 // 50 = starts at 11:59:50 PM (10-second countdown)


int length = 28;                          // the number of notes
char notes[] = "GGAGcB GGAGdc GGxecBA yyecdc";
int beats[] = {2,2,8,8,8,16,1,2,2,8,8,8,16,1,2,2,8,8,8,8,16,1,2,2,8,8,8,16};
int tempo = 200;                          // time delay between notes


// Custom LCD characters
byte heart[8] = {
  0b00000, 0b01010, 0b11111, 0b11111,
  0b11111, 0b01110, 0b00100, 0b00000
};
byte smiley[8] = {
  0b00000, 0b01010, 0b01010, 0b00000,
  0b10001, 0b01110, 0b00000, 0b00000
};


// Overwrites a whole row (padded with spaces) so the screen never needs clear()
void printLine(int row, const char* text) {
  lcd.setCursor(0, row);
  int n = 0;
  while (text[n] && n < 16) { lcd.print(text[n]); n++; }
  while (n < 16) { lcd.print(' '); n++; }
}


void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(tone);
  }
}


void allLeds(bool on) {
  for (int i = 0; i < 5; i++) digitalWrite(ledPins[i], on ? HIGH : LOW);
}


// Pitch decides which colour lights up: low = red ... high = blue
int ledForTone(int t) {
  if (t >= 1275) return 0;   // G  = red
  if (t >= 1136) return 1;   // A  = white
  if (t >= 1014) return 2;   // B  = yellow
  if (t >= 834)  return 3;   // c, d = green
  return 4;                  // e, x, y = blue
}


void playNote(char note, int duration) {
  char names[] = {'C', 'D', 'E', 'F', 'G', 'A', 'B',
                  'c', 'd', 'e', 'f', 'g', 'a', 'b',
                  'x', 'y'};


  int tones[] = {1915, 1700, 1519, 1432, 1275, 1136, 1014,
                 956,  834,  765,  593,  468,  346,  224,
                 655,  715};


  int SPEE = 5;


  for (int i = 0; i < 16; i++) {
    if (names[i] == note) {
      int led = ledForTone(tones[i]);
      digitalWrite(ledPins[led], HIGH);   // LED glows while the note plays
      playTone(tones[i], duration / SPEE);
      digitalWrite(ledPins[led], LOW);
    }
  }
}


void showCountdown(int sec) {
  char line1[17];
  char line2[17];
  sprintf(line1, "Time left: %2ds", 60 - sec);
  sprintf(line2, "11:59:%02d PM", sec);
  printLine(0, line1);
  printLine(1, line2);
}


void countdown() {
  for (int sec = START_SEC; sec < 60; sec++) {
    unsigned long t0 = millis();
    showCountdown(sec);


    digitalWrite(ledPins[0], HIGH);                    // red LED on + beep
    playTone((sec >= 57) ? 333 : 500, 130);            // higher beep in the last 3 sec
    digitalWrite(ledPins[0], LOW);                     // red LED off + silence


    while (millis() - t0 < 1000) {}
  }
}


void fanfare() {
  printLine(0, "  12:00:00 AM");
  printLine(1, "   IT'S TIME!");


  int fan[5] = {956, 765, 655, 478, 478};              // rising notes
  for (int i = 0; i < 5; i++) {
    digitalWrite(ledPins[i], HIGH);                    // LEDs light one by one
    playTone(fan[i], i == 4 ? 500 : 140);
    delay(20);
  }
  delay(300);
  allLeds(false);
}


// Text stays steady on the screen, never blinks
void showBirthdayText() {
  printLine(0, " Happy Birthday");
  printLine(1, "    to you!");
}


// Only the hearts and smileys appear (on = true) or disappear (on = false)
void decor(bool on) {
  if (on) {
    lcd.setCursor(1, 1);  lcd.write(byte(0));   // heart
    lcd.setCursor(2, 1);  lcd.write(byte(1));   // smiley
    lcd.setCursor(12, 1); lcd.write(byte(1));   // smiley
    lcd.setCursor(13, 1); lcd.write(byte(0));   // heart
  } else {
    lcd.setCursor(1, 1);  lcd.print("  ");
    lcd.setCursor(12, 1); lcd.print("  ");
  }
}


void playSong() {
  showBirthdayText();
  decor(true);
  delay(400);


  int count = 0;
  for (int i = 0; i < length; i++) {
    if (notes[i] == ' ') {
      delay(beats[i] * tempo);                // rest
    } else {
      playNote(notes[i], beats[i] * tempo);
      count++;
      decor(count % 2 == 0);                  // only hearts and smileys toggle, in the silence after each note
    }
    delay(tempo);                             // silence between notes
  }


  // finale: all LEDs glow together once, then all off
  decor(true);
  allLeds(true);
  delay(1500);
  allLeds(false);
}


void setup() {
  pinMode(speakerPin, OUTPUT);
  digitalWrite(speakerPin, LOW);              // keep the transistor off at start
  for (int i = 0; i < 5; i++) pinMode(ledPins[i], OUTPUT);


  lcd.init();
  lcd.backlight();
  lcd.createChar(0, heart);
  lcd.createChar(1, smiley);


  printLine(0, "Birthday");
  printLine(1, "Countdown...");
  delay(1500);


  countdown();
  fanfare();
  playSong();
}


void loop() {
  // after the song: only hearts and smileys keep appearing and disappearing
  decor(true);
  delay(500);
  decor(false);
  delay(500);
}
