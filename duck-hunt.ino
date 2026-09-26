#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <Preferences.h>

// =====================================================
// CYD TFT
// =====================================================

#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1
#define TFT_BL   21

// =====================================================
// TOUCH
// =====================================================

#define TOUCH_MOSI 32
#define TOUCH_MISO 39
#define TOUCH_CLK  25
#define TOUCH_CS   33
#define TOUCH_IRQ  36

#define TOUCH_MIN_X 200
#define TOUCH_MAX_X 3900
#define TOUCH_MIN_Y 200
#define TOUCH_MAX_Y 3900

// =====================================================
// BUZZER
// =====================================================

#define BUZZER_PIN 22

// =====================================================
// SCREEN
// =====================================================

#define SCREEN_W 320
#define SCREEN_H 240
#define HUD_H 32

// =====================================================
// COLORS
// =====================================================

#define SKY_COLOR       0x5DDF
#define SKY_LIGHT       0x7EFF

#define GRASS_COLOR     0x05E0
#define GRASS_DARK      0x03C0

#define GROUND_COLOR    0xA285

#define BUSH_COLOR      0x04A0
#define BUSH_LIGHT      0x0700

#define DUCK_BODY       0xA145
#define DUCK_HEAD       0x0400
#define DUCK_WING       0xC618
#define DUCK_BEAK       0xFD20

#define DOG_COLOR       0xA145
#define DOG_DARK        0x4208
#define DOG_MUZZLE      0xFE73

// =====================================================
// HARDWARE
// =====================================================

SPIClass tftSPI(HSPI);
SPIClass touchSPI(VSPI);

Adafruit_ILI9341 tft(
  &tftSPI,
  TFT_DC,
  TFT_CS,
  TFT_RST
);

XPT2046_Touchscreen ts(
  TOUCH_CS,
  TOUCH_IRQ
);

Preferences preferences;

// =====================================================
// GAME STATES
// =====================================================

enum GameState {

  TITLE_SCREEN,

  DUCK_FLYING,
  DUCK_HIT,
  DUCK_FALLING,

  DOG_PICKUP,
  DOG_LAUGH,

  ROUND_OVER,
  GAME_OVER
};

GameState gameState =
  TITLE_SCREEN;

// =====================================================
// GAME DATA
// =====================================================

const int DUCKS_PER_ROUND = 10;

int roundNumber = 1;
int duckNumber = 0;

int hitsThisRound = 0;

int shotsLeft = 3;

long score = 0;
long highScore = 0;

// =====================================================
// DUCK DATA
// =====================================================

float duckX = 160;
float duckY = 120;

float oldDuckX = 160;
float oldDuckY = 120;

float duckVX = 2.5;
float duckVY = -1.5;

int wingFrame = 0;

unsigned long lastDuckFrame = 0;
unsigned long lastWingFrame = 0;

unsigned long duckStartedAt = 0;

unsigned long directionChangeAt = 0;

unsigned long noAmmoSince = 0;

// about 22 FPS
const unsigned long FRAME_TIME = 45;

// =====================================================
// STATE TIMER
// =====================================================

unsigned long stateStartedAt = 0;

// =====================================================
// TOUCH
// =====================================================

bool previousTouch = false;

// =====================================================
// TITLE MUSIC
// =====================================================

const int titleNotes[] = {

  392, 523, 659, 784,
  659, 523,

  440, 587, 698, 880,
  698, 587,

  523, 659, 784, 1047,
  784, 659
};

const int titleDurations[] = {

  120, 120, 120, 220,
  120, 180,

  120, 120, 120, 220,
  120, 180,

  120, 120, 150, 260,
  150, 250
};

const int TITLE_NOTE_COUNT =
  sizeof(titleNotes) /
  sizeof(titleNotes[0]);

int titleNote = 0;

unsigned long titleNoteStarted = 0;

bool titleMusicPlaying = false;

// =====================================================
// BASIC SOUND
// =====================================================

void soundOff() {

  ledcWriteTone(
    BUZZER_PIN,
    0
  );
}

// =====================================================
// SHOT
// =====================================================

