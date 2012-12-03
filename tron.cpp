#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <SD.h>
#include "lcd_image.h"

#define SD_CS 5
#define TFT_CS 6
#define TFT_DC 7
#define TFT_RST 8
#define LCD_WIDTH 160
#define LCD_HEIGHT 128
#define JOYSTICK_BUTTON_PIN 9
#define JOYSTICK_MOVE_X_PIN 0 //pins x,y are reversed because of screen orientation
#define JOYSTICK_MOVE_Y_PIN 1
#define COUNTDOWN_START_RED_1 22
#define COUNTDOWN_START_RED_2 30
#define COUNTDOWN_MID_RED_1 24
#define COUNTDOWN_MID_RED_2 28
#define COUNTDOWN_GREEN 26
#define WALL_ARRAY_SIZE = 2560 

typedef struct {
  uint8_t x;
  uint8_t y;
} position_t;

typedef struct {
  int8_t x;
  int8_t y;
} movement_t;

typedef struct {
  position_t currentPosition;
  movement_t direction;
  uint8_t score;
  uint16_t colour;
} player_t;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

int joystickXCentre;
int joystickYCentre;
uint8_t wallPositions[128][20] = { 0 };
bool debug = false;
bool gameStarted = false;
bool gameCreated = false;
int winner;
Sd2Card card;
lcd_image_t logoImage = { "tron.lcd", 100, 50 };
player_t player1;
player_t player2;

/* Takes no input, returns position_t of the amount of *increase* required to add to x,y
 *  eg. movement_t { x = -2; y = 0 } would mean decrease x by 1. It will always return one value zero
 *  the other always -2 or 2 unless the joystick is at identical x,y
 * */
movement_t getJoystickInput();
/* Takes the current movement_t taken from the joystick and the old movement_t that maintains speed.
 * valid inputs have: a direction and cannot reverse directions on the spot. they must turn
 */
bool validInput(movement_t in, movement_t old);
/*
 * Checks position for legality. Returns false if illegal. Legal positions have:
 *  -no walls present
 *  -not in contact with the borders of the screen
 */
bool legalPosition(position_t pos);
/*
 * takes a position_t and adds walls to the four pixel block starting at the top left
 */
void addWallPosition(position_t pos);
/*
 * checks single pixel for a wall. Returns true if wall, false otherwise. Must be called four times to check all of cursor size
 */
bool getWallPosition(uint8_t x, uint8_t y);
void printWalls();
/*
 * Takes a string and outputs it to both the serial monitor and LCD display
 */
void dualPrint(char *s);

void drawGUI();

/*
 * Waits for hardware. Takes a digital hardware pin and the desired state (HIGH/LOW) and will wait until the hardware is in that state
 */
bool waitUntil(int pin, bool pos);

bool startNetwork();
/*
 * Handles the transmitting of spawn locations accross serial port
 */
void setSpawns(player_t *p1, player_t *p2);
void setColour(player_t *p1, player_t *p2);
/*
 * called only by the leading party (winner of random flip). sets the current position of both players to an acceptable opposite
 * spawn position, randomly decided
 */
void getSpawns(player_t *p1, player_t *p2); 

uint8_t getUint();

void sendDeltas(movement_t *m);

void receiveDeltas(movement_t *m);
void startCountdown();
int gameOver(position_t *p1, position_t *p2);

int8_t getInt();
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  tft.initR(INITR_REDTAB);
  randomSeed(analogRead(4));
  joystickXCentre = analogRead(JOYSTICK_MOVE_X_PIN) - 512;
  joystickYCentre = analogRead(JOYSTICK_MOVE_Y_PIN) - 512;
  pinMode(JOYSTICK_BUTTON_PIN, INPUT_PULLUP);
  pinMode(COUNTDOWN_START_RED_1, OUTPUT);
  pinMode(COUNTDOWN_START_RED_2, OUTPUT);
  pinMode(COUNTDOWN_MID_RED_1, OUTPUT);
  pinMode(COUNTDOWN_MID_RED_2, OUTPUT);
  pinMode(COUNTDOWN_GREEN, OUTPUT);
  tft.setRotation(1); //because our screen is nonstandard rotation
  player1.score = 0;
  player2.score = 0;
  if (!SD.begin(SD_CS)) {
    Serial.println("SD Init Failed");
    return;
  }
  if (!card.init(SPI_HALF_SPEED, SD_CS)) {
    Serial.println("Raw SD Init failed");
    while (1);
  }
  tft.fillScreen(ST7735_BLACK);
}

