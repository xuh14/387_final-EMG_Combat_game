#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <TouchScreen.h>
#define MINPRESSURE 200
#define MAXPRESSURE 1000

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// ALL Touch panels and wiring is DIFFERENT
// copy-paste results from TouchScreen_Calibr_native.ino
const int XP = 7, XM = A1, YP = A2, YM = 6; //ID=0x9341
//const int TS_LEFT = 925, TS_RT = 207, TS_TOP = 195, TS_BOT = 950; // portrait
const int TS_LEFT = 207, TS_RT = 925, TS_TOP = 195, TS_BOT = 950; // landscape
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

Adafruit_GFX_Button player1_life, player2_life, combat1, combat2;

int pixel_x, pixel_y;     //Touch_getXY() updates global vars
int life1, life2, combat1_val, combat2_val;
int combat_max = 290;
int life_max = 90;

// frame settings
int life_frame_width = 100;
int life_frame_height = 40;
int combat_frame_width = 250;
int combat_frame_height = 40;
int frame_width = 20;

// counter
float current_timer, start_timer, end_timer;
bool time_start = 0;
int ran_low = 3;
int ran_high = 5;
int counter_xpos = 160 - 20;
int counter_ypos = frame_width + 10;

int n_of_player = 2;

// combat zone
int zone_width, zone_width2;
int zone_pos, zone_pos2;

bool receive_flag = 0;
int muscle_level = 0;

int left_bar1, right_bar1, left_bar2, right_bar2;
bool life1_lost_flag, life2_lost_flag;
int life1_lost_amount, life2_lost_amount;
float life1_lost_timer, life2_lost_timer;
int in_box1, in_box2;
bool in_box1_start, in_box2_start;
float hold1_timer, hold2_timer, current_hold1_timer, current_hold2_timer;
float restart_timer = 0;

int win = 0;

void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600);
  uint16_t ID = tft.readID();
  Serial.print("TFT ID = 0x");
  Serial.println(ID, HEX);
  tft.begin(ID);
  tft.setRotation(1);            //landscape
  tft.fillScreen(BLACK);
  init_page();
  draw_frame();
  draw_life();
  draw_combat();
  //tft.fillRect(0, 0, 80, 50, RED);
  start_timer = micros();
}

void init_page() {
  tft.setCursor (50, 20);
  tft.setTextSize (2);
  tft.setTextColor(RED);
  tft.print("Welcome to ROOM 101");

  tft.setCursor (50, 60);
  tft.setTextSize (1);
  tft.setTextColor(WHITE);
  tft.print("1. Each player has 100 life");
  tft.setCursor (50, 80);
  tft.print("2. Each player need to muscle up ");
  tft.setCursor (55, 100);
  tft.print("to the bar before timer ends");
  tft.setCursor (50, 120);
  tft.print("3. if fails to do so, player will lose life");
  tft.setCursor (50, 140);
  tft.print("4. Player who stands the last wins the game");
  tft.setCursor (140, 170);
  tft.setTextSize (2);
  tft.setTextColor(MAGENTA);
  tft.print("Enjoy");
  tft.setCursor (100, 195);
  tft.setTextSize (1);
  tft.print("Press screen to proceed");

  // wait for a press
  while (true) {
    bool down = Touch_getXY();
    if (down) {
      break;
    }
  }
  tft.fillScreen(BLACK);
}