void soundShot() {

  ledcWriteTone(
    BUZZER_PIN,
    1700
  );

  delay(12);

  ledcWriteTone(
    BUZZER_PIN,
    1100
  );

  delay(12);

  ledcWriteTone(
    BUZZER_PIN,
    650
  );

  delay(15);

  ledcWriteTone(
    BUZZER_PIN,
    250
  );

  delay(22);

  soundOff();
}

// =====================================================
// HIT
// =====================================================

void soundHit() {

  ledcWriteTone(
    BUZZER_PIN,
    950
  );

  delay(45);

  ledcWriteTone(
    BUZZER_PIN,
    1300
  );

  delay(55);

  ledcWriteTone(
    BUZZER_PIN,
    1800
  );

  delay(90);

  soundOff();
}

// =====================================================
// DUCK FALL
// =====================================================

void soundFall() {

  ledcWriteTone(
    BUZZER_PIN,
    900
  );

  delay(50);

  ledcWriteTone(
    BUZZER_PIN,
    700
  );

  delay(50);

  ledcWriteTone(
    BUZZER_PIN,
    500
  );

  delay(50);

  soundOff();
}

// =====================================================
// ESCAPE
// =====================================================

void soundEscape() {

  ledcWriteTone(
    BUZZER_PIN,
    520
  );

  delay(80);

  ledcWriteTone(
    BUZZER_PIN,
    400
  );

  delay(90);

  ledcWriteTone(
    BUZZER_PIN,
    280
  );

  delay(150);

  soundOff();
}

// =====================================================
// DOG LAUGH
// =====================================================

void soundDogLaugh() {

  for (
    int i = 0;
    i < 3;
    i++
  ) {

    ledcWriteTone(
      BUZZER_PIN,
      700
    );

    delay(70);

    ledcWriteTone(
      BUZZER_PIN,
      1000
    );

    delay(70);
  }

  soundOff();
}

// =====================================================
// ROUND START
// =====================================================

void soundRoundStart() {

  int notes[] = {
    523,
    659,
    784,
    1047
  };

  for (
    int i = 0;
    i < 4;
    i++
  ) {

    ledcWriteTone(
      BUZZER_PIN,
      notes[i]
    );

    delay(85);
  }

  soundOff();
}

// =====================================================
// ROUND WIN
// =====================================================

void soundRoundWin() {

  int notes[] = {

    523,
    659,
    784,
    1047,
    1319
  };

  for (
    int i = 0;
    i < 5;
    i++
  ) {

    ledcWriteTone(
      BUZZER_PIN,
      notes[i]
    );

    delay(100);
  }

  soundOff();
}

// =====================================================
// GAME OVER
// =====================================================

void soundGameOver() {

  ledcWriteTone(
    BUZZER_PIN,
    600
  );

  delay(120);

  ledcWriteTone(
    BUZZER_PIN,
    450
  );

  delay(140);

  ledcWriteTone(
    BUZZER_PIN,
    300
  );

  delay(180);

  ledcWriteTone(
    BUZZER_PIN,
    180
  );

  delay(300);

  soundOff();
}

// =====================================================
// TITLE MUSIC
// =====================================================

void startTitleMusic() {

  titleNote = 0;

  titleNoteStarted =
    millis();

  titleMusicPlaying =
    true;

  ledcWriteTone(
    BUZZER_PIN,
    titleNotes[0]
  );
}

void stopTitleMusic() {

  titleMusicPlaying =
    false;

  soundOff();
}

void updateTitleMusic() {

  if (
    !titleMusicPlaying
  ) {
    return;
  }

  unsigned long now =
    millis();

  if (
    now -
    titleNoteStarted >=
    titleDurations[titleNote]
  ) {

    titleNote++;

    if (
      titleNote >=
      TITLE_NOTE_COUNT
    ) {

      titleNote = 0;
    }

    ledcWriteTone(
      BUZZER_PIN,
      titleNotes[titleNote]
    );

    titleNoteStarted =
      now;
  }
}

// =====================================================
// CENTER TEXT
// =====================================================

void centerText(
  const char* text,
  int y,
  int size,
  uint16_t color
) {

  tft.setTextSize(size);

  tft.setTextColor(
    color
  );

  int16_t x1;
  int16_t y1;

  uint16_t w;
  uint16_t h;

  tft.getTextBounds(
    text,
    0,
    y,
    &x1,
    &y1,
    &w,
    &h
  );

  tft.setCursor(
    (SCREEN_W - w) / 2,
    y
  );

  tft.print(
    text
  );
}

