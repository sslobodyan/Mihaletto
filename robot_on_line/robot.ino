/*  
CODEPAGE WIN-1251
�����, ���������� �� �����
Arduino MEGA2560-UNO & MotorMonster shield
(c) sslobodyan@ya.ru for Mihaletto
2017 
*/

#define VERSION 0.19
//#define TEST_DRIVE  // ����������������� ��� ������ ��� ��������� ����������
//#define DEBUG_DELTA // ����������������� ��� ������ ��� ������� ������������ �� ������ 

#define SERIAL_DEBUG	// ��������������� ��� ���������� ������ � ������ ���������� ����������

// ����� ������ (�������� ���� ������������) ���� �� ����� ��������� ������������ ���������
#define LEVO_PRAVO // ������������ �������
#define VPERED_NAZAD // ����������� �������� �������

#if defined(SERIAL_DEBUG) // �������� ����� ������� � ������ ������
	#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
		#define  DPRINT(...) Serial1.print(__VA_ARGS__)  // �� ����� ���� ������ ������� ������ ��� MEGA2560
		#define  DPRINTLN(...) Serial1.println(__VA_ARGS__)
		#define  DBEGIN(...) Serial1.begin(__VA_ARGS__)
	#else
		#define  DPRINT(...) Serial.print(__VA_ARGS__)  // �� ����� ���� ������ ������� ������ ��� UNO
		#define  DPRINTLN(...) Serial.println(__VA_ARGS__)
		#define  DBEGIN(...) Serial.begin(__VA_ARGS__)
	#endif
#else // ��������� ����� �� ������ ������� � ������
	#define  DPRINT(...) //
	#define  DPRINTLN(...) //
	#define  DBEGIN(...) //
#endif

#define LED 13

#define SPEED 30 // �������� �������� �� 1 �� 255, 0-������ �����
#define PERCENT_SPEED 12 // �� ������� ����������� ��������� ��������/�������������� ������ ��� �������� ������ ��� ������������
#define KOEF_DELTA 0.1f // ���� ��������� �� ����� 
#define TIME_CROSS_LED 500 // ���������� ������� ���������� ���������� �����������
#define START_TIME 50 // 5 ������ (�� 0.1���) �� ��������� ������ �� ����� ����� ���������

/* ������ �������� ������ */
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
	#define LEFT 1 // ����� �����
	#define RIGHT 0 // ������ �����
#else 
	#define LEFT 0 // ����� �����
	#define RIGHT 1 // ������ �����
#endif 

#define WHITE 0 // ����� ����
#define BLACK 1 // ������ �����

/* ����������� ����������� ����� ����������: ������� �����, ����� ������ ����� */
int inApin[2] = {7, 4}; // INA: �� �������
int inBpin[2] = {8, 9}; // INB: ������ �������
int pwmpin[2] = {5, 6}; // PWM: ��������

String line_state;
int Delta=0; // ������������ ��������� �����. <0 ������� �����, >0 ������� ������, 0=�������� ����������

#define SENSOR_CNT 5
#define CENTER 2 // ����������� ������

int sensorpin[SENSOR_CNT] = {A4, A3, A2, A1, A0}; // ������� ����� ����� ������� ���� �������� �� ������ ������
int ledpin[SENSOR_CNT] = {2,3,10,11,12}; // ���������� ���������� �������� (0-�4, 1-�3 � �.�.)

bool sensor[5]; // ���������� ������ � ��������
int analog[5]; // ���������� ������ � ��������

byte state; // ������� ����� ��������
byte test_mode;

int cross_cnt=0; // ������� ������������
uint32_t time_cross=0; // ����� ������� ���������� ������� ����������� � "������ �����"
bool cross=false; // ������� �� ����������� - ����� �� ������ � ������� ������� �� ����� 0.9 �� ������

/* ��������� ������� */
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
  
  // ��� ����������� �� �����
  for (int i=0; i<2; i++)
  {
    pinMode(inApin[i], OUTPUT);
    pinMode(inBpin[i], OUTPUT);
    pinMode(pwmpin[i], OUTPUT);
  }
  
  // ��� ���������� �� ����� � ��������
  for (int i=0; i<SENSOR_CNT; i++)
  {
    pinMode(ledpin[i], OUTPUT);
		digitalWrite(ledpin[i], WHITE);
		sensor[i] = WHITE;
  }

	// ��������� ������� ��� ��� ������� ���������� ��������� 
	setPwmFrequency(pwmpin[0], 8);  // 16000000 / 256 / 2 / 8 = 3,9KHz
	setPwmFrequency(pwmpin[1], 8);  // ���������� ����� �������� 64 -> 490 ��

  motorStop();
	show_state();

	DPRINT("Waiting "); 
	for (byte i=0; i<START_TIME; i++) {
		delay(100);
	  DPRINT(".");
		get_sensors(); // ���������� ��� ������� ����� �����
		digitalWrite( LED, i%2 );
	}
	DPRINTLN(" Let's GO !!!");
	digitalWrite( LED, LOW );

}

void motorStop()
/*
  ��������� � ������������� � �����
*/
{
  motor(LEFT, BRAKEGND, 0);
  motor(RIGHT, BRAKEGND, 0);
  state = STOP;
	Delta = 0;
}

