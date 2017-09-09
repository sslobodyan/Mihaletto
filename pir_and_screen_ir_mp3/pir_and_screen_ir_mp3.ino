#include <TFT.h>
#include <SPI.h>
#include <IRremote.h>

#include "screen.h"
#include "mp3.h"
#include "pir.h"
#include "ir.h"

#define DEBUG(x) Serial.print(x);
#define DEBUGLN(x) Serial.println(x);

uint8_t mode, start_screen;
bool in_mode;
uint32_t time_ir, time_first_screen;
uint8_t track;  // номер трека

void setup(void) {
  mode = 0;
  in_mode = false;
  time_ir = 0;
  track = 0;
  Serial.begin(9600); // для отладки
  setup_screen();
  setup_mp3();
  setup_pir();
  setup_ir();
  
/*
Подаем питание на устройство. у нас 2 тфт экрана, светодиод, мп3 модуль от дфробот и ик пульт.
После влючения тфт экраны прогоняют 3 режима, которые у вас есть с делай 1000 между режимами и воспроизводится 1 трек.

Запускаем пиры с нашей последней программы и тфт экраны - у вас это уже готово.

Если сработала Кнопка пульта ‎456778890 - (ЗНАК ВВЕРХ) РЕЖИМЫ ЛИСТАЮТСЯ 1-2-3-4-5-6. 
Если кнопка ‎555676778 (ВНИЗ) РЕЖИМЫ ЛИСТАЮТСЯ ‎6-5-4-3-2-1-0. 
Если кнопка ‎8778877894 - вход в режим. 
Если кнопка ‎777666555 выход из режима.
Дефолтный уровень громкости 20. 
Если кнопка ‎555667788 - громкость вверх. 
Если кнопка ‎4754674466 громкость - вниз.
Кнопка ‎666556454 - выход из режима. и возврат к воспоизведению 1 трека.
Если в течении 30 секунд нет нажатия на кнопки то запускаем второй трек, если еще через 30 сек нет отзыва на кнопки запускаем 3 трек.
Во время воспроизведеня трека светодиод мирцает ( 400) от бизи плеера. если плеер молчит и диод выкл. - он привязан к плееру.
Когда мы листаем режимы то на одном экране мы "вместо функции" - выводим цифрой режим. как только заходим в режим снова выводим что у нас по функции.
*/
  track = 1;
  play(track);
  
  time_first_screen = millis();
  start_screen = 1;
  show1_1();
  show2_1();                
  DEBUG("FIRST Screen=");DEBUGLN(start_screen);

}

void loop(void) {
  if ( irrecv.decode( &results )) { // если данные пришли

    time_ir = millis();
    
    if (results.value == IR_VOL_UP) {
      mp3_volume_up();
      DEBUG("VOL_UP Volume=");DEBUGLN(volume);
    }
    
    if (results.value == IR_VOL_DOWN) {
      mp3_volume_down();
      DEBUG("VOL_DOWN Volume=");DEBUGLN(volume);
    }
    
    if (results.value == IR_UP){
      if ( !in_mode ) {
        if ( mode < 6 ) mode++;
        show_mode( mode );
      }
      DEBUG("UP Mode=");DEBUGLN(mode);
    }

    if (results.value == IR_DOWN){
      if ( !in_mode ) {
        if ( mode > 1 ) mode--;
        show_mode( mode );
      }
      DEBUG("DOWN Mode=");DEBUGLN(mode);
    }

    if (results.value == IR_MODE_IN){
      in_mode = true;
      DEBUGLN("IN");
      if (status1) show2_1(); // показываем функцию вместо номера режима на 2 экране
      else if (status2) show2_2();
      else if (status3) show2_3();
    }

    if (results.value == IR_MODE_OUT){
      in_mode = false;
      DEBUGLN("OUT");
      show_mode( mode ); // показываем номер режима вместо функции на 2 экране
    }

    if (results.value == IR_EXIT_TO_1){
      in_mode = false;
      track = 1;
      play(track);
      DEBUGLN("OUT to 1");
    }

    irrecv.resume(); // принимаем следующую команду
  }

  if ( time_ir && (millis() > time_ir+30000) ) { // 30 секунд пауза в нажатиях
    time_ir = millis();    
    if (track == 1) {
      track=2;
      play(track);
    }
    if (track == 2) {
      track=3;
      play(track);
    }
    DEBUG("Pause 30 sec Track=");DEBUGLN(track);
  }

  if ( !digitalRead(BUSY_MP3) ) { // идет воспроизведение - мигаем светодиодом
    if ( millis() % 800 > 400 ) LED_ON;
    else LED_OFF;  
  } else LED_OFF; 

  if (mode == 0) { // начальные экраны
    if ( millis() > time_first_screen+1000 ) {
      time_first_screen = millis();
      start_screen++;
      DEBUG("FIRST Screen=");DEBUGLN(start_screen);
      if ( start_screen==2 ) {
        show1_2();
        show2_2();          
      } else if ( start_screen==3 ) {
        show1_3();
        show2_3();          
      } else { // показали все экраны - идем в главный цикл
        mode = 1;      
      }
    }
  }

  if (mode) { // не в начальных экранах - контролируем пиры
    if ( check_pir1() ) {
        DEBUGLN("Pir1");
        show1_1();
        show2_1();                
    }
    if ( check_pir2() ) {
        DEBUGLN("Pir2");
        show1_2();
        show2_2();                
    }
    if ( check_pir3() ) {
        DEBUGLN("Pir3");
        show1_3();
        show2_3();                
    }
  }
  
}