// =====================================================
// BACKGROUND
// =====================================================

void drawBackground() {

  // sky

  tft.fillRect(
    0,
    HUD_H,
    SCREEN_W,
    138,
    SKY_COLOR
  );

  // horizon

  tft.fillRect(
    0,
    170,
    SCREEN_W,
    15,
    SKY_LIGHT
  );

  // grass

  tft.fillRect(
    0,
    185,
    SCREEN_W,
    55,
    GRASS_COLOR
  );

  // ground strip

  tft.fillRect(
    0,
    224,
    SCREEN_W,
    16,
    GROUND_COLOR
  );

  // bushes

  for (
    int x = -10;
    x < SCREEN_W + 10;
    x += 24
  ) {

    tft.fillCircle(
      x,
      187,
      16,
      BUSH_COLOR
    );

    tft.fillCircle(
      x + 10,
      178,
      13,
      BUSH_LIGHT
    );
  }

  // grass blades

  for (
    int x = 5;
    x < SCREEN_W;
    x += 18
  ) {

    tft.drawLine(
      x,
      215,
      x - 4,
      204,
      GRASS_DARK
    );

    tft.drawLine(
      x,
      215,
      x + 4,
      202,
      GRASS_DARK
    );
  }
}

// =====================================================
// REQUIRED HITS
// =====================================================

int requiredHits() {

  int required =
    6 +
    (roundNumber - 1) / 2;

  if (
    required > 10
  ) {

    required = 10;
  }

  return required;
}

// =====================================================
// HUD
// =====================================================

void drawHUD() {

  tft.fillRect(
    0,
    0,
    SCREEN_W,
    HUD_H,
    ILI9341_BLACK
  );

  tft.setTextSize(1);

  tft.setTextColor(
    ILI9341_WHITE
  );

  // ROUND

  tft.setCursor(
    4,
    4
  );

  tft.print(
    "ROUND"
  );

  tft.setCursor(
    4,
    17
  );

  tft.print(
    roundNumber
  );

  // DUCK

  tft.setCursor(
    50,
    4
  );

  tft.print(
    "DUCK"
  );

  tft.setCursor(
    50,
    17
  );

  tft.print(
    duckNumber
  );

  tft.print(
    "/10"
  );

  // SHOTS

  tft.setCursor(
    105,
    4
  );

  tft.print(
    "SHOT"
  );

  tft.setCursor(
    105,
    17
  );

  for (
    int i = 0;
    i < 3;
    i++
  ) {

    if (
      i < shotsLeft
    ) {

      tft.setTextColor(
        ILI9341_YELLOW
      );

      tft.print("*");

    } else {

      tft.setTextColor(
        ILI9341_DARKGREY
      );

      tft.print("-");
    }
  }

  // HIT

  tft.setTextColor(
    ILI9341_WHITE
  );

  tft.setCursor(
    155,
    4
  );

  tft.print(
    "HIT"
  );

  tft.setCursor(
    155,
    17
  );

  tft.print(
    hitsThisRound
  );

  // SCORE

  tft.setCursor(
    200,
    4
  );

  tft.print(
    "SCORE"
  );

  tft.setCursor(
    200,
    17
  );

  tft.print(
    score
  );

  // HIGH

  tft.setCursor(
    275,
    4
  );

  tft.print(
    "HI"
  );

  tft.setCursor(
    275,
    17
  );

  if (
    highScore > 99999
  ) {

    tft.print(
      "MAX"
    );

  } else {

    tft.print(
      highScore
    );
  }
}

// =====================================================
// DRAW DUCK
// =====================================================

