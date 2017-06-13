/*  
CODEPAGE WIN-1251
Робот, движущийся по линии
Arduino MEGA2560-UNO & MotorMonster shield
(c) sslobodyan@ya.ru for Mihaletto
2017 
*/

#define VERSION 0.19
//#define TEST_DRIVE  // раскомментировать эту строку для включения тестдрайва
//#define DEBUG_DELTA // раскомментировать эту строку для отладки подруливания на прямой 

#define SERIAL_DEBUG	// закоментировать для отключения вывода в сериал отладочной информации

// здесь менять (оставить либо закомментить) пока не будет правильно отрабатывать тестдрайв
#define LEVO_PRAVO // расположение моторов
#define VPERED_NAZAD // направление вращения моторов

#if defined(SERIAL_DEBUG) // включить вывод отладки в нужный сериал
	#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
		#define  DPRINT(...) Serial1.print(__VA_ARGS__)  // на какой порт пойдет отладка команд для MEGA2560
		#define  DPRINTLN(...) Serial1.println(__VA_ARGS__)
		#define  DBEGIN(...) Serial1.begin(__VA_ARGS__)
	#else
		#define  DPRINT(...) Serial.print(__VA_ARGS__)  // на какой порт пойдет отладка команд для UNO
		#define  DPRINTLN(...) Serial.println(__VA_ARGS__)
		#define  DBEGIN(...) Serial.begin(__VA_ARGS__)
	#endif
#else // Отключаем вывод не нужной отладки в сериал
	#define  DPRINT(...) //
	#define  DPRINTLN(...) //
	#define  DBEGIN(...) //
#endif

#define LED 13

#define SPEED 30 // скорость движения от 1 до 255, 0-всегда стоим
#define PERCENT_SPEED 12 // на сколько максимально процентов ускорять/подтормаживать колесо при движении вперед для подруливания
#define KOEF_DELTA 0.1f // надо подобрать по факту 
#define TIME_CROSS_LED 500 // милисекунд горения светодиода найденного перекрестка
#define START_TIME 50 // 5 секунд (по 0.1сек) на установку робота на линию после включения

/* режимы движения робота */
#define FORWARD 0
#define TURN_LEFT 1
#define TURN_RIGHT 2
#define STOP 3
#define Backward 4

#define BRAKEVCC 0
#define CW   1
#define CCW  2
#define BRAKEGND 3

#ifdef LEVO_PRAVO
	#define LEFT 1 // левый мотор
	#define RIGHT 0 // правый мотор
#else 
	#define LEFT 0 // левый мотор
	#define RIGHT 1 // правый мотор
#endif 

#define WHITE 0 // белое поле
#define BLACK 1 // черная линия

/* Определение подключения пинов моторшилда: сначала ЛЕВЫЙ, затем ПРАВЫЙ мотор */
int inApin[2] = {7, 4}; // INA: по часовой
int inBpin[2] = {8, 9}; // INB: против часовой
int pwmpin[2] = {5, 6}; // PWM: скорость

String line_state;
int Delta=0; // дифференциал скоростей колес. <0 доворот влево, >0 доворот вправо, 0=скорости одинаковые

#define SENSOR_CNT 5
#define CENTER 2 // центральный сенсор

int sensorpin[SENSOR_CNT] = {A4, A3, A2, A1, A0}; // датчики линии СЛЕВА НАПРАВО если смотреть на робота СВЕРХУ
int ledpin[SENSOR_CNT] = {2,3,10,11,12}; // светодиоды аналогично датчикам (0-А4, 1-А3 и т.д.)

bool sensor[5]; // логические уровни с датчиков
int analog[5]; // аналоговые уровни с датчиков

byte state; // текущий режим движения
byte test_mode;

int cross_cnt=0; // счетчик перекрестков
uint32_t time_cross=0; // время горения светодиода наличия перекрестка и "слепое время"
bool cross=false; // наехали на перекресток - центр на полосе и крайние боковые не менее 0.9 от центра

/* Прототипы функций */
void motorStop(); 
void motorLeft();
void motorRight();
void motorBackward();
void motorFORWARD();
void motor(uint8_t mtr, uint8_t mode, uint8_t spd);
void get_sensors();
void autopilote();
void show_state();
void test_on_cross();
void setPwmFrequency(int pin, int divisor);

///////////////////////////////////////////////////////////////////////

