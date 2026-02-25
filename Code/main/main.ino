#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// =====================
// BUTTONS
// =====================
#define BTN_UP     13
#define BTN_DOWN   12
#define BTN_LEFT   4
#define BTN_RIGHT  5
#define BTN_ENTER  17
#define BTN_BACK   18

// =====================
// CURSOR
// =====================
int cursorX = 20;
int cursorY = 20;
int cursorSpeed = 2;

// =====================
// DESKTOP BITMAP
// =====================
const unsigned char image_battery_status_bits[] PROGMEM = {
  0x3f,0xff,0x00,0x60,0x00,0x80,0xc0,0x00,
  0x80,0x80,0x00,0x80,0x80,0x00,0x80,0xc0,
  0x00,0x80,0x60,0x00,0x80,0x3f,0xff,0x00
};

const unsigned char image_user_interface_bits[] PROGMEM = {
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0x80,0x10,0x00,0x10,0x00,0x10,0x00,0x10,
  0x00,0x10,0x00,0x01,0x00,0x00,0x00,0x01,
  0x8f,0x13,0x80,0x13,0xff,0x90,0x10,0x10,
  0x7c,0x10,0x00,0x01,0x00,0x00,0x00,0x01,
  0x9f,0x94,0x7f,0x96,0x00,0xd0,0xba,0x10,
  0x82,0x10,0x00,0x01,0x00,0x00,0x00,0x01,
  0xb6,0xd4,0x00,0x55,0x01,0x50,0x44,0x10,
  0xba,0x10,0x00,0x01,0x00,0x00,0x00,0x01,
  0xb9,0xd7,0xff,0xd4,0x82,0x50,0x92,0x10,
  0x82,0x10,0x00,0x01,0x00,0x00,0x00,0x01,
  0xb9,0xd4,0x00,0x54,0x7c,0x51,0xab,0x10,
  0xaa,0x10,0x00,0x01,0x00,0x00,0x00,0x01,
  0xb6,0xd4,0x00,0x55,0x83,0x50,0x92,0x10,
  0x82,0x10,0x00,0x01,0x00,0x00,0x00,0x01,
  0x9f,0x94,0x00,0x56,0x00,0xd0,0x44,0x10,
  0xaa,0x10,0x00,0x01,0x00,0x00,0x00,0x01,
  0x8f,0x13,0xff,0x93,0xff,0x90,0xba,0x10,
  0x7c,0x10,0x00,0x01,0x00,0x00,0x00,0x01,
  0x80,0x10,0x00,0x10,0x00,0x10,0x00,0x10,
  0x00,0x10,0x00,0x01,0x00,0x00,0x00,0x01,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

#define CALC_X 60
#define CALC_Y 52
#define CALC_W 15
#define CALC_H 10

// =====================
// CURSOR
// =====================
void drawCursor() {
  display.drawLine(cursorX, cursorY, cursorX+6, cursorY+3, SSD1306_WHITE);
  display.drawLine(cursorX, cursorY, cursorX+3, cursorY+6, SSD1306_WHITE);
}

// =====================
// INPUT
// =====================
void updateInput() {
  if(!digitalRead(BTN_UP))    cursorY -= cursorSpeed;
  if(!digitalRead(BTN_DOWN))  cursorY += cursorSpeed;
  if(!digitalRead(BTN_LEFT))  cursorX -= cursorSpeed;
  if(!digitalRead(BTN_RIGHT)) cursorX += cursorSpeed;

  cursorX = constrain(cursorX,0,127);
  cursorY = constrain(cursorY,0,63);
}

// =====================
// DESKTOP
// =====================
void drawDesktop() {
  display.clearDisplay();
  display.drawBitmap(0,52, image_user_interface_bits, 128,12, SSD1306_WHITE);
  display.setCursor(97,55);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.print("12:34");
  display.drawBitmap(77,54, image_battery_status_bits, 17,8, SSD1306_WHITE);
  drawCursor();
  display.display();
}

// =====================
// CALCULATOR
// =====================
bool inCalculator = false;
String calcInput = "";
String calcResult = "";

// pozycje kafelków kalkulatora
const char* keys[4][4] = {
  {"C","*","/","-"},
  {"7","8","9","+"},
  {"4","5","6","="},
  {"1","2","3","0"}
};
int keyX[4] = {10, 40, 70, 100};
int keyY[4] = {20, 30, 40, 50};
int keyWidth = 20;
int keyHeight = 10;
int selectedRow = 0;
int selectedCol = 0;

int simpleCalc(String s) {
  // bardzo prosty parser dla +,-,*,/
  double total = 0;
  char op = '+';
  String num = "";
  for(int i=0;i<s.length();i++){
    char c = s[i];
    if(c>='0' && c<='9') num += c;
    else {
      double n = num.toFloat();
      if(op=='+') total += n;
      else if(op=='-') total -= n;
      else if(op=='*') total *= n;
      else if(op=='/') total = n!=0 ? total / n : 0;
      op = c;
      num = "";
    }
  }
  if(num!=""){
    double n = num.toFloat();
    if(op=='+') total += n;
    else if(op=='-') total -= n;
    else if(op=='*') total *= n;
    else if(op=='/') total = num.toFloat()!=0 ? total / num.toFloat() : 0;
  }
  return (int)total;
}

void drawCalculator() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print(calcInput);
  display.setCursor(0,10);
  display.print(calcResult);

  for(int r=0;r<4;r++){
    for(int c=0;c<4;c++){
      int x = keyX[c];
      int y = keyY[r];
      display.drawRect(x,y,keyWidth,keyHeight, SSD1306_WHITE);
      display.setCursor(x+5,y+2);
      display.print(keys[r][c]);
      // zaznaczony kafelek
      if(r==selectedRow && c==selectedCol){
        display.drawRect(x-1,y-1,keyWidth+2,keyHeight+2, SSD1306_WHITE);
      }
    }
  }
  drawCursor();
  display.display();
}

void calculatorLoop() {
  drawCalculator();

  // poruszanie zaznaczeniem kafelków
  if(!digitalRead(BTN_UP)) selectedRow = (selectedRow + 3) % 4;
  if(!digitalRead(BTN_DOWN)) selectedRow = (selectedRow + 1) % 4;
  if(!digitalRead(BTN_LEFT)) selectedCol = (selectedCol + 3) % 4;
  if(!digitalRead(BTN_RIGHT)) selectedCol = (selectedCol + 1) % 4;

  if(!digitalRead(BTN_ENTER)){
    String key = keys[selectedRow][selectedCol];
    if(key=="C"){ calcInput=""; calcResult=""; }
    else if(key=="="){ calcResult = String(simpleCalc(calcInput)); calcInput=""; }
    else calcInput += key;
    delay(200); // debounce
  }

  if(!digitalRead(BTN_BACK)) inCalculator = false;
  delay(50);
}

// =====================
// SETUP
// =====================
void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);

  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_ENTER, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}

// =====================
// LOOP
// =====================
void loop() {
  updateInput();

  if(!digitalRead(BTN_ENTER) &&
     cursorX >= CALC_X && cursorX <= CALC_X+CALC_W &&
     cursorY >= CALC_Y && cursorY <= CALC_Y+CALC_H) {
    inCalculator = true;
    selectedRow = 0;
    selectedCol = 0;
    while(inCalculator) calculatorLoop();
  }

  drawDesktop();
  delay(16);
}