void drawDuck(
  int x,
  int y,
  bool facingRight,
  int wing
) {

  int dir =
    facingRight ?
    1 :
    -1;

  // ===================================================
  // BODY
  // ===================================================

  tft.fillRoundRect(
    x - 12,
    y - 7,
    24,
    14,
    6,
    DUCK_BODY
  );

  // ===================================================
  // HEAD
  // ===================================================

  tft.fillCircle(
    x + dir * 11,
    y - 7,
    7,
    DUCK_HEAD
  );

  // ===================================================
  // WHITE NECK
  // ===================================================

  if (
    facingRight
  ) {

    tft.fillRect(
      x + 5,
      y - 5,
      6,
      5,
      ILI9341_WHITE
    );

  } else {

    tft.fillRect(
      x - 11,
      y - 5,
      6,
      5,
      ILI9341_WHITE
    );
  }

  // ===================================================
  // BEAK
  // ===================================================

  if (
    facingRight
  ) {

    tft.fillTriangle(
      x + 17,
      y - 8,

      x + 25,
      y - 5,

      x + 17,
      y - 3,

      DUCK_BEAK
    );

  } else {

    tft.fillTriangle(
      x - 17,
      y - 8,

      x - 25,
      y - 5,

      x - 17,
      y - 3,

      DUCK_BEAK
    );
  }

  // ===================================================
  // EYE
  // ===================================================

  tft.fillCircle(
    x + dir * 13,
    y - 9,
    2,
    ILI9341_WHITE
  );

  tft.fillCircle(
    x + dir * 14,
    y - 9,
    1,
    ILI9341_BLACK
  );

  // ===================================================
  // TAIL
  // ===================================================

  if (
    facingRight
  ) {

    tft.fillTriangle(
      x - 10,
      y,

      x - 19,
      y - 5,

      x - 17,
      y + 5,

      DUCK_BODY
    );

  } else {

    tft.fillTriangle(
      x + 10,
      y,

      x + 19,
      y - 5,

      x + 17,
      y + 5,

      DUCK_BODY
    );
  }

  // ===================================================
  // WING
  // ===================================================

  if (
    wing == 0
  ) {

    tft.fillTriangle(
      x - 5,
      y,

      x,
      y - 17,

      x + 6,
      y,

      DUCK_WING
    );

  } else if (
    wing == 1
  ) {

    tft.fillRoundRect(
      x - 9,
      y - 3,
      18,
      9,
      4,
      DUCK_WING
    );

  } else {

    tft.fillTriangle(
      x - 5,
      y + 2,

      x,
      y + 17,

      x + 6,
      y + 2,

      DUCK_WING
    );
  }
}

// =====================================================
// ERASE DUCK
// =====================================================

void eraseDuck(
  int x,
  int y
) {

  tft.fillRect(
    x - 30,
    y - 25,
    61,
    50,
    SKY_COLOR
  );
}

// =====================================================
// SHOT DUCK
// =====================================================

void drawShotDuck(
  int x,
  int y
) {

  // body

  tft.fillRoundRect(
    x - 12,
    y - 7,
    24,
    14,
    6,
    DUCK_BODY
  );

  // head

  tft.fillCircle(
    x,
    y - 10,
    7,
    DUCK_HEAD
  );

  // wing down

  tft.fillTriangle(
    x - 6,
    y + 4,

    x,
    y + 18,

    x + 6,
    y + 4,

    DUCK_WING
  );

  // stars

  tft.setTextSize(1);

  tft.setTextColor(
    ILI9341_YELLOW
  );

  tft.setCursor(
    x - 19,
    y - 23
  );

  tft.print("*");

  tft.setCursor(
    x + 15,
    y - 20
  );

  tft.print("*");
}

// =====================================================
// FALLING DUCK
// =====================================================

void drawFallingDuck(
  int x,
  int y
) {

  // vertical body

  tft.fillRoundRect(
    x - 7,
    y - 11,
    14,
    22,
    6,
    DUCK_BODY
  );

  // head downward

  tft.fillCircle(
    x,
    y + 11,
    5,
    DUCK_HEAD
  );

  // wings

  tft.fillTriangle(
    x - 5,
    y,

    x - 15,
    y - 5,

    x - 5,
    y + 6,

    DUCK_WING
  );

  tft.fillTriangle(
    x + 5,
    y,

    x + 15,
    y - 5,

    x + 5,
    y + 6,

    DUCK_WING
  );
}

// =====================================================
// DOG
// =====================================================