void setup()
{
    DBEGIN(115200);

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    DPRINT("Robot MEGA2560  Version ");  DPRINTLN(VERSION);
#else
    DPRINT("Robot UNO  Version ");  DPRINTLN(VERSION);
#endif
  
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  
  // все управляющие на вывод
  for (int i=0; i<2; i++)
  {
    pinMode(inApin[i], OUTPUT);
    pinMode(inBpin[i], OUTPUT);
    pinMode(pwmpin[i], OUTPUT);
  }
  
  // все светодиоды на вывод и погасить
  for (int i=0; i<SENSOR_CNT; i++)
  {
    pinMode(ledpin[i], OUTPUT);
		digitalWrite(ledpin[i], WHITE);
		sensor[i] = WHITE;
  }

	// установим частоту ШИМ для выводов управления скоростью 
	setPwmFrequency(pwmpin[0], 8);  // 16000000 / 256 / 2 / 8 = 3,9KHz
	setPwmFrequency(pwmpin[1], 8);  // стандартно стоит делитель 64 -> 490 Гц

  motorStop();
	show_state();

	DPRINT("Waiting "); 
	for (byte i=0; i<START_TIME; i++) {
		delay(100);
	  DPRINT(".");
		get_sensors(); // показываем как датчики видят линию
		digitalWrite( LED, i%2 );
	}
	DPRINTLN(" Let's GO !!!");
	digitalWrite( LED, LOW );

}

void motorStop()
/*
  Остановка с блокированием к земле
*/
{
  motor(LEFT, BRAKEGND, 0);
  motor(RIGHT, BRAKEGND, 0);
  state = STOP;
	Delta = 0;
}

void motorFORWARD()
/*
  Двигаемся вперед
*/
{
  motor(LEFT, CW, SPEED+Delta);
  motor(RIGHT, CW, SPEED-Delta);
  state = FORWARD;
}

void motorBackward()
/*
  Двигаемся назад
*/
{
  motor(LEFT, CCW, SPEED);
  motor(RIGHT, CCW, SPEED);
  state = Backward;
	Delta = 0;
}

void motorLeft()
/*
  Поворачиваем влево
*/
{
  motor(LEFT, BRAKEGND, 0);
  motor(RIGHT, CW, SPEED);
  state = TURN_LEFT;
	Delta = 0;
}

void motorRight()
/*
  Поворачиваем направо
*/
{
  motor(LEFT, CW, SPEED);
  motor(RIGHT, BRAKEGND, 0);
  state = TURN_RIGHT;
	Delta = 0;
}


void motor(uint8_t mtr, uint8_t mode, uint8_t spd)
/* Управление отдельным мотором
 mtr: номер мотора
 mode: режим работы мотора
     BRAKEVCC:  стоп к питанию
     CW:        по часовой
     CCW:       против часовой
     BRAKEGND:  стоп к земле
 spd: скорость 0-255
*/
{
  if (mtr <= 1) {
    switch (mode) {
      case BRAKEVCC: 
            digitalWrite(inApin[mtr], HIGH);
            digitalWrite(inBpin[mtr], HIGH);
            break;
      case CW: 
						#ifdef VPERED_NAZAD
							digitalWrite(inApin[mtr], HIGH);
							digitalWrite(inBpin[mtr], LOW);
						#else
							digitalWrite(inApin[mtr], LOW);
							digitalWrite(inBpin[mtr], HIGH);
						#endif
            break;
      case CCW: 
						#ifdef VPERED_NAZAD
							digitalWrite(inApin[mtr], LOW);
							digitalWrite(inBpin[mtr], HIGH);
						#else
							digitalWrite(inApin[mtr], HIGH);
							digitalWrite(inBpin[mtr], LOW);
						#endif
            break;
      case BRAKEGND: 
            digitalWrite(inApin[mtr], LOW);
            digitalWrite(inBpin[mtr], LOW);
            break;
      default: ;      
    }
  }
  analogWrite(pwmpin[mtr], spd);
}

void setPwmFrequency(int pin, int divisor) {
/* устанавливаем частоту ШИМ на нужном пине, указывая делитель 1,8,64,256,1024 */
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
		switch (pin) {
			case 5: TCCR3B = TCCR3B & 0b11111000 | mode; break;
			case 6: TCCR4B = TCCR4B & 0b11111000 | mode; break;
			case 9: ;
			case 10: TCCR2B = TCCR2B & 0b11111000 | mode; break;
		}
#else
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
#endif
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x07; break;
      default: return;
    }
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
		switch (pin) {
			case 3: TCCR3B = TCCR3B & 0b11111000 | mode; break;
			case 11: TCCR1B = TCCR1B & 0b11111000 | mode; break;
		}
#else
		TCCR2B = TCCR2B & 0b11111000 | mode; 
#endif
  }
}

void get_sensors() 
/* что под датчиком - поле либо полоса */
{
  for (byte i=0; i<SENSOR_CNT; i++) {
    analog[i] = analogRead( sensorpin[i] );
		sensor[i] = WHITE; // выключаем все светодиоды
  }

	if ( (analog[CENTER] >= analog[CENTER-1]) && (analog[CENTER] >= analog[CENTER+1]) ) {
		// центральный видит темнее полосу чем первые боковые - идем по линии
		sensor[CENTER] = BLACK;
		test_on_cross();
	} else  { // линия резко свернула (центр увидел белое поле) - поворот показываем самыми крайними
		if ( analog[CENTER-1] > analog[CENTER+1] ) {
			// первый левый видит темнее полосу чем первый правый - отклонились влево
			sensor[CENTER-2] = BLACK;
		}
		if ( analog[CENTER-1] < analog[CENTER+1] ) {
			sensor[CENTER+2] = BLACK;
		}
	}

	if ( analog[CENTER-1] > analog[CENTER+1] ) {
		// первый левый видит темнее полосу чем первый правый - отклонились влево
		sensor[CENTER-1] = BLACK;
	}
	if ( analog[CENTER-1] < analog[CENTER+1] ) {
		// первый правый видит темнее полосу чем первый левый - отклонились вправо
		sensor[CENTER+1] = BLACK;
	}

  for (byte i=0; i<SENSOR_CNT; i++) {	
		digitalWrite (ledpin[i], sensor[i]); // индицируем уровень с сенсора (светит - черная линия)
	}
}

