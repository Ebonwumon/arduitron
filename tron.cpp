#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#define TFT_CS 6
#define TFT_DC 7
#define TFT_RST 8
#define LCD_WIDTH 160
#define LCD_HEIGHT 128
#define JOYSTICK_BUTTON_PIN 9
#define JOYSTICK_MOVE_X_PIN 0 //pins x,y are reversed because of screen orientation
#define JOYSTICK_MOVE_Y_PIN 1
typedef struct {
  uint8_t x;
  uint8_t y;
} position_t;

typedef struct {
  int8_t x;
  int8_t y;
} movement_t;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

int joystickXCentre;
int joystickYCentre;
//bool wallPositions[160][128] = {false};
uint8_t wallPositions[128][20] = { 0 };
bool debug = false;
position_t currentPosition = {158,1}; 
movement_t direction = {-2, 0};

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
 * TODO DOCS
 */
bool legalPosition(position_t pos);


/*
 * TODO DOCS
 */
bool intersects(position_t dotPlace);

void addWallPosition(position_t pos);

bool getWallPosition(position_t pos);

void printWalls();

void setup() {
  Serial.begin(9600);
  tft.initR(INITR_REDTAB);
  joystickXCentre = analogRead(JOYSTICK_MOVE_X_PIN) - 512;
  joystickYCentre = analogRead(JOYSTICK_MOVE_Y_PIN) - 512;
  //currentPosition = {80, 64};

  tft.setRotation(1); //because our screen is nonstandard rotation
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0,0);
  tft.println("Starting in 2 seconds...");
  Serial.println("Starting...");
}

void loop() {
  tft.fillRect(currentPosition.x, currentPosition.y, 2, 2, ST7735_RED);
  addWallPosition(currentPosition); 
  movement_t temp = getJoystickInput();
  if (validInput(temp, direction)) direction = temp;
  else temp = direction; 
  currentPosition.x += temp.x;
  currentPosition.y += temp.y;
  if(!legalPosition(currentPosition)) {
    tft.setCursor(0, 80);
    tft.println("YOU SUPER LOSE");
    while(1);
  }
  delay(100);
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
  if (getWallPosition(pos)){
    return false;
  }
  else if (pos.x == LCD_WIDTH -1 || pos.x == 0 || pos.x +1 == LCD_WIDTH -1 || pos.x +1 == 0) {
    Serial.println("2: ");
    return false;
  }
  else if (pos.y == LCD_HEIGHT -1 || pos.y == 0 || pos.y +1 == LCD_HEIGHT -1 || pos.y +1 == 0) { 
    Serial.println("3: ");
    return false;
  }
  else return true;
}

bool intersects(position_t dotPlace) {
  
}

bool validInput(movement_t in, movement_t old) {
  if (in.x == 0 && in.y == 0) return false;
  if ((in.x * -1 == in.x && in.x != 0)  || (in.y * -1 == in.y && in.y != 0)) return false;
  return true;
}

void addWallPosition(position_t pos) {
  wallPositions[pos.y][pos.x/8] |= ((pos.x % 8 > 0) ? 1 << (pos.x % 8) - 1 : 1);
  wallPositions[pos.y][(pos.x+1)/8] |= (((pos.x+1) % 8 > 0) ? 1 << ((pos.x+1) % 8) - 1 : 1);
  wallPositions[pos.y +1][pos.x/8] |=  ((pos.x % 8 > 0) ? 1 << (pos.x % 8) - 1 : 1);
  wallPositions[pos.y +1][(pos.x +1)/8] |= (((pos.x+1) % 8 > 0) ? 1 << ((pos.x+1) % 8) - 1 : 1);
  if (debug) {
    Serial.print("adding Wall position: ");
    Serial.print(pos.y);
    Serial.print(", ");
    Serial.println(wallPositions[pos.y][pos.x/8], BIN);
  }
}

bool getWallPosition(position_t pos) {
  uint8_t val = wallPositions[pos.y][pos.x / 8];
  if (debug) {
    Serial.print("Val of WalPos at ");
    Serial.print(pos.y);
    Serial.print(": ");
    Serial.println(val, BIN);
  }
  switch (pos.x % 8) {
    case 0: return (val & 1); break;
    case 1: return (val & 2); break;
    case 2: return (val & 4); break;
    case 3: return (val & 8); break;
    case 4: return (val & 16); break;
    case 5: return (val & 32); break;
    case 6: return (val & 64); break;
    case 7: return (val & 128); break;
  } 
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