void drawDog(
  int x,
  int y,
  bool holdingDuck
) {

  // ears

  tft.fillRoundRect(
    x - 22,
    y - 33,
    11,
    27,
    5,
    DOG_DARK
  );

  tft.fillRoundRect(
    x + 11,
    y - 33,
    11,
    27,
    5,
    DOG_DARK
  );

  // head

  tft.fillCircle(
    x,
    y - 15,
    18,
    DOG_COLOR
  );

  // muzzle

  tft.fillRoundRect(
    x - 10,
    y - 13,
    20,
    13,
    6,
    DOG_MUZZLE
  );

  // eyes

  tft.fillCircle(
    x - 6,
    y - 18,
    2,
    ILI9341_BLACK
  );

  tft.fillCircle(
    x + 6,
    y - 18,
    2,
    ILI9341_BLACK
  );

  // nose

  tft.fillCircle(
    x,
    y - 10,
    3,
    ILI9341_BLACK
  );

  // body

  tft.fillRoundRect(
    x - 15,
    y,
    30,
    25,
    6,
    DOG_COLOR
  );

  if (
    holdingDuck
  ) {

    // captured duck body

    tft.fillRoundRect(
      x + 18,
      y - 14,
      20,
      12,
      5,
      DUCK_BODY
    );

    // head

    tft.fillCircle(
      x + 37,
      y - 13,
      5,
      DUCK_HEAD
    );

    // beak

    tft.fillTriangle(
      x + 41,
      y - 14,

      x + 48,
      y - 11,

      x + 41,
      y - 9,

      DUCK_BEAK
    );
  }
}

// =====================================================
// TITLE SCREEN
// =====================================================

void drawTitleScreen() {

  tft.fillScreen(
    SKY_COLOR
  );

  // grass

  tft.fillRect(
    0,
    170,
    SCREEN_W,
    70,
    GRASS_COLOR
  );

  // bushes

  for (
    int x = -10;
    x < SCREEN_W + 10;
    x += 25
  ) {

    tft.fillCircle(
      x,
      175,
      16,
      BUSH_COLOR
    );

    tft.fillCircle(
      x + 10,
      169,
      11,
      BUSH_LIGHT
    );
  }

  // ducks

  drawDuck(
    65,
    80,
    true,
    0
  );

  drawDuck(
    255,
    92,
    false,
    2
  );

  // dog

  drawDog(
    160,
    175,
    false
  );

  // title box

  tft.fillRoundRect(
    35,
    18,
    250,
    42,
    8,
    ILI9341_BLACK
  );

  tft.drawRoundRect(
    35,
    18,
    250,
    42,
    8,
    ILI9341_WHITE
  );

  centerText(
    "DUCK HUNT",
    29,
    3,
    ILI9341_YELLOW
  );

  centerText(
    "ESP32 EDITION",
    115,
    2,
    ILI9341_WHITE
  );

  centerText(
    "TAP TO START",
    143,
    2,
    ILI9341_YELLOW
  );

  tft.setTextSize(1);

  tft.setTextColor(
    ILI9341_WHITE
  );

  tft.setCursor(
    113,
    225
  );

  tft.print(
    "HIGH SCORE "
  );

  tft.print(
    highScore
  );
}

// =====================================================
// TOUCH
// =====================================================

bool getNewTap(
  int &screenX,
  int &screenY
) {

  bool touched =
    ts.touched();

  bool newTap =
    touched &&
    !previousTouch;

  previousTouch =
    touched;

  if (
    !newTap
  ) {

    return false;
  }

  TS_Point p =
    ts.getPoint();

  screenX =
    map(
      p.x,
      TOUCH_MIN_X,
      TOUCH_MAX_X,
      0,
      SCREEN_W
    );

  screenY =
    map(
      p.y,
      TOUCH_MIN_Y,
      TOUCH_MAX_Y,
      0,
      SCREEN_H
    );

  screenX =
    constrain(
      screenX,
      0,
      SCREEN_W - 1
    );

  screenY =
    constrain(
      screenY,
      0,
      SCREEN_H - 1
    );

  return true;
}

// =====================================================
// HIT TEST
// =====================================================

bool duckHit(
  int tapX,
  int tapY
) {

  int dx =
    abs(
      tapX -
      (int)duckX
    );

  int dy =
    abs(
      tapY -
      (int)duckY
    );

  // generous touchscreen hitbox

  return (
    dx <= 30 &&
    dy <= 24
  );
}

// =====================================================
// DUCK TIME LIMIT
// =====================================================

unsigned long duckTimeLimit() {

  long result =
    7000 -
    (roundNumber - 1) * 300;

  if (
    result < 3500
  ) {

    result = 3500;
  }

  return result;
}

