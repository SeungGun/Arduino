/* Gachon University Robotics  Team : Let's Be  "Smart Tea Machine" */
#include <LiquidCrystal_I2C.h>  // LCD I2C 라이브러리 
#include <Wire.h>// I2C 통신 헤더파일
#include <DallasTemperature.h>  //온도 센서용 라이브러리
#include <OneWire.h> // 디바이스와 통신 헤더파일
#include <Servo.h> // 서보모터 라이브러리

#define ONE_WIRE_BUS 51 //DS18B20 온도센서 51번 핀

Servo servo[8]; // 서보모터 객체배열 생성
LiquidCrystal_I2C lcd(0x27, 16, 2); // 16x2 lcd 사용
OneWire oneWire(ONE_WIRE_BUS); // oneWire로 디바이스와 통신
DallasTemperature sensors(&oneWire); //oneWire선언한 것을 sensors 선언시 참조


#define init_time 1  //티백모드 기본초기시간 1분 설정
#define buzzer 25 // 부저 25번핀
#define cup_trig 27 // 컵의 유무 판단하는 초음파센서 trig 27번핀
#define cup_echo 26 // 컵의 유무 판단하는 초음파센서 echo 26번핀

#define nor 13 // DC모터 정회전 PWM핀 13번핀
#define rev 12 // DC모터 역회전 PWM핀 12번핀 

#define teabag_btn 22 //티백모드 시작버튼 22번핀
#define add_time 23 //시간추가 버튼 23번핀
#define mix_btn 24 //믹스모드 시작버튼 24번핀
#define water_btn 31 // 물양설정 버튼 31번핀
#define tmp_btn 32 // 온도설정 버튼 32번핀
#define checking 34 // 설정한 값 확인용 버튼 34번핀 

int pin[8] = {2, 3, 4, 5, 6, 7, 8, 9}; //서보모터 PWM핀들의 배열
/* 2,3,4,5는 Teabagging Mode 용
   6,7,8,9는 Mix Mode용
   각각 순서대로 하단부, 중심부, 중심부, 상단부
*/
int sec = 0; //초
int minu = init_time;  //분 (초기값은 init_time으로 할당)
int t_time;  // 총 시간
int count = 0; //시간초 지날때 몇번 60초가 지났는지 세는 변수 (sec값을 0~60으로 맞추기 위함)
unsigned long start = 0;  //티백모드 타이머 변수
unsigned long timer = 0;  //믹스모드 타이머 변수

int basic_angle = 80; //서보모터의 초기값 각도
int basic_mix = 20; // 기본 믹스모드 타이머 시간값(n초)
int cvt; // 믹스모드와 티백모드 구분용 변수 믹스모드 : 1 티백모드 : 2
int j, k, a, b; // 서보모터 동작 딜레이주기 위한 for문 각도 변수
unsigned long dis; // 초음파센서로 측정한 거리값 저장할 변수

int water = 100; // 기본 물양 100ml
int leds[6] = {38, 39, 40, 41, 42, 43}; // 물양에 따른 led 핀 배열

float temp = 0.0;  //온도센서로 가질값 초기화
int userTemp = 70; //기본 온도 70도로 설정

int check = 0; //상태 전환
/* 1일 때 : 물의 양 설정
  2일 때 : 물의 양에 따른 led 켜기
  4일 때 : 접근 불가 상태
*/
int check2 = 0; // 상태 전환
/* 1일 때 : 온도 설정
  2일 때 : 온도 측정
  3일 때 : 시간 설정
  4일 때 : 모터작동
*/
boolean dis_cvt = true; // 거리측정 판단 변수

int set_time = 0; //시간설정 되었는지 판단
int set_temp = 0; // 온도설정 되었는지 판단
int set_water = 0; // 물양설정 되었는지 판단
unsigned long distance; // Distance() 함수 리턴값 받는 거리 값

