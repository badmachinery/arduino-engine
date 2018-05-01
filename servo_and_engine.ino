#include "PWM.h"
#include <Wire.h>

int signals_sleep[2] = {200, 1000}; //задержка индикатора как в азбуке морзе короткий/длинный 0/1

//включение индикатора подключенного через пин - pin_num и передача сигнала без прерывания работы проограммы
void led_on_without_interruption(int _signal, byte pin_num){
  pinMode(pin_num, OUTPUT);
  int _bin[3];
  for(int i=2; i>=0; --i){
    _bin[i] = _signal%2;
    _signal/=2;
  }
  for(int i=0; i<3; ++i){
    digitalWrite(pin_num, HIGH);
    delay(signals_sleep[_bin[i]]);
    digitalWrite(pin_num, LOW);
    delay(200);    
    _signal/=2;
  }
  delay(1800);
}

//зацикливание мерцания индикатора на случай если мы не хотим продолжать работу программы
//допустим по pin_from пришел некий сигнал и до тех пор пока на обозреваемый pin_from подается напряжение
//индикатор будет о чем-то сигнализировать
void led_on_with_interruption(int _signal, byte pin_num){
  pinMode(pin_num, OUTPUT);
  while(true){
    led_on_without_interruption(_signal,pin_num);
  }
}


//нейтраль и её крайние положения
int neutral = 7200; //7445, 6965
int sp = neutral, rt = neutral;
int myservo = 9;
int engine = 10;

int32_t servo_frequency = 75;
int32_t engine_frequency = 600;

//диапазон значений частот servo в миллисекундах для pwmWrite
//1.02+-0.2 - abs left
//1.02 - left 20
//1.75 +-0.2 - abs right
//1.78 - right 35

//engine в микросекундах
//forward = 1916
//med = 1500
//backward = 1100

int time_in = millis();

int readVal(char &check){
  //digitalWrite(LED_BUILTIN, LOW);
  int tmp = Serial.read();
  Serial.print(2);
  while(tmp!='s' && tmp!='r' && tmp!='b'){
   tmp = Serial.read();
   if(millis()-time_in>200){
    sp = 7200;
    rt = 7200;  
    pwmWriteHR(engine,sp);
    pwmWriteHR(myservo,rt);
    Serial.print(3);
    return -1;
   }
  }
  check = tmp;
  tmp = Serial.parseInt();
  return tmp;
}

int s_fl_time = millis();
int fl_bool = 1;
int on = 0;
int _time; 
int time_for_reset;
int last_sp;
int last_sp_t;
int stop_reg = 1;

void setup(){
  Serial.begin(115200);
  InitTimersSafe();
  bool succes = SetPinFrequencySafe(engine,servo_frequency);
  succes = SetPinFrequencySafe(myservo,servo_frequency);
  pwmWriteHR(myservo, neutral);
  pwmWriteHR(engine, neutral);
  Wire.begin();
  _time = millis();
  last_sp = neutral;
  last_sp_t = millis();
  pinMode(13,OUTPUT);
}

void loop(){
    if(millis()-time_in>200){
      sp = 7200;
      rt = 7200;
      pwmWriteHR(myservo, rt);
      pwmWriteHR(engine, sp);
    }
    if ((millis()-_time) > 1000 ) {
      //Serial.end();
      //Serial.begin(9600);
      if(on) {
         digitalWrite(LED_BUILTIN, LOW);
         on = 0;
      }
      else {
        digitalWrite(LED_BUILTIN, HIGH);
        on = 1;
      }
      _time = millis();
    }
    if (Serial.available() > 0){
      _time = millis();
      time_in = millis();
      char check;
      int tmp = readVal(check);
      if(check == 's' && tmp!=-1) 
        sp = tmp;
      else 
        if (check == 'r' && tmp!=-1)
          rt = tmp;
      else 
        if (check == 'b' && tmp!=-1){
          pwmWriteHR(engine, neutral);
          delay(10);
          pwmWriteHR(engine, 7200-tmp);
          delay(40);
        }  
      pwmWriteHR(myservo, rt);
      pwmWriteHR(engine, sp);
      Serial.print(1);
      
      /*
      Serial.print("S: ");
      Serial.print(sp);
      Serial.print(" R: ");
      Serial.println(rt);
      */
         
    }
    Serial.print(1);
}