// =====================================================
// NEW GAME
// =====================================================

void newGame() {

  Serial.println(
    "NEW GAME"
  );

  roundNumber = 1;

  score = 0;

  hitsThisRound = 0;

  duckNumber = 0;

  soundRoundStart();

  startRound();
}

// =====================================================
// START ROUND
// =====================================================

void startRound() {

  Serial.print(
    "ROUND "
  );

  Serial.println(
    roundNumber
  );

  hitsThisRound = 0;

  duckNumber = 0;

  startNextDuck();
}

// =====================================================
// START NEXT DUCK
// =====================================================

void startNextDuck() {

  duckNumber++;

  if (
    duckNumber >
    DUCKS_PER_ROUND
  ) {

    showRoundOver();

    return;
  }

  Serial.print(
    "Starting duck "
  );

  Serial.println(
    duckNumber
  );

  shotsLeft = 3;

  noAmmoSince = 0;

  drawBackground();

  drawHUD();

  float speed =
    2.3 +
    (roundNumber - 1) * 0.35;

  duckX =
    random(
      60,
      260
    );

  duckY =
    145;

  bool goRight =
    random(
      0,
      2
    );

  duckVX =
    goRight ?
    speed :
    -speed;

  duckVY =
    -(
      1.2 +
      random(
        5,
        18
      ) / 10.0
    );

  oldDuckX =
    duckX;

  oldDuckY =
    duckY;

  wingFrame = 0;

  unsigned long now =
    millis();

  duckStartedAt =
    now;

  directionChangeAt =
    now +
    random(
      600,
      1200
    );

  lastDuckFrame =
    now;

  lastWingFrame =
    now;

  gameState =
    DUCK_FLYING;

  stateStartedAt =
    now;

  drawDuck(
    (int)duckX,
    (int)duckY,
    duckVX > 0,
    wingFrame
  );
}

// =====================================================
// DUCK ESCAPED
// =====================================================

void duckEscaped() {

  Serial.println(
    "DUCK ESCAPED"
  );

  eraseDuck(
    (int)oldDuckX,
    (int)oldDuckY
  );

  drawBackground();

  drawHUD();

  drawDog(
    160,
    195,
    false
  );

  centerText(
    "HA HA!",
    102,
    3,
    ILI9341_RED
  );

  centerText(
    "DUCK ESCAPED",
    135,
    2,
    ILI9341_WHITE
  );

  soundDogLaugh();

  gameState =
    DOG_LAUGH;

  stateStartedAt =
    millis();
}

// =====================================================
// HIT DUCK
// =====================================================

void hitDuckNow() {

  Serial.println(
    "HIT!"
  );

  eraseDuck(
    (int)oldDuckX,
    (int)oldDuckY
  );

  hitsThisRound++;

  unsigned long elapsed =
    millis() -
    duckStartedAt;

  int timeBonus = 0;

  if (
    elapsed < 6000
  ) {

    timeBonus =
      (6000 - elapsed) / 20;
  }

  score +=
    500 +
    timeBonus +
    roundNumber * 100;

  if (
    score >
    highScore
  ) {

    highScore =
      score;
  }

  drawHUD();

  soundHit();

  drawShotDuck(
    (int)duckX,
    (int)duckY
  );

  gameState =
    DUCK_HIT;

  stateStartedAt =
    millis();
}

// =====================================================
// UPDATE FLYING DUCK
// =====================================================