void motorFORWARD()
/*
  ��������� ������
*/
{
  motor(LEFT, CW, SPEED+Delta);
  motor(RIGHT, CW, SPEED-Delta);
  state = FORWARD;
}

void motorBackward()
/*
  ��������� �����
*/
{
  motor(LEFT, CCW, SPEED);
  motor(RIGHT, CCW, SPEED);
  state = Backward;
	Delta = 0;
}

void motorLeft()
/*
  ������������ �����
*/
{
  motor(LEFT, BRAKEGND, 0);
  motor(RIGHT, CW, SPEED);
  state = TURN_LEFT;
	Delta = 0;
}

void motorRight()
/*
  ������������ �������
*/
{
  motor(LEFT, CW, SPEED);
  motor(RIGHT, BRAKEGND, 0);
  state = TURN_RIGHT;
	Delta = 0;
}


void motor(uint8_t mtr, uint8_t mode, uint8_t spd)
/* ���������� ��������� �������
 mtr: ����� ������
 mode: ����� ������ ������
     BRAKEVCC:  ���� � �������
     CW:        �� �������
     CCW:       ������ �������
     BRAKEGND:  ���� � �����
 spd: �������� 0-255
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
/* ������������� ������� ��� �� ������ ����, �������� �������� 1,8,64,256,1024 */
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
/* ��� ��� �������� - ���� ���� ������ */
{
  for (byte i=0; i<SENSOR_CNT; i++) {
    analog[i] = analogRead( sensorpin[i] );
		sensor[i] = WHITE; // ��������� ��� ����������
  }

	if ( (analog[CENTER] >= analog[CENTER-1]) && (analog[CENTER] >= analog[CENTER+1]) ) {
		// ����������� ����� ������ ������ ��� ������ ������� - ���� �� �����
		sensor[CENTER] = BLACK;
		test_on_cross();
	} else  { // ����� ����� �������� (����� ������ ����� ����) - ������� ���������� ������ ��������
		if ( analog[CENTER-1] > analog[CENTER+1] ) {
			// ������ ����� ����� ������ ������ ��� ������ ������ - ����������� �����
			sensor[CENTER-2] = BLACK;
		}
		if ( analog[CENTER-1] < analog[CENTER+1] ) {
			sensor[CENTER+2] = BLACK;
		}
	}

	if ( analog[CENTER-1] > analog[CENTER+1] ) {
		// ������ ����� ����� ������ ������ ��� ������ ������ - ����������� �����
		sensor[CENTER-1] = BLACK;
	}
	if ( analog[CENTER-1] < analog[CENTER+1] ) {
		// ������ ������ ����� ������ ������ ��� ������ ����� - ����������� ������
		sensor[CENTER+1] = BLACK;
	}

  for (byte i=0; i<SENSOR_CNT; i++) {	
		digitalWrite (ledpin[i], sensor[i]); // ���������� ������� � ������� (������ - ������ �����)
	}
}

void test_drive() 
/* ���� �������� */
{
	delay(1000);
	test_mode = 1; // ������������� ������ ������ ������� (��������� ����� ������, ������ �����)
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
/* ��������� �������� �� ����� */
{
  if ( sensor[CENTER] == BLACK ) { // ���� �� ������
		// ���������� ������������ ��������� ���, ����� 1 � 3 ������� �������� ���������� ��������, �� ���� ���� ����� �� ������ 
		int err = analog[CENTER-1] - analog[CENTER+1];
		const int max_delta_speed = (int) SPEED * PERCENT_SPEED / 100; // �������� ��������� ������������
		float f = (float) KOEF_DELTA * abs(err);
		f = constrain( f, 0, max_delta_speed);
		if (err < 0) { // ������ ������ 
			Delta = f; // ������� ������
		}
		else {
			Delta = -f; // ������� ����� ��� ����������
		}

#ifdef  DEBUG_DELTA
		  DPRINT("Delta=");   DPRINT(Delta);
		  DPRINT(" Left=");   DPRINT(SPEED+Delta);
		  DPRINT(" Right=");   DPRINTLN(SPEED-Delta);
#endif

    motorFORWARD(); // ���� �����
    line_state = "On line ";
  } 
  else { // ����������� ������ ������� ������
    // ��������� ������ ����
    for (byte i=CENTER+1; i<SENSOR_CNT; i++) {
      if ( sensor[i] == BLACK ) {
        motorRight(); // ������������ ������
        line_state = "Line on right ";
        return ;
      }
    }
    // ��������� ����� ����
    for (byte i=0; i<CENTER; i++) {
      if ( sensor[i] == BLACK ) {
        motorLeft(); // ������������ �����
        line_state = "Line on left ";
        return ;
      }
    }
    // �� ���� ������ �� ����� ������
    motorStop();
    line_state = "Line loose! ";
  }
}

void show_state() 
// ������� ��������� �������� � ����������� ��������
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
// ����� �����������
	if ( time_cross ) return; // ���� ������ �����
	if ( (analog[CENTER-2] >= (float) 0.9*analog[CENTER]) && (analog[CENTER+2] >= (float) 0.9*analog[CENTER]) ) {
		if ( !cross ) { // ��� �� ���� �� �����������
			cross = true;
			time_cross = millis() + TIME_CROSS_LED;
			digitalWrite( LED, HIGH );
 	    DPRINTLN("Cross start");
		} 
	} else {
		if ( cross ) { // �������� � �����������
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
