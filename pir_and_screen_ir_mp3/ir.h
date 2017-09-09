#define RECV_PIN 23

#define IR_VOL_UP 555667788ul // vol_up
#define IR_VOL_DOWN 4754674466ul // vol_down

#define IR_UP 456778890 // (ЗНАК ВВЕРХ) РЕЖИМЫ ЛИСТАЮТСЯ 1-2-3-4-5-6
#define IR_DOWN 555676778 // (ВНИЗ) РЕЖИМЫ ЛИСТАЮТСЯ ‎6-5-4-3-2-1-0
#define IR_MODE_IN 8778877894 // вход в режим
#define IR_MODE_OUT 777666555 // выход из режима
#define IR_EXIT_TO_1 666556454 //выход из режима. и возврат к воспоизведению 1 трека

IRrecv irrecv(RECV_PIN); // указываем вывод, к которому подключен приемник
decode_results results;


void setup_ir(){
  irrecv.enableIRIn(); // запускаем прием
}