void updateFlyingDuck() {

  unsigned long now =
    millis();

  // timeout

  if (
    now -
    duckStartedAt >=
    duckTimeLimit()
  ) {

    duckEscaped();

    return;
  }

  // no ammunition

  if (
    shotsLeft == 0 &&
    noAmmoSince > 0 &&
    now -
    noAmmoSince >=
    650
  ) {

    duckEscaped();

    return;
  }

  // wing animation

  if (
    now -
    lastWingFrame >=
    120
  ) {

    lastWingFrame =
      now;

    wingFrame++;

    if (
      wingFrame > 2
    ) {

      wingFrame = 0;
    }
  }

  // movement frame

  if (
    now -
    lastDuckFrame <
    FRAME_TIME
  ) {

    return;
  }

  lastDuckFrame =
    now;

  // erase old sprite

  eraseDuck(
    (int)oldDuckX,
    (int)oldDuckY
  );

  // movement

  duckX +=
    duckVX;

  duckY +=
    duckVY;

  // LEFT

  if (
    duckX < 32
  ) {

    duckX = 32;

    duckVX =
      abs(duckVX);
  }

  // RIGHT

  if (
    duckX > 288
  ) {

    duckX = 288;

    duckVX =
      -abs(duckVX);
  }

  // TOP

  if (
    duckY < 60
  ) {

    duckY = 60;

    duckVY =
      abs(duckVY);
  }

  // BOTTOM

  if (
    duckY > 145
  ) {

    duckY = 145;

    duckVY =
      -abs(duckVY);
  }

  // random direction

  if (
    now >=
    directionChangeAt
  ) {

    if (
      random(
        0,
        100
      ) < 55
    ) {

      duckVX =
        -duckVX;
    }

    duckVY =
      random(
        -20,
        21
      ) / 10.0;

    if (
      duckVY > -0.8 &&
      duckVY < 0.8
    ) {

      duckVY =
        random(
          0,
          2
        ) ?
        1.2 :
        -1.2;
    }

    directionChangeAt =
      now +
      random(
        500,
        1200
      );
  }

  // draw new

  drawDuck(
    (int)duckX,
    (int)duckY,
    duckVX > 0,
    wingFrame
  );

  oldDuckX =
    duckX;

  oldDuckY =
    duckY;
}

// =====================================================
// UPDATE FALLING DUCK
// =====================================================

void updateFallingDuck() {

  unsigned long now =
    millis();

  if (
    now -
    lastDuckFrame <
    55
  ) {

    return;
  }

  lastDuckFrame =
    now;

  eraseDuck(
    (int)oldDuckX,
    (int)oldDuckY
  );

  duckY += 6;

  if (
    duckY >= 150
  ) {

    drawBackground();

    drawHUD();

    drawDog(
      160,
      195,
      true
    );

    centerText(
      "GOT ONE!",
      105,
      2,
      ILI9341_YELLOW
    );

    gameState =
      DOG_PICKUP;

    stateStartedAt =
      millis();

    return;
  }

  drawFallingDuck(
    (int)duckX,
    (int)duckY
  );

  oldDuckX =
    duckX;

  oldDuckY =
    duckY;
}

// =====================================================
// ROUND OVER
// =====================================================