void setup() {
  analogWrite(nor, 0); //dc모터 정회전 값 0으로 초기화
  analogWrite(rev, 0); //dc모터 역회전 값 0으로 초기화

  Serial.begin(9600); // 시리얼 통신
  lcd.init(); // lcd 시작
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Welcome!");
  lcd.setCursor(0, 1);
  lcd.print("We are STM");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set condition");
  lcd.setCursor(0, 1);
  lcd.print("Water,Time,Temp");

  for (int i = 0; i < 6; i++) {
    //led 핀들 출력모드로 설정
    pinMode(leds[i], OUTPUT);
  }
  pinMode(nor, OUTPUT); // 정회전 출력모드
  pinMode(rev, OUTPUT); // 역회전 출력모드
  pinMode(buzzer, OUTPUT); // 부저 출력모드
  pinMode(cup_trig, OUTPUT); // 초음파 쏘기 출력모드
  pinMode(cup_echo, INPUT); // 초음파 받기 입력모드

  for (int i = 0; i < 8; i++) { //서보모터 배열 연결
    servo[i].attach(pin[i]);
  }

  servo[3].write(90); // 상단부 리셋
  servo[7].write(90); // 상단부 리셋

  // 아래 if문 기본 각도 설정
  if (servo[1].read() > basic_angle) {
    for (int i = servo[1].read(); i > basic_angle; i--) {
      servo[1].write(i);
      delay(100);
    }
  }
  else {
    for (int i = servo[1].read(); i < basic_angle; i++) {
      servo[1].write(i);
      delay(100);
    }
  }
  if (servo[2].read() > basic_angle) {
    for (int i = servo[2].read(); i > basic_angle; i--) {
      servo[2].write(i);
      delay(100);
    }
  }
  else {
    for (int i = servo[2].read(); i < basic_angle; i++) {
      servo[2].write(i);
      delay(100);
    }
  }

}