void draw_frame() {
  tft.fillRect(0, 0, 320, frame_width, RED);
  tft.fillRect(0, 240 - frame_width, 320, frame_width, RED);
  tft.fillRect(0, 0, frame_width, 240, RED);
  tft.fillRect(320 - frame_width, 0, frame_width, 240, RED);

  // draw life frame
  tft.drawRect(5 + frame_width, 5 + frame_width, life_frame_width, life_frame_height, WHITE);
  if (n_of_player == 2) {
    tft.drawRect(320 - life_frame_width - 5 - frame_width, 5 + frame_width, life_frame_width, life_frame_height, WHITE);
  }
  // draw combat frame
  if (n_of_player == 2) {
    tft.drawRect(160 - combat_frame_width / 2, 240 - combat_frame_height - 20 - combat_frame_height - frame_width, combat_frame_width, combat_frame_height, WHITE);
  }
  tft.drawRect(160 - combat_frame_width / 2, 240 - combat_frame_height - 10 - frame_width, combat_frame_width, combat_frame_height, WHITE);

  // draw text
  tft.setCursor (5 + frame_width + life_frame_width / 2 - 10, 5);
  tft.setTextSize (1);
  tft.setTextColor(WHITE);
  tft.println("life1");
  if (n_of_player == 2) {
    tft.setCursor (320 - life_frame_width - 5 - frame_width + life_frame_width / 2 - 10, 5);
    tft.setTextSize (1);
    tft.setTextColor(WHITE);
    tft.println("life2");
  }
  tft.setCursor (160 - combat_frame_width / 2 + combat_frame_width / 2 - 10, 240 - 15);
  tft.setTextSize (1);
  tft.setTextColor(WHITE);
  tft.println("combat");

  // init life
  life1 = 100;
  life2 = 100;
}

bool Touch_getXY(void)
{
  TSPoint p = ts.getPoint();
  pinMode(YP, OUTPUT);
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH);
  digitalWrite(XM, HIGH);
  bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
  if (pressed) {
    pixel_x = map(p.y, TS_LEFT, TS_RT, 0, 320);
    pixel_y = map(p.x, TS_TOP, TS_BOT, 0, 240);
  }
  return pressed;
}

void clear_life() {
  int perc_life1 = life_frame_width;
  int perc_life2 = life_frame_width;
  tft.fillRect(5 + frame_width + 2, 5 + frame_width + 1, perc_life1 - 4, life_frame_height - 2, BLACK);
  if (n_of_player == 2) {
    tft.fillRect(320 - life_frame_width - 5 - frame_width + 2, 5 + frame_width + 1, perc_life2 - 4, life_frame_height - 2, BLACK);
  }
}

void update_life() {
  clear_life();
  // update life
  int perc_life1 = map(life1, 0, 100, 0, life_frame_width);
  int perc_life2 = map(life2, 0, 100, 0, life_frame_width);
  tft.fillRect(5 + frame_width + 2, 5 + frame_width + 1, perc_life1 - 4, life_frame_height - 2, RED);
  if (n_of_player == 2) {
    tft.fillRect(320 - life_frame_width - 5 - frame_width + 2, 5 + frame_width + 1, perc_life2 - 4, life_frame_height - 2, RED);
  }

  if (life1 <= 0) {
    win = 2;
    game_over_text();
  } else if (life2 <= 0) {
    win = 1;
    game_over_text();
  }
}

void game_over_text() {
  if (win == 1) {
    tft.setCursor (100, 50);
    tft.setTextSize (2);
    tft.setTextColor(BLUE);
    tft.print("GAME OVER!");
    tft.setCursor (80, 80);
    tft.setTextColor(YELLOW);
    tft.print("Player1 wins!");
  } else if (win == 2) {
    tft.setCursor (100, 50);
    tft.setTextSize (2);
    tft.setTextColor(BLUE);
    tft.print("GAME OVER!");
    tft.setCursor (80, 80);
    tft.setTextColor(YELLOW);
    tft.print("Player2 wins!");
  }
  
  delay(2000);
  tft.setCursor (100, 195);
  tft.setTextSize (1);
  tft.print("Press screen to restart");
  // wait for a press
  while (true) {
    bool down = Touch_getXY();
    if (down) {
      break;
    }
  }
  
  life1 = 100;
  life2 = 100;
  tft.setTextColor(BLACK);
  if (win == 1) {
    tft.setCursor (100, 50);
    tft.setTextSize (2);
    tft.print("GAME OVER!");
    tft.setCursor (80, 80);
    tft.print("Player1 wins!");
  } else if (win == 2) {
    tft.setCursor (100, 50);
    tft.setTextSize (2);
    tft.print("GAME OVER!");
    tft.setCursor (80, 80);
    tft.print("Player2 wins!");
  }
  tft.setCursor (100, 195);
  tft.setTextSize (1);
  tft.print("Press screen to restart");
  win = 0;
}