void showRoundOver() {

  if (
    score >
    highScore
  ) {

    highScore =
      score;

    preferences.putLong(
      "high",
      highScore
    );
  }

  drawBackground();

  drawHUD();

  int needed =
    requiredHits();

  bool passed =
    hitsThisRound >=
    needed;

  tft.fillRoundRect(
    38,
    55,
    244,
    130,
    12,
    ILI9341_BLACK
  );

  tft.drawRoundRect(
    38,
    55,
    244,
    130,
    12,
    ILI9341_WHITE
  );

  if (
    passed
  ) {

    centerText(
      "ROUND CLEAR!",
      70,
      2,
      ILI9341_GREEN
    );

  } else {

    centerText(
      "ROUND FAILED",
      70,
      2,
      ILI9341_RED
    );
  }

  tft.setTextSize(2);

  tft.setTextColor(
    ILI9341_WHITE
  );

  tft.setCursor(
    85,
    108
  );

  tft.print(
    "HITS: "
  );

  tft.print(
    hitsThisRound
  );

  tft.print(
    "/10"
  );

  tft.setCursor(
    85,
    135
  );

  tft.print(
    "NEED: "
  );

  tft.print(
    needed
  );

  if (
    passed
  ) {

    soundRoundWin();

    centerText(
      "TAP NEXT ROUND",
      165,
      1,
      ILI9341_YELLOW
    );

    gameState =
      ROUND_OVER;

  } else {

    soundGameOver();

    centerText(
      "TAP TO RESTART",
      165,
      1,
      ILI9341_YELLOW
    );

    gameState =
      GAME_OVER;
  }
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(
    115200
  );

  Serial.println();
  Serial.println(
    "Duck Hunt V1.1 booting..."
  );

  // ===================================================
  // BUZZER
  // ===================================================

  ledcAttach(
    BUZZER_PIN,
    2000,
    8
  );

  soundOff();

  // ===================================================
  // BACKLIGHT
  // ===================================================

  pinMode(
    TFT_BL,
    OUTPUT
  );

  digitalWrite(
    TFT_BL,
    HIGH
  );

  // ===================================================
  // TFT
  // ===================================================

  tftSPI.begin(
    TFT_SCLK,
    TFT_MISO,
    TFT_MOSI,
    TFT_CS
  );

  tft.begin();

  tft.setRotation(
    1
  );

  tft.setTextWrap(
    false
  );

  Serial.println(
    "TFT OK"
  );

  // ===================================================
  // TOUCH
  // ===================================================

  touchSPI.begin(
    TOUCH_CLK,
    TOUCH_MISO,
    TOUCH_MOSI,
    TOUCH_CS
  );

  ts.begin(
    touchSPI
  );

  ts.setRotation(
    1
  );

  Serial.println(
    "Touch OK"
  );

  // ===================================================
  // HIGH SCORE
  // ===================================================

  preferences.begin(
    "duckhunt11",
    false
  );

  highScore =
    preferences.getLong(
      "high",
      0
    );

  // ===================================================
  // RANDOM
  // ===================================================

  randomSeed(
    micros()
  );

  // ===================================================
  // TITLE
  // ===================================================

  drawTitleScreen();

  Serial.println(
    "Title screen drawn"
  );

  startTitleMusic();

  Serial.println(
    "Duck Hunt V1.1 READY"
  );
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  unsigned long now =
    millis();

  // ===================================================
  // TITLE MUSIC
  // ===================================================

  if (
    gameState ==
    TITLE_SCREEN
  ) {

    updateTitleMusic();
  }

  // ===================================================
  // TOUCH
  // ===================================================

  int tapX;
  int tapY;

  if (
    getNewTap(
      tapX,
      tapY
    )
  ) {

    Serial.print(
      "TAP X="
    );

    Serial.print(
      tapX
    );

    Serial.print(
      " Y="
    );

    Serial.println(
      tapY
    );

    // =================================================
    // TITLE
    // =================================================

    if (
      gameState ==
      TITLE_SCREEN
    ) {

      stopTitleMusic();

      newGame();

      return;
    }

    // =================================================
    // NEXT ROUND
    // =================================================

    if (
      gameState ==
      ROUND_OVER
    ) {

      roundNumber++;

      soundRoundStart();

      startRound();

      return;
    }

    // =================================================
    // GAME OVER RESTART
    // =================================================

    if (
      gameState ==
      GAME_OVER
    ) {

      newGame();

      return;
    }

    // =================================================
    // FIRE
    // =================================================

    if (
      gameState ==
      DUCK_FLYING &&
      shotsLeft > 0
    ) {

      shotsLeft--;

      soundShot();

      drawHUD();

      if (
        duckHit(
          tapX,
          tapY
        )
      ) {

        hitDuckNow();

        return;
      }

      if (
        shotsLeft == 0
      ) {

        noAmmoSince =
          millis();
      }
    }
  }

  // ===================================================
  // STATE MACHINE
  // ===================================================

  switch (
    gameState
  ) {

    // =================================================
    // FLYING
    // =================================================

    case DUCK_FLYING:

      updateFlyingDuck();

      break;

    // =================================================
    // HIT FREEZE
    // =================================================

    case DUCK_HIT:

      if (
        now -
        stateStartedAt >=
        350
      ) {

        eraseDuck(
          (int)duckX,
          (int)duckY
        );

        soundFall();

        gameState =
          DUCK_FALLING;

        stateStartedAt =
          now;

        lastDuckFrame =
          now;

        oldDuckX =
          duckX;

        oldDuckY =
          duckY;
      }

      break;

    // =================================================
    // FALLING
    // =================================================

    case DUCK_FALLING:

      updateFallingDuck();

      break;

    // =================================================
    // DOG PICKUP
    // =================================================

    case DOG_PICKUP:

      if (
        now -
        stateStartedAt >=
        1400
      ) {

        startNextDuck();
      }

      break;

    // =================================================
    // DOG LAUGH
    // =================================================

    case DOG_LAUGH:

      if (
        now -
        stateStartedAt >=
        1500
      ) {

        startNextDuck();
      }

      break;

    default:

      break;
  }

  delay(2);
}
