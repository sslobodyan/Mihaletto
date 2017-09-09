#define PIR1_PIN 2 // куда подключены датчики
#define PIR2_PIN 3
#define PIR3_PIN 4

bool status1,status2,status3 = false;

void setup_pir();
bool check_pir1();
bool check_pir2();
bool check_pir3();

void setup_pir(){
  pinMode(PIR1_PIN, INPUT);
  pinMode(PIR2_PIN, INPUT);
  pinMode(PIR3_PIN, INPUT);
}

bool check_pir1(){
  if ( digitalRead( PIR1_PIN ) ) {
    if ( !status1 ) {
      status1 = true;
      return true;
    }
  } else status1 = false;    
  return false;
}

bool check_pir2(){
  if ( digitalRead( PIR2_PIN ) ) {
    if ( !status2 ) {
      status2 = true;
      return true;
    }
  } else status2 = false;    
  return false;
}

bool check_pir3(){
  if ( digitalRead( PIR3_PIN ) ) {
    if ( !status3 ) {
      status3 = true;
      return true;
    }
  } else status3 = false;    
  return false;
}