void loop() {
  // Though counter-intuitive, game creation cannot be in setup because of varying arduino boot times and boot gibberish
  if (!gameCreated) {  
    drawGUI();
    while(!waitUntil(JOYSTICK_BUTTON_PIN, false));
    if (!startNetwork()) {
      //TODO FAIL CASE
    }
    // TODO COLOUR SELECTION
    gameCreated = true;
  }
  if (!gameStarted) {
    setSpawns(&player1, &player2);
    setColour(&player1, &player2);
    Serial.println(player1.currentPosition.x);
    tft.fillScreen(ST7735_BLACK);
    startCountdown();
    gameStarted = true;
  }
  winner = gameOver(&player1.currentPosition, &player2.currentPosition);
  if (winner) {
    tft.setCursor(0, 80);
    String message;
    switch (winner) {
      case -1: message = "YOU SUPER TIE"; break;
      case 1: message = "YOU SUPER WIN"; player1.score++; break;
      case 2: message = "YOU SUPER LOSE"; player2.score++; break;
    }
    tft.println(message);
    tft.println("SCORES:");
    tft.print("You: ");
    tft.print(player1.score);
    tft.print(" | Him: ");
    tft.println(player2.score);
    tft.println("Again? <Press Joystick>");
    waitUntil(JOYSTICK_BUTTON_PIN, LOW);
    memset(&wallPositions, 0, 2560); // 2560 is a magic number because size_ts were acting unexpectedly 
    gameStarted = false;
    tft.fillScreen(ST7735_BLACK);
  } else {
    // add a wall ad draw car at current position
    addWallPosition(player1.currentPosition);
    addWallPosition(player2.currentPosition);
    tft.fillRect(player1.currentPosition.x, player1.currentPosition.y, 2, 2, player1.colour);
    tft.fillRect(player2.currentPosition.x, player2.currentPosition.y, 2, 2, player2.colour);
    movement_t temp = getJoystickInput();
    if (validInput(temp, player1.direction)) player1.direction = temp;
    else temp = player1.direction;
    sendDeltas(&temp);
    receiveDeltas(&player2.direction);
    player1.currentPosition.x += temp.x;
    player1.currentPosition.y += temp.y;
    player2.currentPosition.x += player2.direction.x;
    player2.currentPosition.y += player2.direction.y; 
    
    delay(75);
  }
}
movement_t getJoystickInput() {
  int8_t joystickXMap;
  int8_t joystickYMap;
  joystickXMap = constrain(map(analogRead(JOYSTICK_MOVE_X_PIN) - joystickXCentre, 1023, 0, -100, 101), -100, 100);
  joystickYMap = constrain(map(analogRead(JOYSTICK_MOVE_Y_PIN) - joystickYCentre, 0, 1023, -100, 101), -100, 100);
  
  if (joystickXMap * joystickXMap  > joystickYMap * joystickYMap){
    joystickYMap = 0;
    joystickXMap = (joystickXMap < 0) ? -2 : 2;
  } else if (joystickXMap == joystickYMap) {
    joystickXMap = 0;
    joystickYMap = 0;
  } else {
    joystickXMap = 0;
    joystickYMap = (joystickYMap < 0) ? -2 : 2;
  }
  movement_t temp = {joystickXMap, joystickYMap};
  return  temp ;
}

bool legalPosition(position_t pos) {
  bool hasWall = false;
  for (uint8_t i = 0; i < 2; i++) {
    for (uint8_t j = 0; j < 2; j++) {
      hasWall |= getWallPosition(pos.x + i, pos.y + j);
      }
    }
  if (hasWall) {
    return false;
  }
  else if (pos.x == LCD_WIDTH || pos.x == 0 || pos.x +1 == LCD_WIDTH || pos.x +1 == 0) { // TODO UINT comparisons for -1 ?
    return false;
  }
  else if (pos.y == LCD_HEIGHT || pos.y == 0 || pos.y +1 == LCD_HEIGHT || pos.y +1 == 0) { 
    return false;
  }
  else return true;
}

bool validInput(movement_t in, movement_t old) {
  if (in.x == 0 && in.y == 0) return false;
  if ((in.x * -1 == old.x && in.x != 0)  || (in.y * -1 == old.y && in.y != 0)) return false;
  return true;
}

void addWallPosition(position_t pos) {
  uint8_t divisionOfXPosition = pos.x/8;
  uint8_t modOfXPosition = pos.x % 8;
  uint8_t divisionOfX2Position = (pos.x +1) / 8;
  uint8_t modOfX2Position = (pos.x +1) % 8;
  
  wallPositions[pos.y][divisionOfXPosition] |= (1 << modOfXPosition);
  wallPositions[pos.y][divisionOfX2Position] |= (1 << modOfX2Position);
  wallPositions[pos.y +1][(divisionOfXPosition)] |=  (1 << modOfXPosition);
  wallPositions[pos.y +1][divisionOfX2Position] |= (1 << modOfX2Position);
}

bool getWallPosition(uint8_t x, uint8_t y) {
  uint8_t val = wallPositions[y][x / 8];
  switch (x % 8) {
    case 0: return (val & 1); 
    case 1: return (val & 2);
    case 2: return (val & 4); 
    case 3: return (val & 8); 
    case 4: return (val & 16); 
    case 5: return (val & 32);
    case 6: return (val & 64);
    case 7: return (val & 128); 
  } 
  return 0;
}

void drawGUI() {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0,70);
  dualPrint("Play a game?");
  dualPrint("<Press Joystick>");
  lcd_image_draw(&logoImage, &tft, 0, 0, 30, 10, 100, 50);
}

