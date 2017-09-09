#define TFT_CS1     10  // куда подключен 1 экран
#define TFT_RST1    9  
#define TFT_DC1     8

#define TFT_CS2     5  // куда подключен 2 экран
#define TFT_RST2    6  
#define TFT_DC2     7

TFT screen1 = TFT(TFT_CS1, TFT_DC1, TFT_RST1);
TFT screen2 = TFT(TFT_CS2, TFT_DC2, TFT_RST2);


void setup_screen();

void show1_1();
void show1_2();
void show1_3();

void show2_1();
void show2_2();
void show2_3();

void show_mode( uint8_t mode );

void setup_screen() {
  screen1.begin();
  screen2.begin();
}

void show1_1(){
  // 1 режим
  screen1.background(255,255,255);
  screen1.stroke(0,0,70);
  screen1.fill(0,51,102);
  screen1.circle(screen1.width()/1.9, screen1.height()/2.1, 26);
  screen1.stroke(0,0,0);
  screen1.fill(0,0,0);
  screen1.circle(screen1.width()/1.9, screen1.height()/2.1, 12);
}

void show1_2(){
  // 2 режим
  screen1.background(255,255,255);
  screen1.stroke(0,0,70);
  screen1.fill(0,51,102);
  screen1.circle(screen1.width()/2.5, screen1.height()/2.1, 26);
  screen1.stroke(0,0,0);
  screen1.fill(0,0,0);
  screen1.circle(screen1.width()/2.5, screen1.height()/2.1, 12);
}

void show1_3(){
  // 3 режим
  screen1.background(255,255,255);
  screen1.stroke(0,0,70);
  screen1.fill(0,51,102);
  screen1.circle(screen1.width()/3.1, screen1.height()/2.1, 26);
  screen1.stroke(0,0,0);
  screen1.fill(0,0,0);
  screen1.circle(screen1.width()/3.1, screen1.height()/2.1, 12);  
}

///////////////////////// 2 экран ///////////////////////////////
void show2_1(){
  // 1 режим
  screen2.background(255,255,255);
  screen2.stroke(0,0,70);
  screen2.fill(120,51,102);
  screen2.circle(screen2.width()/1.9, screen2.height()/2.1, 26);
  screen2.stroke(0,0,0);
  screen2.fill(0,40,0);
  screen2.circle(screen2.width()/1.9, screen2.height()/2.1, 12);
}

void show2_2(){
  // 2 режим
  screen2.background(255,255,255);
  screen2.stroke(0,0,70);
  screen2.fill(120,51,102);
  screen2.circle(screen2.width()/2.5, screen2.height()/2.1, 26);
  screen2.stroke(0,0,0);
  screen2.fill(0,40,0);
  screen2.circle(screen2.width()/2.5, screen2.height()/2.1, 12);
}

void show2_3(){
  // 3 режим
  screen2.background(255,255,255);
  screen2.stroke(0,0,70);
  screen2.fill(120,51,102);
  screen2.circle(screen2.width()/3.1, screen2.height()/2.1, 26);
  screen2.stroke(0,0,0);
  screen2.fill(0,40,0);
  screen2.circle(screen2.width()/3.1, screen2.height()/2.1, 12);  
}

void show_mode( uint8_t mode ) {
  // показать на 2 экране номер режима
  char printout[4];
  String stmode = String(mode);
  stmode.toCharArray(printout,4);

  screen2.background(0,0,0);
  screen2.setTextSize(3);
  screen2.stroke(255,0,255);
  screen2.text(printout,0,30);
}