void loop() {
  if (dis_cvt) {
    distance = Distance(); // 거리측정 함수에서 받은 거리값 저장
    Serial.println(distance);
  }
  if (distance < 20) { // 컵과의 거리가 20cm이하라면
    dis_cvt = false; // 거리함수 못쓰도록 상태 변환
    int mixstate = digitalRead(mix_btn); // 믹스모드 시작 버튼을 누른 상태값 저장 눌렀으면 mixstate의 값 판단하는 if문으로 이동
    if (digitalRead(teabag_btn) == HIGH) { // 티백모드 버튼
      check = 1;  // check상태 1로 전환 -> 물양 설정으로 가기위한
      lcd.clear(); //lcd 초기화
      delay(1000);
    }
    while (check == 1) { //check가 1인 동안, 물양버튼
      lcd.setCursor(0, 0);
      lcd.print("Set the water");
      lcd.setCursor(0, 1);
      lcd.print(water); //물의 양 값 lcd로 출력
      lcd.print("ml");

      if (digitalRead(water_btn) == HIGH) { // 물양버튼 누른다면
        water += 10;  // 물의 양 값 10씩 증가
        while (digitalRead(water_btn) == HIGH); //물양 버튼 누르는 동안만
      }
      if (water > 200) { // 물양 값이 200이 초과하면
        water = 100; //다시 물의 양값을 100으로 리셋
      }
      if (digitalRead(checking) == HIGH) { // 확인버튼을 누른다면
        check = 2;  //check상태 2로 전환 -> 물의양에 따른 LED 켜기
        check2 = 1; // check2상태 1로 전환 -> 온도 설정
        delay(500);
        lcd.clear(); //lcd 초기화
        delay(100);
        set_water = 1; //물의양 설정완료 확인변수 -> 값 1로
      }
    }
    while (check == 2) { // check 상태가 2인 동안에만
      /* 물의 양에 따른 led (led는 컵의 높이를 의미) */
      if (water <= 100) {  //물의양이 100ml 이하라면
        digitalWrite(leds[0], HIGH);
        digitalWrite(leds[1], LOW);
        digitalWrite(leds[2], LOW);
        digitalWrite(leds[3], LOW);
        digitalWrite(leds[4], LOW);
        digitalWrite(leds[5], LOW);
      }
      else if (water > 100 && water < 120) { //물의 양이 100에서 120 사이면
        digitalWrite(leds[0], HIGH);
        digitalWrite(leds[1], HIGH);
        digitalWrite(leds[2], LOW);
        digitalWrite(leds[3], LOW);
        digitalWrite(leds[4], LOW);
        digitalWrite(leds[5], LOW);
      }
      else if (water == 120) { // 물의 양이 120이면
        digitalWrite(leds[0], LOW);
        digitalWrite(leds[1], HIGH);
        digitalWrite(leds[2], LOW);
        digitalWrite(leds[3], LOW);
        digitalWrite(leds[4], LOW);
        digitalWrite(leds[5], LOW);
      }
      else if (water > 120 && water < 140) { //물의양이 120에서 140 사이라면
        digitalWrite(leds[0], LOW);
        digitalWrite(leds[1], HIGH);
        digitalWrite(leds[2], HIGH);
        digitalWrite(leds[3], LOW);
        digitalWrite(leds[4], LOW);
        digitalWrite(leds[5], LOW);
      }
      else if (water == 140) { // 물의 양이 140이면
        digitalWrite(leds[0], LOW);
        digitalWrite(leds[1], LOW);
        digitalWrite(leds[2], HIGH);
        digitalWrite(leds[3], LOW);
        digitalWrite(leds[4], LOW);
        digitalWrite(leds[5], LOW);
      }
      else if (water > 140 && water < 160) { // 물의양이 140에서 160이면
        digitalWrite(leds[0], LOW);
        digitalWrite(leds[1], LOW);
        digitalWrite(leds[2], HIGH);
        digitalWrite(leds[3], HIGH);
        digitalWrite(leds[4], LOW);
        digitalWrite(leds[5], LOW);
      }
      else if (water == 160) { // 물의 양이 160이면
        digitalWrite(leds[0], LOW);
        digitalWrite(leds[1], LOW);
        digitalWrite(leds[2], LOW);
        digitalWrite(leds[3], HIGH);
        digitalWrite(leds[4], LOW);
        digitalWrite(leds[5], LOW);
      }
      else if (water > 160 && water < 180) { // 물의양이 160에서 180이면
        digitalWrite(leds[0], LOW);
        digitalWrite(leds[1], LOW);
        digitalWrite(leds[2], LOW);
        digitalWrite(leds[3], HIGH);
        digitalWrite(leds[4], HIGH);
        digitalWrite(leds[5], LOW);
      }
      else if ( water == 180) { // 물의양이 180이면
        digitalWrite(leds[0], LOW);
        digitalWrite(leds[1], LOW);
        digitalWrite(leds[2], LOW);
        digitalWrite(leds[3], LOW);
        digitalWrite(leds[4], HIGH);
        digitalWrite(leds[5], LOW);
      }
      else if (water > 180 && water < 200) { // 물의양이 180에서 200이면
        digitalWrite(leds[0], LOW);
        digitalWrite(leds[1], LOW);
        digitalWrite(leds[2], LOW);
        digitalWrite(leds[3], LOW);
        digitalWrite(leds[4], HIGH);
        digitalWrite(leds[5], HIGH);
      }
      else if (water == 200) {  // 물의양이 200이면
        digitalWrite(leds[0], LOW);
        digitalWrite(leds[1], LOW);
        digitalWrite(leds[2], LOW);
        digitalWrite(leds[3], LOW);
        digitalWrite(leds[4], LOW);
        digitalWrite(leds[5], HIGH);
      }

      while (check2 == 1) { //check2 가 1인 동안만
        if (digitalRead(checking) == HIGH) { //확인 버튼 누른다면
          check2 = 2;  // check2 상태 2 전환 ->  시간 설정
          delay(1000);
          lcd.clear();
          set_temp = 1; // 온도설정 완료 1로 값 변경
        }
        else {
          lcd.setCursor(0, 0);
          lcd.print("Set Temperature");
          lcd.setCursor(0, 1);
          lcd.print(userTemp);
          if (digitalRead(tmp_btn) == HIGH) { // 온도 설정 버튼을 누르면
            userTemp += 5; // 5도씩 증가
            if (userTemp > 100) { // 온도가 100 넘어 가면
              userTemp = 70; // 다시 70으로 재설정
              lcd.setCursor(2, 1);
              lcd.print(" ");
            }
          }
          delay(100);
        }
        delay(100);
      }
      while (check2 == 2) { //check2 상태가 2인 동안만
        lcd.setCursor(0, 0);
        lcd.print("Set the time");
        lcd.setCursor(0, 1);
        lcd.print("Time ");
        lcd.setCursor(5, 1);
        lcd.print(minu);
        lcd.print(":");
        lcd.print(sec);

        if (digitalRead(add_time) == HIGH) { // 시간 추가 버튼은 누른다면
          sec += 10; // 10초씩 증가
          if (sec == 60) { //sec가 60이면 -> 60초가 되면은
            minu++; // 1분추가
            sec = 0; // sec 0으로 초기화
          }
          lcd.setCursor(5, 1);
          lcd.print(minu);
          lcd.print(":");
          lcd.print(sec);
          while (digitalRead(add_time) == HIGH); // 시간추가 버튼을 누르고 있는 동안에만
        }
        if (digitalRead(checking) == HIGH) { // 확인 버튼을 누른다면
          lcd.clear();
          delay(500);
          set_time = 1; // 시간설정 완료
          check = 4; // check 상태 4로 전환 -> 아무 모드도 아닌상태
          check2 = 3; // check2 상태 3으로 전환 -> 온도측정
        }
      }

      while (check2 == 3) { //check2가 3인 동안만
        sensors.requestTemperatures();  //온도측정 요청
        temp = sensors.getTempCByIndex(0) + 20; //온도측정하고 값 temp에 저장
        Serial.println(temp);

        if (temp <= userTemp) { //설정한 온도가 측정한 온도와 같아지는 순간(측정 온도는 점점 줄기 때문에 <=)
          check2 = 4; // check2 상태 4로 전환 -> 모터 작동
          lcd.clear();
          break; // 끝내기
        }
        lcd.setCursor(0, 1);
        lcd.print("Curr Temp:");
        lcd.print(temp);
        delay(100);
      }
    }

    /*믹스모드---------------------------------------------------------------------------------------------------*/

    if (mixstate == HIGH) { //믹스모드
      cvt = 1; // 서보모터 동작함수에서 믹스모드 전환

      down_midSer(); // 중심부 상승
      delay(1000);
      down_topSer(); // 상단부 하강
      delay(1500);
      up_midSer();  // 중심부 하강
      delay(1500);
      servo[7].write(25); // 컵안으로 안정적으로 넣기 위해 상단부 서보모터 앞으로 이동
      delay(1000);
      Start_Mix(); // DC모터 작동해서 믹싱 시작

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Mix mode");
      lcd.setCursor(0, 1);
      lcd.print("Time:");
      lcd.print(basic_mix);
      lcd.print(" / ");

      while (timer != basic_mix) { //타이머 시작 basic_mix 시간까지
        timer++;
        delay(1000);
        lcd.setCursor(10, 1);
        lcd.print(timer);
      }
      if (timer == basic_mix) {  // 타이머가 끝났다면
        Stop_Mix(); // DC모터 동작 중지
        delay(500);
        down_midSer(); // 중심부 상승
        delay(1000);
        up_topSer(); // 상단부 상승
        delay(1000);
        music(); // 동작 종료후 알람
        timer = 0; // 타이머 초기화
      }
      mixstate = LOW; // 버튼 상태 초기화
      lcd.clear();
    }
    /*Teabag Mode-------------------------------------------------------------------------------------------*/
    int teastate = digitalRead(teabag_btn); // 티백모드 상태

    if (check == 4 && set_time == 1 && set_temp == 1 && set_water == 1) { // 시간, 온도, 물의양 조건 맞춘뒤 티백모드 시작 버튼 누른다면
      cvt = 2; // 티백모드로 전환

       up_midSer(); // 중심부 상승
      delay(2000);
      up_topSer(); // 상단부 상승
      delay(3000);
      down_midSer(); // 중심부 하강
      delay(2000);
      teabag_down(); // 상단부 하강

      t_time = minu * 60 + sec;  //설정한 총 시간초 초단위로 저장
      // Serial.println(t_time);
      int m = minu;  int s = sec; // 초와 분을 출력용과 실제 타이머 돌릴 때 사용하기 위해 새로운 변수에 분,초 값 저장
      minu = 0; sec = 0; //타이머 돌리기위해 다 0값으로 초기화

      lcd.setCursor(0, 0);
      lcd.print(userTemp);
      lcd.print("C ");
      lcd.print(water);
      lcd.print("ml");

      lcd.setCursor(0, 1);
      lcd.print("Time:");
      lcd.setCursor(5, 1);
      lcd.print(m); //지정한 분 출력
      lcd.print(":");
      lcd.print(s); // 지정한 초 출력
      lcd.print("/");

      while (start != t_time) {  //설정한 시간까지 타이머 시작
        start++; // millis() 함수 로직 모순으로 대체
        delay(1000);
        sec = start - (60 * count); // sec값을 0~60으로 유지 -> 딱 초만을 출력하기 위함
        if (minu > 0 && sec < 10) { //타이머가 분이 추가될때 lcd에서 쓸데없는 값 지우기 위함
          lcd.setCursor(13, 1);
          lcd.print(" ");
        }
        lcd.setCursor(10, 1); lcd.print(minu); lcd.print(":"); lcd.print(sec);

        if (start % 60 == 0) { //타이머가 60초가 될때마다 -> 분으로 넘어갈때
          minu++; count++; // 분 추가, 카운트 추가
        }
      }
      if (start == t_time) { //타이머가 설정한 시간까지 다되면
        start = 0; //타이머 초기화
        t_time = 0; // 시간설정 초기화
        minu = 1; // 분 초기값 1로 초기화
        sec = 0; // 초 초기화
        count = 0; // 카운트 초기화
        set_time = 0; // 시간확인 변수 초기화
        set_temp = 0; // 온도확인 변수 초기화
        set_water = 0; // 물양확인 변수 초기화

        up_midSer(); // 중심부 상승
        delay(2000);
        up_topSer();
        delay(1000);
        music(); // 종료후 울리는 알람
      }
      teastate = LOW; // 버튼상태 초기화
      check = 0; // 다시 사용을 위한 check 상태 초기화
      check2 = 0; // 다시 사용을 위한 check2 상태 초기화
      water = 100; // 물양 기본값으로 초기화
      userTemp = 70; // 온도값 기본값으로 초기화
    }
    else if (teastate == HIGH) { // 시간,온도,물양 설정 안한상태에서 티백모드 시작버튼 눌렀다면
      check = 0;  // 다시 사용을 위한 check 상태 초기화
      check2 = 0; // 다시 사용을 위한 check2 상태 초기화
      teastate = LOW; // 버튼상태 초기화
    }

  }
  else { // 컵이 없다면
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set the Cup!"); // 컵을 두라고 알림
    dis_cvt = true; // 거리측정 판단변수 초기화
    delay(2000);
  }
}