void printWalls() {
  for (int i=0; i < 10; i++) {
    Serial.print(i);
    Serial.print(": ");
    for (int j=0; j < 20; j++) {
      Serial.print(wallPositions[i][j], BIN);
      Serial.print(", ");
    }
    Serial.println();
    Serial.println();
  }
}

bool waitUntil(int pin, bool pos) {
  while (digitalRead(pin) != pos);
  return true;
}

bool startNetwork() {
  Serial1.write('r');
  for (int i=0; i < 10; i++) {

  if (Serial1.peek() == 'r') {
    Serial1.read();
    dualPrint("Connection established!");
    return true;
  }
  else {
    // TODO fail case
    dualPrint("Establishing connection...");
    delay(500);
  }
  }
}

void setSpawns(player_t *p1, player_t *p2) {
  uint8_t myFlip = random(0, 255);
  uint8_t hisFlip = 0;
  Serial1.write(myFlip);
  while (hisFlip == 0) {
    if (Serial1.available()) {
      hisFlip = Serial1.read();
      if (myFlip > hisFlip) {
        getSpawns(p1, p2);
        Serial1.write((uint8_t)p1->currentPosition.x);
        Serial1.write((uint8_t)p1->currentPosition.y);
        Serial1.write((int8_t)p1->direction.x);
        Serial1.write((int8_t)p1->direction.y);
        Serial1.write((uint8_t)p2->currentPosition.x);
        Serial1.write((uint8_t)p2->currentPosition.y);
        Serial1.write((int8_t)p2->direction.x);
        Serial1.write((int8_t)p2->direction.y);
      } else {
        p2->currentPosition.x = getUint();
        p2->currentPosition.y = getUint();
        p2->direction.x = getInt();
        p2->direction.y = getInt();
        p1->currentPosition.x = getUint();
        p1->currentPosition.y = getUint();
        p1->direction.x = getInt();
        p1->direction.y = getInt();
      }
    } else {
      Serial.println("Serial is not available");
    }
  }
  Serial.println("finished creation");
}

void setColour(player_t *p1, player_t *p2) {
  p1->colour = ST7735_RED;
  p2->colour = ST7735_BLUE;
}

void getSpawns(player_t *p1, player_t *p2) {
  uint8_t randNum = random(0,2);
  Serial.println("randNum: ");
  Serial.println(randNum);
  if (randNum == 0) {
    position_t pos1 = {8, 64}; movement_t dir1 = {2, 0}; 
    position_t pos2 = {152, 64}; movement_t dir2 = {-2, 0};  
    p1->currentPosition = pos1;
    p1->direction = dir1;
    p2->currentPosition = pos2;
    p2->direction = dir2;
  } else {
    position_t pos1 = {80, 120}; movement_t dir1 = {0, -2}; 
    position_t pos2 = {80, 8}; movement_t dir2 = {0, 2};
    p1->currentPosition = pos1;
    p1->direction = dir1;
    p2->currentPosition = pos2;
    p2->direction = dir2;
  }
}

void sendDeltas(movement_t *m) {
  Serial1.write((int8_t)m->x);
  Serial1.write((int8_t)m->y);
}

void receiveDeltas(movement_t *m) {
  int8_t deltx = getInt();
  int8_t delty = getInt();
  Serial.print("deltx: ");
  Serial.println(deltx);
  Serial.print("delty: ");
  Serial.println(delty);
  m->x = deltx;
  m->y = delty;
}

void dualPrint(char *s) {
  Serial.println(s);
  tft.println(s);
}

uint8_t getUint() {
  int val = 0;
  Serial.println("getUint");
  while (val == 0) {
    if (Serial1.available()) {
      val = Serial1.read();
      return (uint8_t) val;
    }
  }
}

int8_t getInt() {
  int val = 0;
  while (val == 0) {
    if (Serial1.available()) {
      val = Serial1.read();
      return (int8_t) val;
    }
  }
}

int gameOver(position_t *p1, position_t *p2) {
  bool p1legal = legalPosition(*p1);
  bool p2legal = legalPosition(*p2);
  if (p1legal && !p2legal) return 1;
  if (!p1legal && p2legal) return 2;
  if (!p1legal && !p2legal) return -1;
  return 0;
}



void startCountdown() {
  digitalWrite(COUNTDOWN_GREEN, LOW);
  digitalWrite(COUNTDOWN_START_RED_1, HIGH);
  digitalWrite(COUNTDOWN_START_RED_2, HIGH);
  delay(700);
  digitalWrite(COUNTDOWN_MID_RED_1, HIGH);
  digitalWrite(COUNTDOWN_MID_RED_2, HIGH);
  delay(700);
  digitalWrite(COUNTDOWN_START_RED_1, LOW);
  digitalWrite(COUNTDOWN_START_RED_2, LOW);
  digitalWrite(COUNTDOWN_MID_RED_1, LOW);
  digitalWrite(COUNTDOWN_MID_RED_2, LOW);
  digitalWrite(COUNTDOWN_GREEN, HIGH);
}

/*void printPlayer(player_t *p) {
  Serial.print("player has x: ")
}*/