void draw_combat() {
  /**
    tft.fillRect(5, 1, life1, 40, RED);
    tft.fillRect(220, 1, life2, 40, RED);*/
}

void draw_life() {
  //player1_life.initButton(&tft,  50, 20, 100, 50, WHITE, BLACK, WHITE, "", 1);
  //player2_life.initButton(&tft, 265, 20, 100, 50, WHITE, BLACK, WHITE, "", 1);
  //player1_life.drawButton(false);
  // player2_life.drawButton(false);
}

void update_combat() {
  left_bar1 = map(combat1_val, 0, 100, 160, 160 - combat_frame_width / 2);
  right_bar1 = map(combat1_val, 0, 100, 160, 160 - combat_frame_width / 2 + combat_frame_width);
  left_bar2 = map(combat2_val, 0, 100, 160, 160 - combat_frame_width / 2);
  right_bar2 = map(combat2_val, 0, 100, 160, 160 - combat_frame_width / 2 + combat_frame_width);
  if (n_of_player == 2) {
    tft.fillRect(left_bar2, 240 - combat_frame_height - 20 - combat_frame_height - frame_width + 2, 160 - left_bar2, combat_frame_height - 4, RED);
    tft.fillRect(160, 240 - combat_frame_height - 20 - combat_frame_height - frame_width + 2, right_bar2 - 160, combat_frame_height - 4, RED);
  }
  tft.fillRect(left_bar1, 240 - combat_frame_height - 10 - frame_width + 2, 160 - left_bar1, combat_frame_height - 4, RED);
  tft.fillRect(160, 240 - combat_frame_height - 10 - frame_width + 2, right_bar1 - 160, combat_frame_height - 4, RED);
}

void draw_counter() {
  // clear timer
  tft.setCursor (counter_xpos, counter_ypos);
  tft.setTextSize (2);
  tft.setTextColor(BLACK);
  tft.println(String(floor(10 * current_timer) / 10));

  current_timer = (end_timer - micros()) / 1000000;

  if (time_start == 0) {
    current_timer = 0;
  }
  if (current_timer < 0.01 && time_start == 1) {
    current_timer = 0;
    time_start = 0;
    time_up();
  }
  // update timer
  tft.setCursor (counter_xpos, counter_ypos);
  tft.setTextSize (2);
  tft.setTextColor(WHITE);
  tft.println(String(floor(10 * current_timer) / 10));
}

void time_up() {
  clear_combat_zone();
  in_box1_start = 0; // stop the timer
  in_box2_start = 0;

  // compare player hold time

  // if hold time < 0.3 s, lose 5 life
  // else, who holds the least time, lose 3 life. 
  if (current_hold2_timer < 0.3) {
    life2_lost_amount = 10;
    life2_lost_flag = 1;
  } else if (current_hold2_timer < current_hold1_timer) {
      life2_lost_amount = 5;
      life2_lost_flag = 1;
  }
  if (current_hold1_timer < 0.3) {
    life1_lost_amount = 10;
    life1_lost_flag = 1;
  } else if (current_hold1_timer < current_hold2_timer) {
      life1_lost_amount = 5;
      life1_lost_flag = 1;
  }
  restart_timer = micros() + 2 * 1000000;
  //delay(1000);
}

