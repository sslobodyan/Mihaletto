/*  
�����, ���������� �� �����
Arduino UNO & MotorMonster shield
(c) sslobodyan@ya.ru for Mihaletto
2017
*/

#define VERSION 0.13
//#define TEST_DRIVE  // ����������������� ��� ������ ��� ��������� ����������


#define LED 13
#define DEBUG Serial1  // �� ����� ���� ������ ������� ������
#define SPEED 30 // �������� �������� �� 1 �� 255, 0-������ �����
#define PERCENT_SPEED 12 // �� ������� ��������� ��������/�������������� ������ ��� �������� ������ ��� ������������
#define KOEF_DELTA 0.1f // ���� ��������� �� ����� 

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

#define LEFT 0 // ����� �����
#define RIGHT 1 // ������ �����

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
uint32_t time_led=0; // ����� ������� ���������� ������� �����������


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

///////////////////////////////////////////////////////////////////////

void setup()
{
  DEBUG.begin(115200);
  DEBUG.print("Robot  Version ");DEBUG.println(VERSION);
  
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

  get_sensors();
  motorStop();
	show_state();
  
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
            digitalWrite(inApin[mtr], HIGH);
            digitalWrite(inBpin[mtr], LOW);
            break;
      case CCW: 
            digitalWrite(inApin[mtr], LOW);
            digitalWrite(inBpin[mtr], HIGH);
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

void get_sensors() 
/* ��� ��� �������� - ���� ���� ������ */
{
  for (byte i=0; i<SENSOR_CNT; i++) {
    analog[i] = analogRead( sensorpin[i] );
  }

	if ( (analog[CENTER] >= analog[CENTER-1]) && (analog[CENTER] >= analog[CENTER+1]) ) {
		// ����������� ����� ������ ������ ��� ������ ������� - ���� �� �����
		sensor[CENTER] = BLACK;
		sensor[CENTER-2] = WHITE;
		sensor[CENTER+2] = WHITE;
	} else  { // ����� ����� �������� - ������� ���������� ������ ��������
		sensor[CENTER] = WHITE;
		if ( analog[CENTER-1] > analog[CENTER+1] ) {
			// ������ ����� ����� ������ ������ ��� ������ ������ - ����������� �����
			sensor[CENTER-2] = BLACK;
			sensor[CENTER+2] = WHITE;
		}
		if ( analog[CENTER-1] < analog[CENTER+1] ) {
			sensor[CENTER+2] = BLACK;
			sensor[CENTER-2] = WHITE;
		}
	}

	sensor[CENTER-1] = WHITE;
	sensor[CENTER+1] = WHITE;

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
			Delta = -f; // ������� ������
		}
		else {
			Delta = f; // ������� ����� ��� ����������
		}

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
  DEBUG.print(" Sensors: ");
  for (byte i=0; i<SENSOR_CNT; i++) {
    DEBUG.print( analog[i] );
    if ( sensor[i] == WHITE ) {
      DEBUG.print( "(W)" );
    }
    else DEBUG.print( "(B)" );
    DEBUG.print(" \t");
  }
  DEBUG.print(" > ");

  DEBUG.print(line_state);  DEBUG.print(" > ");

  
  switch (state) {
    case STOP:
          DEBUG.println("Stop");
          break;
    case FORWARD:
          DEBUG.println("VPERED");
          break;
    case TURN_LEFT:
          DEBUG.println("VLEVO");
          break;
    case TURN_RIGHT:
          DEBUG.println("VPRAVO");
          break;
    case Backward:
          DEBUG.println("NAZAD");
          break;
    default: ;
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

}
