#define SerialP Serial2 // пин 16 через резистор 1 КОм к RX плеера
#define BUSY_MP3 12     // ножка BUSY занятость мп3 (0-играет, 1-молчит)
#define LED_MP3  22     // ножка LED 

#define PLAY  0x03
#define VOL   0x06
#define STOP  0x16

#define LED_ON digitalWrite(LED_MP3,HIGH)
#define LED_OFF digitalWrite(LED_MP3,LOW)

byte buff[10] = {0x7E, 0xFF, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF};
uint8_t volume;

void setup_mp3();
void mp3_cmd (byte com, byte atr);
void playToEnd (byte np);
void play (byte np);
void playIfEnd (byte np);
void playWaitEnd (byte np);
void stopPlay ();
void mp3_check (uint8_t *buf);
void mp3_volume_up();
void mp3_volume_down();

void setup_mp3(){
  SerialP.begin(9600);
  pinMode(BUSY_MP3, INPUT);
  volume = 20;
  pinMode(LED_MP3, OUTPUT);
  LED_OFF;
}

void playToEnd (byte np) {  // проигрываем запись номер nm и ждем пока она закончится
  play (np);
  while (!digitalRead(BUSY_MP3));
}

void play (byte np) {     // проигрываем запись номер nm в любой момент
  mp3_cmd(PLAY, np);
  delay(100);
}

void playIfEnd (byte np) {  // проигрываем запись номер nm если закончилась предыдущая
  if (!digitalRead(BUSY_MP3)) return;
  play (np);
}

void playWaitEnd (byte np) {  // ждем когда закончится предыдущая и проигрываем запись номер nm
  while (!digitalRead(BUSY_MP3));
  play (np);
}

void stopPlay () {
  mp3_cmd(STOP, 0);
  delay(100);
}

void mp3_cmd (byte com, byte atr) {  // команда на плеер
  buff[3] = com;
  buff[6] = atr;
  mp3_check(buff);
  for (byte i = 0; i < 10; i++) {
    SerialP.write(buff[i]);
  }
}

void mp3_check (uint8_t *buf) {      // КС для плеера
  uint16_t sum = 0;
  for (byte i = 1; i < 7; i++) {
    sum += buf[i];
  }
  sum = -sum;
  *(buf + 7) = (uint8_t)(sum >> 8);
  *(buf + 8) = (uint8_t)sum;
}

void mp3_volume_up(){
  if (++volume > 30) volume=30; 
  mp3_cmd(VOL, volume);
}

void mp3_volume_down(){
  if (volume) volume--;
  mp3_cmd(VOL, volume);  
}