void draw_combat_zone() {
  zone_width = random(40, 70);
  zone_pos = random(160 - combat_frame_width / 2 + 5, 160 - zone_width);
  zone_width2 = random(40, 70);
  zone_pos2 = random(160 - combat_frame_width / 2 + 5, 160 - zone_width);
  if (n_of_player == 2) {
    tft.drawRect(zone_pos2, 240 - combat_frame_height - 20 - combat_frame_height - frame_width + 1, zone_width2, combat_frame_height - 2, CYAN);
    tft.drawRect(320 - zone_pos2 - zone_width2, 240 - combat_frame_height - 20 - combat_frame_height - frame_width + 1, zone_width2, combat_frame_height - 2, CYAN);
  }
  tft.drawRect(zone_pos, 240 - combat_frame_height - 10 - frame_width + 1, zone_width, combat_frame_height - 2, MAGENTA);
  tft.drawRect(320 - zone_pos - zone_width, 240 - combat_frame_height - 10 - frame_width + 1, zone_width, combat_frame_height - 2, MAGENTA);
}

void clear_combat_zone() {
  if (n_of_player == 2) {
    tft.drawRect(zone_pos2, 240 - combat_frame_height - 20 - combat_frame_height - frame_width + 1, zone_width2, combat_frame_height - 2, BLACK);
    tft.drawRect(320 - zone_pos2 - zone_width2, 240 - combat_frame_height - 20 - combat_frame_height - frame_width + 1, zone_width2, combat_frame_height - 2, BLACK);
  }
  tft.drawRect(zone_pos, 240 - combat_frame_height - 10 - frame_width + 1, zone_width, combat_frame_height - 2, BLACK);
  tft.drawRect(320 - zone_pos - zone_width, 240 - combat_frame_height - 10 - frame_width + 1, zone_width, combat_frame_height - 2, BLACK);
}

void life_lost() {
  clear_life_lost();
  if (life1_lost_flag) {
    tft.setCursor (5 + frame_width + life_frame_width / 2 - 10, 80);
    tft.setTextSize (2);
    tft.setTextColor(WHITE);
    tft.print("-"); tft.println(life1_lost_amount);
    life1_lost_timer = micros();
    life1_lost_flag = 0;
    life1 = life1 - life1_lost_amount;
  }
  if (life2_lost_flag) {
    tft.setCursor (320 - life_frame_width - 5 - frame_width + life_frame_width / 2 - 10, 80);
    tft.setTextSize (2);
    tft.setTextColor(WHITE);
    tft.print("-"); tft.println(life2_lost_amount);
    life2_lost_timer = micros();
    life2_lost_flag = 0;
    life2 = life2 - life2_lost_amount;
  }
}

void clear_life_lost() {
  if (micros() - life1_lost_timer >  2000000) {
    tft.setCursor (5 + frame_width + life_frame_width / 2 - 10, 80);
    tft.setTextSize (2);
    tft.setTextColor(BLACK);
    tft.print("-"); tft.println(life1_lost_amount);
  }
  if (micros() - life2_lost_timer >  2000000) {
    tft.setCursor (320 - life_frame_width - 5 - frame_width + life_frame_width / 2 - 10, 80);
    tft.setTextSize (2);
    tft.setTextColor(BLACK);
    tft.print("-"); tft.println(life2_lost_amount);
  }
}

int check_in_box1() {
  if (left_bar1 <= zone_pos + zone_width && left_bar1 >= zone_pos) {
    return 1; // in box
  } else if (left_bar1 < zone_pos) {
    return 2; // exceed box;
  } else {
    return 3; // not reached
  }
}

int check_in_box2() {
  if (left_bar2 <= zone_pos2 + zone_width2 && left_bar2 >= zone_pos2) {
    return 1; // in box
  } else if (left_bar2 < zone_pos2) {
    return 2; // exceed box;
  } else {
    return 3; // not reached
  }
}

void clear_hold_timer1() {
  // clear timer
    tft.setCursor (50, 100);
    tft.setTextSize (1);
    tft.setTextColor(BLACK);
    tft.println(String(floor(10 * current_hold1_timer) / 10));
}