/* 중심부 서보모터가 하강하는 함수 */
void down_midSer() {
  if (cvt == 2) { // 티백 모드일때
    for (j = servo[1].read(), k = servo[2].read(); j < 100; j++, k--) { // 두 서보모터가 마주보고 있어 하나는 각도 증가, 하나는 각도 감소로 균형 맞추기
      servo[1].write(j); //100 중심-우측 3번핀
      servo[2].write(k); //60 중심-좌측  4번핀
      delay(15);
    }
  }
  else if (cvt == 1) { // 믹스 모드일때
    for (j = servo[5].read(), k = servo[6].read(); j < 100; j++, k--) { //두 서보모터가 마주보고 있어 하나는 각도 증가, 하나는 각도 감소로 균형 맞추기
      servo[5].write(j);
      servo[6].write(k);
      delay(15);
    }
  }
}
/* 중심부 서보모터가 상승하는 함수 */
void up_midSer() {
  if (cvt == 2) { // 티백 모드일때
    for (a = servo[1].read(), b = servo[2].read(); a > 40; a--, b++) { //두 서보모터가 마주보고 있어 하나는 각도 증가, 하나는 각도 감소로 균형 맞추기
      servo[1].write(a); //40
      servo[2].write(b); //120
      delay(15);
    }
  }
  else if (cvt == 1) { // 믹스 모드일때
    for (a = servo[5].read(), b = servo[6].read(); a > 40; a--, b++) { //두 서보모터가 마주보고 있어 하나는 각도 증가, 하나는 각도 감소로 균형 맞추기
      if (a == 60) {
        servo[7].write(20);
      }
      servo[5].write(a); //40
      servo[6].write(b); //120
      delay(15);
    }
  }
}
/* 상단부 서보모터가 상승하는 함수 */
void up_topSer() {
  if (cvt == 2) { //티백 모드일때
    for (int i = servo[3].read(); i < 90; i++) {// 조건 각도가 높아질수록 더 올라감
      servo[3].write(i);
      delay(15);
    }
  }
  else if (cvt == 1) {// 믹스 모드일때
    for (int i = servo[7].read(); i < 80; i++) {// 조건 각도가 높아질수록 더 올라감
      servo[7].write(i);
      delay(15);
    }
  }
}
/* 상단부 서보모터가 하강하는 함수 */
void down_topSer() {
  if (cvt == 2) { // 티백 모드일때
    for (int i = servo[3].read(); i > 0; i--) {
      servo[3].write(i);
      delay(15);
    }
  }
  else if (cvt == 1) {// 믹스 모드일때
    for (int i = servo[7].read(); i > 0; i--) {
      servo[7].write(i);
      delay(15);
    }
  }

  /* 티백모드 전용 상단부 하강 함수 */
}
void teabag_down() {
  for (int i = servo[3].read(); i > 0; i--) {
    servo[3].write(i);
    delay(15);
  }
}
/* 티백모드 전용 중심부 하강 함수 */
void teabag_down_mid() {
  for (j = servo[1].read(), k = servo[2].read(); j < 100; j++, k--) {
    servo[1].write(j); //100 중심-우측 3번핀
    servo[2].write(k); //60 중심-좌측  4번핀
    delay(15);
  }
}
/* DC모터 실행하는 함수 */
void Start_Mix() {
  analogWrite(nor, 160); //건전지 6V로 전원 입력  150값 -> 약 3.52V로 정회전
  analogWrite(rev, 0);  // 역회전 X
}
/* DC모터 정지시키는 함수 */
void Stop_Mix() {
  analogWrite(nor, 0); // 정회전X
  analogWrite(rev, 0); // 역회전 X
}
/* 부저로 동작 종료후 울리는 알람 함수 */
void music() {
  tone(25, 650);
  delay(500);
  tone(25, 622);
  delay(500);
  tone(25, 650);
  delay(500);
  tone(25, 622);
  delay(500);
  tone(25, 650);
  delay(500);
  tone(25, 485);
  delay(500);
  tone(25, 590);
  delay(500);
  tone(25, 529);
  delay(500);
  tone(25, 435);
  delay(500);
  noTone(25);
}
/* 초음파 센서로 거리 측정하는 함수 */
int Distance() {
  digitalWrite(cup_trig, LOW);
  digitalWrite(cup_echo, LOW);
  delayMicroseconds(2);
  digitalWrite(cup_trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(cup_trig, LOW);
  unsigned long duration1 = pulseIn(cup_echo, HIGH);
  dis = ((340 * duration1) / 10000) / 2;
  return dis; // 측정한 거리값 리턴
}