void test_drive() 
/* тест движения */
{
	delay(1000);
	test_mode = 1; // принудительно делаем правый поворот (вращается левое колесо, правое стоит)
	switch (test_mode) {
		case 0: motorFORWARD(); break;
		case 1: motorRight(); break;
		case 2: motorLeft(); break;
		case 3: motorStop(); break;
		case 4: motorBackward(); break;
	}
	if (test_mode) test_mode-=1;
	else test_mode=4;
}

void autopilote() 
/* автопилот движения по линии */
{
  if ( sensor[CENTER] == BLACK ) { // идем по полосе
		// определяем дифференциал скоростей так, чтобы 1 и 3 датчики отдавали одинаковое значение, то есть идем точно по центру 
		int err = analog[CENTER-1] - analog[CENTER+1];
		const int max_delta_speed = (int) SPEED * PERCENT_SPEED / 100; // максимум процентов дифференциал
		float f = (float) KOEF_DELTA * abs(err);
		f = constrain( f, 0, max_delta_speed);
		if (err < 0) { // справа темнее 
			Delta = f; // доворот вправо
		}
		else {
			Delta = -f; // доворот влево или прекратить
		}

#ifdef  DEBUG_DELTA
		  DPRINT("Delta=");   DPRINT(Delta);
		  DPRINT(" Left=");   DPRINT(SPEED+Delta);
		  DPRINT(" Right=");   DPRINTLN(SPEED-Delta);
#endif

    motorFORWARD(); // идем прямо
    line_state = "On line ";
  } 
  else { // центральный датчик потерял полосу
    // проверяем правый борт
    for (byte i=CENTER+1; i<SENSOR_CNT; i++) {
      if ( sensor[i] == BLACK ) {
        motorRight(); // поворачиваем вправо
        line_state = "Line on right ";
        return ;
      }
    }
    // проверяем левый борт
    for (byte i=0; i<CENTER; i++) {
      if ( sensor[i] == BLACK ) {
        motorLeft(); // поворачиваем влево
        line_state = "Line on left ";
        return ;
      }
    }
    // ни один датчик не видит полосу
    motorStop();
    line_state = "Line loose! ";
  }
}

void show_state() 
// отладка состояния сенсоров и направления движения
{
  DPRINT(" Sensors: ");
  for (byte i=0; i<SENSOR_CNT; i++) {
    DPRINT( analog[i] );
    if ( sensor[i] == WHITE ) {
      DPRINT( "(W)" );
    }
    else DPRINT( "(B)" );
    DPRINT(" \t");
  }
  DPRINT(" > "); DPRINT(line_state); DPRINT(" > ");

  switch (state) {
    case STOP:
          DPRINTLN("Stop");
          break;
    case FORWARD:
          DPRINTLN("VPERED");
          break;
    case TURN_LEFT:
          DPRINTLN("VLEVO");
          break;
    case TURN_RIGHT:
          DPRINTLN("VPRAVO");
          break;
    case Backward:
          DPRINTLN("NAZAD");
          break;
    default: ;
  }
}

void test_on_cross() {
// поиск перекрестка
	if ( time_cross ) return; // идет слепое время
	if ( (analog[CENTER-2] >= (float) 0.9*analog[CENTER]) && (analog[CENTER+2] >= (float) 0.9*analog[CENTER]) ) {
		if ( !cross ) { // еще не были на перекрестке
			cross = true;
			time_cross = millis() + TIME_CROSS_LED;
			digitalWrite( LED, HIGH );
 	    DPRINTLN("Cross start");
		} 
	} else {
		if ( cross ) { // съезжаем с перекрестка
			cross = false;
			cross_cnt += 1;
			DPRINT("Cross finish"); DPRINT(" > CROSS_CNT="); DPRINTLN( cross_cnt );
		}
	}
}

////////////////////////////////////////////////////////////////////


void loop()
{

  get_sensors();
  byte old_state = state;
  
#ifdef TEST_DRIVE
  test_drive();
#else  
  autopilote();
#endif  

  if ( old_state != state ) {
    show_state();
  }

	if ( time_cross ) {
		if ( millis() > time_cross ) {
			time_cross = 0;
			digitalWrite ( LED, LOW );
		}
	}
}