void clear_hold_timer2() {
  // clear timer
    tft.setCursor (230, 100);
    tft.setTextSize (1);
    tft.setTextColor(BLACK);
    tft.println(String(floor(10 * current_hold2_timer) / 10));
}

void draw_hold_counter() {
  if (in_box1_start == 1) {
    clear_hold_timer1();
    if (in_box1 == 1) {
      current_hold1_timer = (micros() - hold1_timer) / 1000000;
      Serial.println(current_hold1_timer);
    } else {
      hold1_timer = micros() - current_hold1_timer*1000000; // pause time
    }
    // update timer
    tft.setCursor (50, 100);
    tft.setTextSize (1);
    tft.setTextColor(MAGENTA);
    tft.println(String(floor(10 * current_hold1_timer) / 10));
  }
  // box2
  if (in_box2_start == 1) {
    clear_hold_timer2();
    if (in_box2 == 1) {
      current_hold2_timer = (micros() - hold2_timer) / 1000000;
      Serial.println(current_hold2_timer);
    } else {
      hold2_timer = micros() - current_hold2_timer*1000000; // pause time
    }
    // update timer
    tft.setCursor (230, 100);
    tft.setTextSize (1);
    tft.setTextColor(CYAN);
    tft.println(String(floor(10 * current_hold2_timer) / 10));
  }
}

void reset_hold() {
  clear_hold_timer1();
  clear_hold_timer2();
  current_hold1_timer = 0;
  current_hold2_timer = 0;
}


void loop(void)
{
  bool down = Touch_getXY();
  
  if (micros() > restart_timer && (time_start == 0)) {
    end_timer = micros() + 1000000 * random(ran_low, ran_high);
    time_start = 1;
    clear_combat_zone();
    draw_combat_zone();
    reset_hold();
  }
  draw_counter();

  if (Serial1.available() >= 2) {
    receive_flag = 1;
    muscle_level = Serial1.read() * 256 + Serial.read();
  } else {
    receive_flag = 0;
  }
  if (receive_flag = 1) {
    Serial.print("Percentage: "); Serial.print(muscle_level); Serial.println("%");
  }
  // constantly update combat zone incase overlap
  if (n_of_player == 2) {
    tft.drawRect(zone_pos2, 240 - combat_frame_height - 20 - combat_frame_height - frame_width + 1, zone_width2, combat_frame_height - 2, CYAN);
    tft.drawRect(320 - zone_pos2 - zone_width2, 240 - combat_frame_height - 20 - combat_frame_height - frame_width + 1, zone_width2, combat_frame_height - 2, CYAN);
  }
  tft.drawRect(zone_pos, 240 - combat_frame_height - 10 - frame_width + 1, zone_width, combat_frame_height - 2, MAGENTA);
  tft.drawRect(320 - zone_pos - zone_width, 240 - combat_frame_height - 10 - frame_width + 1, zone_width, combat_frame_height - 2, MAGENTA);


  combat1_val = 20;
  combat2_val = 30;
  update_combat();
  update_life();
  in_box1 = check_in_box1();
  in_box2 = check_in_box2();
  if (in_box1 == 1 && in_box1_start == 0 && time_start != 0) { // if in the box
    in_box1_start = 1;
    hold1_timer = micros();
  }
  if (in_box2 == 1 && in_box2_start == 0 && time_start != 0) { // if in the box
    in_box2_start = 1;
    hold2_timer = micros();
  }
  draw_hold_counter();
  life_lost();
  /**
    life1 = life_max;
    life2 = life_max;
    combat1_val = combat_max;
    combat2_val = 0;


    update_life();
    update_combat();
    tft.setCursor (35, 50);
    tft.setTextSize (1);
    tft.setTextColor(WHITE);
    tft.println("life1");

    tft.setCursor (250, 50);
    tft.setTextSize (1);
    tft.setTextColor(WHITE);
    tft.println("life2");

    tft.setCursor (130, 85);
    tft.setTextSize (2);
    tft.setTextColor(WHITE);
    tft.println("combat");
  */
}
