//===============================================================================================================================//
//= Прошивка для модуля индикации на 4-х газоразрядных индикаторах                                                              =//
//= Автор FIRSTQWERT firstqwert@gmail.com                                                                                       =//
//= Версия 1.12_22 от 03.02.2020                                                                                                 =//
//===============================================================================================================================//

#include <EEPROM.h> // подключаем библиотеку EEPROM
#include "Button.h"
Button knopki;

// выводы для дешифратора К155ИД1
  int out1 = A3;
  int out2 = A1;
  int out4 = A0;
  int out8 = A2;
// выводы для транзисторных ключей TLP-627-4
  int key1 = 8; // 1 Сегмент
  int key2 = 7; // 2 Сегмент
  int key3 = 6; // 3 Сегмент
  int key4 = 5; // 4 Сегмент

  int KeyA = 0; // Кнопка А
  int KeyB = 0; // Кнопка B
  int KeyC = 0;
  const int signalPin = 2; // Вход для импульсов
  boolean signalStatus1 = false; //логический флаг предыдущего состояния сигнала включено/выключено. По умолчанию при старте программы выключено
  bool temp_signal; // Переменная для импульсов
  int KeyBL = 1; // Долгое нажатие кнопки B
  int otrav = 0; 
  int otrav2 = 10; // Таймер неактивности ненажатых кнопок сек. Управляет функцией пробега по всем цифрам при бездействии

  long randNumber; // Рандомайзер для режима 8

  byte save_regim = 0; // Сохраняем режим работы

  int led1 = 11; // вывод "P5 PWM" для всего, в том числе для светодиода
  float brightness = 0;    // Начальное состояние светодиода 0 - выключен, 255 - максимум яркости
  float fadeAmount = 0.2;    // Кол-во шагов при зажигании светодиода. Чем меньше, тем дольше и плавнее зажигается

  unsigned long duration; // Для ловли сигнала
  unsigned long startTime = 0; // Для секундомера
  unsigned long startTime2 = 0; // Для секундомера

  int sec = 0; // Счётчик тактов
  int mode = 0; 
  int currentdigit = 0; 
  bool blinkflag = 0;  
//  Переменные для вывода на индикаторы, сделал на 4 сегмента сразу, чтобы если что код не переписывать.
  int a;  //4 сегмент
  int b;  //1 сегмент
  int c;  //2 сегмент
  int d;  //3 сегмент
  int y;  
  int digits[3]; // массив для текущего значения времени на четыре цифры

  int key_sbros = 0; //Сброс к заводским по 4х-кратному долгому нажатию А
  
//===============================================================================================================================
void setup() {
  // задаем частоту ШИМ на 9 выводе 30кГц
  TCCR1B=TCCR1B&0b11111000|0x01;
  analogWrite(9,130);

  randomSeed(analogRead(0)); // Семя рандома берётся из шума на порту A0
 
  //задаем режим работы выходов микроконтроллера
  pinMode(out1,OUTPUT);
  pinMode(out2,OUTPUT);
  pinMode(out4,OUTPUT);
  pinMode(out8,OUTPUT);
 
  pinMode(key1,OUTPUT);
  pinMode(key2,OUTPUT);
  pinMode(key3,OUTPUT);
  pinMode(key4,OUTPUT);

  pinMode(led1,OUTPUT); //Выход светодиода

  pinMode(signalPin, INPUT); // Вход сигнала к земле

  save_regim = EEPROM.read(0);

  Serial.begin(9600);  // Start the serial interface

//Кнопки
  knopki.NO(); // Нормально разомкнутые
  knopki.pullUp(); // Подтяжка к +5В
  knopki.duration_inactivity_Up ( 10000 ); //состояние таймера неактивности ненажатой кнопки ms, 1000ms = 1c
  knopki.duration_bounce       (  50 ); // Таймер дребезга ms
  knopki.duration_press        ( 1000 ); // Таймер длительного нажатия ms
  knopki.button( 3, 4 ); // Пины кнопок
 }
//===============================================================================================================================
void loop() {
  knopki.read();  // Читаем кнопки

  for (;brightness<=255;brightness=brightness+fadeAmount){analogWrite(led1, brightness);}  // Зажигание светодиода

  if (knopki.event_press_long  (0)  == 1 ) { key_sbros++; if (key_sbros == 4) {key_sbros=0; sbros(); }} //Сброс до заводских настроек

  if (knopki.event_press_long  (1)  == 1) { KeyBL++; } // Главное меню 001 - 008 

  d = KeyBL;

  switch ( KeyBL ){
    case 1: if (knopki.event_press_short  (0)  == 1) {regim_1(); sec=0;}  break;
    case 2: if (knopki.event_press_short  (0)  == 1) {regim_2(); sec=0;}  break;
    case 3: if (knopki.event_press_short  (0)  == 1) {regim_3(); sec=0;}  break;
    case 4: if (knopki.event_press_short  (0)  == 1) {regim_4(); sec=0;}  break;
    case 5: if (knopki.event_press_short  (0)  == 1) {regim_5(); sec=0;}  break;
    case 6: if (knopki.event_press_short  (0)  == 1) {regim_6(); sec=0;}  break;
    case 7: if (knopki.event_press_short  (0)  == 1) {regim_7(); sec=0;}  break;
    case 8: if (knopki.event_press_short  (0)  == 1) {regim_8(); sec=0;}  break;
    case 9: if (knopki.event_press_short  (0)  == 1) {regim_9(); sec=0;}  break;
  default: KeyBL=1;
                   }

  switch ( save_regim ){
    case 1: regim_1(); sec=0; break;
    case 2: regim_2(); sec=0; break;
    case 3: regim_3(); sec=0; break;
    case 4: regim_4(); sec=0; break;
    case 5: regim_5(); sec=0; break;
    case 6: regim_6(); sec=0; break;
    case 7: regim_7(); sec=0; break;
    case 8: regim_8(); sec=0; break;
    case 9: regim_9(); sec=0; break;
    default: save_regim = 0;
                        }
  save_regim = 0;

// Вывод массива на индикаторы
  digits[0] = b;  //1 сегмент
  digits[1] = c;  //2 сегмент
  digits[2] = d;  //3 сегмент
  digits[3] = a;  //4 сегмент

  if (knopki.state_inactivity_Up() == 0 ) // Антиотравление индикаторов 
  { 
    startTime = millis()/1000;
    if ( startTime - startTime2 >= 1){otrav++; }
    if ( otrav == otrav2 ) { Safe(digits); otrav = 0;}
    startTime2 = startTime;
  }
  if (knopki.event_press_long  ()  == 1 || knopki.event_press_short  ()  == 1) { otrav = 0; }

  show(digits); // вывести цифры на дисплей
}

//===============================================================================================================================//
//=                                                   Тут всякие функции                                                        =//
//===============================================================================================================================//
void  regim_1()  // Секундомер
{
  EEPROM.write(0, 1);
  a=0; b=0; c=0; d=0;
  while(true)
    {
    knopki.read();  //Читаем кнопки
    if (knopki.event_press_short  (0)  == 1) {KeyA++;}
    if (knopki.event_press_short  (1)  == 1) {KeyB++;}
    if (KeyB>=2){KeyB=1;}
    if (knopki.event_press_long  (1)  == 1) {KeyBL++;}
    if (KeyBL>=2){KeyBL=0; a=0; b=0; c=0; d=1; KeyA=1; break;}

    switch(KeyA)
    {
      case 1:
            if (KeyB==1){sec=0;KeyB=0;a=0; b=0; c=0; d=0;}
      break;

      case 2:
            if (KeyB==1){sec=0;KeyB=0;}
            startTime = millis()/1000;
            if (startTime - startTime2 >= 1){sec++; }
            a = sec/1000;
            y = sec - a*1000;
            b = y/100;
            y = y - b*100;
            c = y/10;
            d = y - c*10;
            startTime2 = startTime;
            if (sec>9999){sec=0;}
      break;
      
      default:
      KeyA=1;
    }

    digits[0] = b;  //1 сегмент
    digits[1] = c;  //2 сегмент
    digits[2] = d;  //3 сегмент
    digits[3] = a;  //4 сегмент

    show(digits); // вывести цифры на дисплей
  
  }
}
//===============================================================================================================================
void  regim_2()  // До 4 тактов
{
    EEPROM.write(0, 2);

    int takt;
    a = EEPROM.read(5);
    b = EEPROM.read(6);
    c = EEPROM.read(7);
    d = EEPROM.read(8);
    takt = 1000*a + 100*b + 10*c + d;

    if (takt == 0 || takt > 9999){takt = 4;}
    
  while(true)
  {
    knopki.read();  //Читаем кнопки
    if (digitalRead(signalPin) == HIGH && !temp_signal) 
    { 
         
      temp_signal = 1;
      signalStatus1 = !signalStatus1;                 //меняем логический флаг на противоположный от установленного предыдущим сигналом
      KeyC++;sec++;
      }
    else if (digitalRead(signalPin) == 0 && temp_signal)temp_signal = 0;
    
    if (knopki.event_press_short  (1)  == 1) {KeyB++;} //Короткое нажатие B
    if (KeyB>=2){KeyB=1;}
    if (knopki.event_press_long  (1)  == 1) {KeyBL=2; a=0; b=0; c=0; d=2; KeyA=1; break;} //Долгое удержание кнопки B, выход
    if (knopki.event_press_long  (0)  == 1)  // Долгое удержание А, вход в функцию выбора
        {
          takt = settings(takt);
          if (takt != 4)
          {
          a = takt/1000; b=(takt/100) % 10; c=(takt/10) % 10; d=takt%10;
          EEPROM.write(5, a);
          EEPROM.write(6, b);
          EEPROM.write(7, c);
          EEPROM.write(8, d);
          }
        }
    if (sec>takt){sec=1;} //Количество отсчётов

    a=10; b=10; c=10; d=sec%10;
    if ( sec > 9 && sec < 100 ){ c=sec/10; d=sec%10; }
    if ( sec == 100 ) { b = 1; c = 0; }
    if ( sec > 100 && sec < 1000 ){ b=sec/100; c=(sec/10) % 10; d=sec%10; }
    if ( sec == 1000 ) { a = 1; b = 0; c = 0; }
    if ( sec > 1000 ){ a = sec/1000; b=(sec/100) % 10; c=(sec/10) % 10; d=sec%10; }
 
    switch(KeyC)
    {
      case 1:
          if (KeyB==1){sec=0;KeyB=0;a=10; b=10; c=10; d=0;}
      break;

      case 2:
           if (KeyB==1){sec=0;KeyB=0;}
      break;
      
      default:
      KeyC=1;
    }
    digits[0] = b;  //1 сегмент
    digits[1] = c;  //2 сегмент
    digits[2] = d;  //3 сегмент
    digits[3] = a;  //4 сегмент

    show(digits); // вывести цифры на дисплей
  }
}
//===============================================================================================================================
void  regim_3()  // До 8 тактов
{
    EEPROM.write(0, 3);

    int takt;
    a = EEPROM.read(9);
    b = EEPROM.read(10);
    c = EEPROM.read(11);
    d = EEPROM.read(12);
    takt = 1000*a + 100*b + 10*c + d;
    
    if (takt == 0 || takt > 9999){takt = 8;}
    
  while(true)
  {
        knopki.read();  //Читаем кнопки
    if (digitalRead(signalPin) == HIGH && !temp_signal) 
    { 
      temp_signal = 1;
      signalStatus1 = !signalStatus1;                 //меняем логический флаг на противоположный от установленного предыдущим сигналом
      KeyC++;sec++;
      }
    else if (digitalRead(signalPin) == 0 && temp_signal)temp_signal = 0;

    if (knopki.event_press_short  (1)  == 1) {KeyB++;} //Короткое нажатие B
    if (KeyB>=2){KeyB=1;}
    if (knopki.event_press_long  (1)  == 1) {KeyBL=3; a=0; b=0; c=0; d=3; KeyA=1; break;} //Долгое удержание кнопки B, выход
    if (knopki.event_press_long  (0)  == 1)  // Долгое удержание А, вход в функцию выбора
        {
          takt = settings(takt);
          if (takt != 8)
          {
          a = takt/1000; b=(takt/100) % 10; c=(takt/10) % 10; d=takt%10;
          EEPROM.write(9, a);
          EEPROM.write(10, b);
          EEPROM.write(11, c);
          EEPROM.write(12, d);
          }
        }
    if (sec>takt){sec=1;} //Количество отсчётов

    a=10; b=10; c=10; d=sec%10;
    if ( sec > 9 && sec < 100 ){ c=sec/10; d=sec%10; }
    if ( sec == 100 ) { b = 1; c = 0; }
    if ( sec > 100 && sec < 1000 ){ b=sec/100; c=(sec/10) % 10; d=sec%10; }
    if ( sec == 1000 ) { a = 1; b = 0; c = 0; }
    if ( sec > 1000 ){ a = sec/1000; b=(sec/100) % 10; c=(sec/10) % 10; d=sec%10; }
 
    switch(KeyC)
    {
      case 1:
          if (KeyB==1){sec=0;KeyB=0;a=10; b=10; c=10; d=0;}
      break;

      case 2:
           if (KeyB==1){sec=0;KeyB=0;}
      break;
      
      default:
      KeyC=1;
    }

    digits[0] = b;  //1 сегмент
    digits[1] = c;  //2 сегмент
    digits[2] = d;  //3 сегмент
    digits[3] = a;  //4 сегмент

    show(digits); // вывести цифры на дисплей
  }
}
//===============================================================================================================================
void  regim_4() // До 16 тактов
{
    EEPROM.write(0, 4);

    int takt;
    a = EEPROM.read(13);
    b = EEPROM.read(14);
    c = EEPROM.read(15);
    d = EEPROM.read(16);
    takt = 1000*a + 100*b + 10*c + d;
    
    if (takt == 0 || takt > 9999){takt = 16;}
    
  while(true)
  {
        knopki.read();  //Читаем кнопки
    if (digitalRead(signalPin) == HIGH && !temp_signal) 
    { 
      temp_signal = 1;
      signalStatus1 = !signalStatus1;                 //меняем логический флаг на противоположный от установленного предыдущим сигналом
      KeyC++;sec++;
      }
    else if (digitalRead(signalPin) == 0 && temp_signal)temp_signal = 0;

    if (knopki.event_press_short  (1)  == 1) {KeyB++;} //Короткое нажатие B
    if (KeyB>=2){KeyB=1;}
    if (knopki.event_press_long  (1)  == 1) {KeyBL=4; a=0; b=0; c=0; d=4; KeyA=1; break;} //Долгое удержание кнопки B, выход
    if (knopki.event_press_long  (0)  == 1)  // Долгое удержание А, вход в функцию выбора
        {
          takt = settings(takt);
          if (takt != 16)
          {
          a = takt/1000; b=(takt/100) % 10; c=(takt/10) % 10; d=takt%10;
          EEPROM.write(13, a);
          EEPROM.write(14, b);
          EEPROM.write(15, c);
          EEPROM.write(16, d);
          }
        }
    if (sec>takt){sec=1;} //Количество отсчётов

    a=10; b=10; c=10; d=sec%10;
    if ( sec > 9 && sec < 100 ){ c=sec/10; d=sec%10; }
    if ( sec == 100 ) { b = 1; c = 0; }
    if ( sec > 100 && sec < 1000 ){ b=sec/100; c=(sec/10) % 10; d=sec%10; }
    if ( sec == 1000 ) { a = 1; b = 0; c = 0; }
    if ( sec > 1000 ){ a = sec/1000; b=(sec/100) % 10; c=(sec/10) % 10; d=sec%10; }
    switch(KeyC)
    {
      case 1:
          if (KeyB==1){sec=0;KeyB=0;a=10; b=10; c=10; d=0;}
      break;

      case 2:
           if (KeyB==1){sec=0;KeyB=0;}
      break;
      
      default:
      KeyC=1;
    }

    digits[0] = b;  //1 сегмент
    digits[1] = c;  //2 сегмент
    digits[2] = d;  //3 сегмент
    digits[3] = a;  //4 сегмент

    show(digits); // вывести цифры на дисплей
  }
}
//===============================================================================================================================
void  regim_5()  // До 32 тактов
{
    EEPROM.write(0, 5);

    int takt;
    a = EEPROM.read(17);
    b = EEPROM.read(18);
    c = EEPROM.read(19);
    d = EEPROM.read(20);
    takt = 1000*a + 100*b + 10*c + d;
    if (takt == 0 || takt > 9999){takt = 32;}
    
  while(true)
  {
        knopki.read();  //Читаем кнопки
    if (digitalRead(signalPin) == HIGH && !temp_signal) 
    { 
      temp_signal = 1;
      signalStatus1 = !signalStatus1;                 //меняем логический флаг на противоположный от установленного предыдущим сигналом
      KeyC++;sec++;
      }
    else if (digitalRead(signalPin) == 0 && temp_signal)temp_signal = 0;

    if (knopki.event_press_short  (1)  == 1) {KeyB++;} //Короткое нажатие B
    if (KeyB>=2){KeyB=1;}
    if (knopki.event_press_long  (1)  == 1) {KeyBL=5; a=0; b=0; c=0; d=5; KeyA=1; break;} //Долгое удержание кнопки B, выход
    if (knopki.event_press_long  (0)  == 1)  // Долгое удержание А, вход в функцию выбора
        {
          takt = settings(takt);
          if (takt != 32)
          {
          a = takt/1000; b=(takt/100) % 10; c=(takt/10) % 10; d=takt%10;
          EEPROM.write(17, a);
          EEPROM.write(18, b);
          EEPROM.write(19, c);
          EEPROM.write(20, d);
          }
        }
    if (sec>takt){sec=1;} //Количество отсчётов

    a=10; b=10; c=10; d=sec%10;
    if ( sec > 9 && sec < 100 ){ c=sec/10; d=sec%10; }
    if ( sec == 100 ) { b = 1; c = 0; }
    if ( sec > 100 && sec < 1000 ){ b=sec/100; c=(sec/10) % 10; d=sec%10; }
    if ( sec == 1000 ) { a = 1; b = 0; c = 0; }
    if ( sec > 1000 ){ a = sec/1000; b=(sec/100) % 10; c=(sec/10) % 10; d=sec%10; }
    switch(KeyC)
    {
      case 1:
          if (KeyB==1){sec=0;KeyB=0;a=10; b=10; c=10; d=0;}
      break;

      case 2:
           if (KeyB==1){sec=0;KeyB=0;}
      break;
      
      default:
      KeyC=1;
    }

    digits[0] = b;  //1 сегмент
    digits[1] = c;  //2 сегмент
    digits[2] = d;  //3 сегмент
    digits[3] = a;  //4 сегмент

    show(digits); // вывести цифры на дисплей
  }
}
//===============================================================================================================================
void  regim_6()  // До 64 тактов
{
    EEPROM.write(0, 6);

    int takt;
    a = EEPROM.read(21);
    b = EEPROM.read(22);
    c = EEPROM.read(23);
    d = EEPROM.read(24);
    takt = 1000*a + 100*b + 10*c + d;
    if (takt == 0 || takt > 9999){takt = 64;}
    
  while(true)
  {
        knopki.read();  //Читаем кнопки
    if (digitalRead(signalPin) == HIGH && !temp_signal) 
    { 
      temp_signal = 1;
      signalStatus1 = !signalStatus1;                 //меняем логический флаг на противоположный от установленного предыдущим сигналом
      KeyC++;sec++;
      }
    else if (digitalRead(signalPin) == 0 && temp_signal)temp_signal = 0;

    if (knopki.event_press_short  (1)  == 1) {KeyB++;} //Короткое нажатие B
    if (KeyB>=2){KeyB=1;}
    if (knopki.event_press_long  (1)  == 1) {KeyBL=6; a=0; b=0; c=0; d=6; KeyA=1; break;} //Долгое удержание кнопки B, выход
    if (knopki.event_press_long  (0)  == 1)  // Долгое удержание А, вход в функцию выбора
        {
          takt = settings(takt);
          if (takt != 64)
          {
          a = takt/1000; b=(takt/100) % 10; c=(takt/10) % 10; d=takt%10;
          EEPROM.write(21, a);
          EEPROM.write(22, b);
          EEPROM.write(23, c);
          EEPROM.write(24, d);
          }
        }
    if (sec>takt){sec=1;} //Количество отсчётов

    a=10; b=10; c=10; d=sec%10;
    if ( sec > 9 && sec < 100 ){ c=sec/10; d=sec%10; }
    if ( sec == 100 ) { b = 1; c = 0; }
    if ( sec > 100 && sec < 1000 ){ b=sec/100; c=(sec/10) % 10; d=sec%10; }
    if ( sec == 1000 ) { a = 1; b = 0; c = 0; }
    if ( sec > 1000 ){ a = sec/1000; b=(sec/100) % 10; c=(sec/10) % 10; d=sec%10; }
    switch(KeyC)
    {
      case 1:
          if (KeyB==1){sec=0;KeyB=0;a=10; b=10; c=10; d=0;}
      break;

      case 2:
           if (KeyB==1){sec=0;KeyB=0;}
      break;
      
      default:
      KeyC=1;
    }

    digits[0] = b;  //1 сегмент
    digits[1] = c;  //2 сегмент
    digits[2] = d;  //3 сегмент
    digits[3] = a;  //4 сегмент

    show(digits); // вывести цифры на дисплей
  }
}
//===============================================================================================================================
void  regim_7()  //До 9999 тактов
{
    EEPROM.write(0, 7);

    int takt;
    a = EEPROM.read(25);
    b = EEPROM.read(26);
    c = EEPROM.read(27);
    d = EEPROM.read(28);
    takt = 1000*a + 100*b + 10*c + d;
    if (takt == 0 || takt > 9999){takt = 9999;}

  while(true)
  {
    knopki.read();  //Читаем кнопки
    if (digitalRead(signalPin) == HIGH && !temp_signal) 
    { 
      temp_signal = 1;
      signalStatus1 = !signalStatus1;                 //меняем логический флаг на противоположный от установленного предыдущим сигналом
      KeyC++;sec++;
     }
    else if (digitalRead(signalPin) == 0 && temp_signal)temp_signal = 0;

    if (knopki.event_press_short  (1)  == 1) {KeyB++;} //Короткое нажатие B
    if (KeyB>=2){KeyB=1;}
    if (knopki.event_press_long  (1)  == 1) {KeyBL=7; a=0; b=0; c=0; d=7; KeyA=1; break;} //Долгое удержание кнопки B, выход
    if (knopki.event_press_long  (0)  == 1)  // Долгое удержание А, вход в функцию выбора
        {
          takt = settings(takt);
          if (takt != 9999)
          {
          a = takt/1000; b=(takt/100) % 10; c=(takt/10) % 10; d=takt%10;
          EEPROM.write(25, a);
          EEPROM.write(26, b);
          EEPROM.write(27, c);
          EEPROM.write(28, d);
          }
        }
    if (sec>takt){sec=1;} //Количество отсчётов

    a=10; b=10; c=10; d=sec%10;
    if ( sec > 9 && sec < 100 ){ c=sec/10; d=sec%10; }
    if ( sec == 100 ) { b = 1; c = 0; }
    if ( sec > 100 && sec < 1000 ){ b=sec/100; c=(sec/10) % 10; d=sec%10; }
    if ( sec == 1000 ) { a = 1; b = 0; c = 0; }
    if ( sec > 1000 ){ a = sec/1000; b=(sec/100) % 10; c=(sec/10) % 10; d=sec%10; }
    if ( sec == 10000 ) { a = 10; b = 10; c = 10; }
   
    switch(KeyC)
    {
      case 1:
          if (KeyB==1){sec=0;KeyB=0;a=10; b=10; c=10; d=0;}
      break;

      case 2:
           if (KeyB==1){sec=0;KeyB=0;}
      break;
      
      default:
      KeyC=1;
    }

    digits[0] = b;  //1 сегмент
    digits[1] = c;  //2 сегмент
    digits[2] = d;  //3 сегмент
    digits[3] = a;  //4 сегмент

    show(digits); // вывести цифры на дисплей
   
  }
}
//==============================================================================================================================
void  regim_8()  // Рандомайзер
{
    EEPROM.write(0, 8);
  while(true)
  {
        knopki.read();  //Читаем кнопки
    if (digitalRead(signalPin) == HIGH && !temp_signal) 
    { 
      temp_signal = 1;
      signalStatus1 = !signalStatus1;                 //меняем логический флаг на противоположный от установленного предыдущим сигналом
      KeyC++; sec++; randNumber = random(10000);
    }
    else if (digitalRead(signalPin) == 0 && temp_signal)temp_signal = 0;

    if (knopki.event_press_short  (1)  == 1) {KeyB++;} //Короткое нажатие B
    if (KeyB>=2){KeyB=1;}
    if (knopki.event_press_long  (1)  == 1) {KeyBL=8; a=0; b=0; c=0; d=8; KeyA=1; break;} //Долгое удержание кнопки B, выход

    a=0; b=0; c=0; d=randNumber%10;
    if ( randNumber > 9 && randNumber < 100 ){ c=randNumber/10; d=randNumber%10; }
    if ( randNumber == 100 ) { b = 1; c = 0; }
    if ( randNumber > 100 ){ b=randNumber/100; c=(randNumber/10) % 10; d=randNumber%10; }
    if ( randNumber == 1000 ) { a = 1; b = 0; c = 0; }
    if ( randNumber > 1000 ){ a = randNumber/1000; b=(randNumber/100) % 10; c=(randNumber/10) % 10; d=randNumber%10; }
    
    switch(KeyC)
    {
      case 1:
          if (KeyB==1){randNumber=0;KeyB=0;a=0; b=0; c=0; d=0;}
      break;

      case 2:
           if (KeyB==1){randNumber=0;KeyB=0;}
      break;
      
      default:
      KeyC=1;
    }

    digits[0] = b;  //1 сегмент
    digits[1] = c;  //2 сегмент
    digits[2] = d;  //3 сегмент
    digits[3] = a;  //4 сегмент

    show(digits); // вывести цифры на дисплей
  
  }
}
//==============================================================================================================================
void  regim_9()  // Signal generator
{
  EEPROM.write(0, 9);
  pinMode(signalPin, OUTPUT);

    int msec;
    int takt;
      
    a = EEPROM.read(29);
    b = EEPROM.read(30);
    c = EEPROM.read(31);
    d = EEPROM.read(32);
        
    takt = 1000*a + 100*b + 10*c + d;
    msec = 60000 / takt;

  if (takt>250){takt=30;}
  if (takt<30){takt=250;}

  while(true)
  {
    knopki.read();  //Читаем кнопки

    if (knopki.event_press_short  (1)  == 1) {takt++; if (takt>250){takt=30;} msec = 60000 / takt ; a = takt/1000; b=(takt/100) % 10; c=(takt/10) % 10; d=takt%10; EEPROM.write(29, a); EEPROM.write(30, b); EEPROM.write(31, c); EEPROM.write(32, d); } //Короткое нажатие B
    if (knopki.event_press_short  (0)  == 1) {takt--; if (takt<30){takt=250;} msec = 60000 / takt ; a = takt/1000; b=(takt/100) % 10; c=(takt/10) % 10; d=takt%10; EEPROM.write(29, a); EEPROM.write(30, b); EEPROM.write(31, c); EEPROM.write(32, d); } //Короткое нажатие A   
    
    if (KeyB>=2){KeyB=1;}
    if (knopki.event_press_long  (1)  == 1) { KeyBL=9; pinMode(signalPin, INPUT);a=0; b=0; c=0; d=9; KeyA=1;break; } //Долгое удержание кнопки B, выход
        
           startTime = millis();
           
            otrav = (millis() / 1000) % otrav2;
            
           if (startTime - startTime2 >= msec){ digitalWrite(signalPin, HIGH);}
           
           if (startTime - startTime2  >= msec + 16)
           { 
            startTime2 = startTime - 19; digitalWrite(signalPin, LOW); 
            if (otrav >= otrav2 - 2) 
            {
              for (int x=0; x < 9; x++)
              {
                digits[3] = x;  
                show(digits);
                }
                for (int x=0;x<9;x++)
                {
                digits[0] = x;  
                show(digits);
                }
                for (int x=0;x<9;x++)
                {
                digits[1] = x;  
                show(digits);
                }
                for (int x=0;x<9;x++)
                {
                digits[2] = x;  
                show(digits);
                }
              }
           }

    a=10; b=10; c=10; d=takt%10;
    if ( takt > 9 && takt < 100 ){ c=takt/10; d=takt%10; }
    if ( takt == 100 ) { b = 1; c = 0; }
    if ( takt > 100 && takt < 1000 ){ b=takt/100; c=(takt/10) % 10; d=takt%10; }
   
    digits[0] = b;  //1 сегмент
    digits[1] = c;  //2 сегмент
    digits[2] = d;  //3 сегмент
    digits[3] = a;  //4 сегмент

    show(digits); // вывести цифры на дисплей
  }
}
//==============================================================================================================================
int settings (int takt)
{
  int result;

  while(true)
  {
    knopki.read();  //Читаем кнопки

    if (knopki.event_press_short  (1)  == 1) {sec++;}
    if (knopki.event_press_short  (0)  == 1) {sec--;}
    if (knopki.event_press_long  (0)  == 1) {break;}
    if (sec>9999){sec=1;} //Количество отсчётов
    if (sec<1){sec=9999;} //Количество отсчётов

    a=10; b=10; c=10; d=sec%10;
    if ( sec > 9 && sec < 100 ){ c=sec/10; d=sec%10; }
    if ( sec == 100 ) { b = 1; c = 0; }
    if ( sec > 100 && sec < 1000 ){ b=sec/100; c=(sec/10) % 10; d=sec%10; }
    if ( sec == 1000 ) { a = 1; b = 0; c = 0; }
    if ( sec > 1000 ){ a = sec/1000; b=(sec/100) % 10; c=(sec/10) % 10; d=sec%10; }

    digits[0] = b;  //1 сегмент
    digits[1] = c;  //2 сегмент
    digits[2] = d;  //3 сегмент
    digits[3] = a;  //4 сегмент

    show(digits); // вывести цифры на дисплей
  }
  result = sec;
  return result;
}
//===============================================================================================================================
void show(int a[])
{
    //выведем цифру a[0] на первый индикатор
  setNumber(a[0]);
  if (!(mode==1&&currentdigit==0&&blinkflag==false)) 
  {
    digitalWrite(key1,HIGH);
    delayMicroseconds(700); // 800
    //потушим первый индикатор
    digitalWrite(key1,LOW);
    delayMicroseconds(400);   // 400
  }
  
  //цифра a[1] на второй индикатор
  setNumber(a[1]);
  if (!(mode==1&&currentdigit==0&&blinkflag==false))
  {
  digitalWrite(key2,HIGH);
    delayMicroseconds(700); // 800
  //потушим второй индикатор
  digitalWrite(key2,LOW);
    delayMicroseconds(400);   // 400
  }
 
  //цифра a[2] на третий индикатор
  setNumber(a[2]);
  if (!(mode==1&&currentdigit==1&&blinkflag==false))
  {
  digitalWrite(key3,HIGH);
    delayMicroseconds(700); // 800
  //потушим третий индикатор
  digitalWrite(key3,LOW);
    delayMicroseconds(400);   // 400
  }
 
  //выведем цифру a[3] на четвертый индикатор
  setNumber(a[3]);
  if (!(mode==1&&currentdigit==1&&blinkflag==false))
  {
  digitalWrite(key4,HIGH);
    delayMicroseconds(700); // 800
  //потушим четвертый индикатор
  digitalWrite(key4,LOW);
    delayMicroseconds(400);   // 400
  }
}
//===============================================================================================================================
void  sbros()  // Сброс до заводских
{
    while(true)
    {
      knopki.read();  //Читаем кнопки
      if (knopki.event_press_long  (1)  == 1) {a=0;b=0;c=0;break;} //Долгое удержание кнопки B, выход
      if (knopki.event_press_long  (0)  == 1) //Долгое удержание кнопки А, сброс к заводским
      {
        EEPROM.write(5, 0); EEPROM.write(6, 0);EEPROM.write(7, 0);EEPROM.write(8, 4);
        EEPROM.write(9, 0); EEPROM.write(10, 0);EEPROM.write(11, 0);EEPROM.write(12, 8);
        EEPROM.write(13, 0); EEPROM.write(14, 0);EEPROM.write(15, 1);EEPROM.write(16, 6);
        EEPROM.write(17, 0); EEPROM.write(18, 0);EEPROM.write(19, 3);EEPROM.write(20, 2);
        EEPROM.write(21, 0); EEPROM.write(22, 0);EEPROM.write(23, 6);EEPROM.write(24, 4);
        EEPROM.write(25, 9); EEPROM.write(26, 9);EEPROM.write(27, 9);EEPROM.write(28, 9);
        EEPROM.write(29, 0); EEPROM.write(30, 0);EEPROM.write(31, 4);EEPROM.write(32, 2);
        delay(2000);
        a=0;b=0;c=0;
        break;
      }

    startTime = millis()/1000;
    if (startTime % 2 == 1){a=6;b=6;c=6;d=6;}
    else {a=10;b=10;c=10;d=10;}


    digits[0] = b;  //1 сегмент
    digits[1] = c;  //2 сегмент
    digits[2] = d;  //3 сегмент
    digits[3] = a;  //4 сегмент
  
    show(digits); // вывести цифры на дисплей
  }
}
//===============================================================================================================================
void setNumber(int num) // передача цифры на дешифратор
{
  switch (num)
  {
    case 0:
    digitalWrite (out1,LOW);
    digitalWrite (out2,LOW);
    digitalWrite (out4,LOW);
    digitalWrite (out8,LOW);
    break;
    case 1:
    digitalWrite (out1,HIGH);
    digitalWrite (out2,LOW);
    digitalWrite (out4,LOW);
    digitalWrite (out8,LOW);
    break;
    case 2:
    digitalWrite (out1,LOW);
    digitalWrite (out2,HIGH);
    digitalWrite (out4,LOW);
    digitalWrite (out8,LOW);
    break;
    case 3:
    digitalWrite (out1,HIGH);
    digitalWrite (out2,HIGH);
    digitalWrite (out4,LOW);
    digitalWrite (out8,LOW);
    break;
    case 4:
    digitalWrite (out1,LOW);
    digitalWrite (out2,LOW);
    digitalWrite (out4,HIGH);
    digitalWrite (out8,LOW);
    break;
    case 5:
    digitalWrite (out1,HIGH);
    digitalWrite (out2,LOW);
    digitalWrite (out4,HIGH);
    digitalWrite (out8,LOW);
    break;
    case 6:
    digitalWrite (out1,LOW);
    digitalWrite (out2,HIGH);
    digitalWrite (out4,HIGH);
    digitalWrite (out8,LOW);
    break;
    case 7:
    digitalWrite (out1,HIGH);
    digitalWrite (out2,HIGH);
    digitalWrite (out4,HIGH);
    digitalWrite (out8,LOW);
    break;
    case 8:
    digitalWrite (out1,LOW);
    digitalWrite (out2,LOW);
    digitalWrite (out4,LOW);
    digitalWrite (out8,HIGH);
    break;
    case 9:
    digitalWrite (out1,HIGH);
    digitalWrite (out2,LOW);
    digitalWrite (out4,LOW);
    digitalWrite (out8,HIGH);
    break;
    case 10: //Гасим цифру
    digitalWrite (out1,HIGH);
    digitalWrite (out2,HIGH);
    digitalWrite (out4,HIGH);
    digitalWrite (out8,HIGH);
    break;
  }
}
//===============================================================================================================================
void Safe(int n[]) //Антиотравление
{
  int h1 = n[0];
  int h2 = n[1];
  int m1 = n[2];
  int m2 = n[3];

  n[0]++;
  while(n[0]!=h1)
  {
    
    for (int x=0;x<20;x++)
    {
      show(n);
    }
    n[0]++;
    if (n[0]>9) n[0]=0;
  }
  
  n[1]++;
  while(n[1]!=h2)
  {
    
    for (int x=0;x<20;x++)
    {
      show(n);
    }
    n[1]++;
    if (n[1]>9) n[1]=0;
  }
  
  n[2]++;
  while(n[2]!=m1)
  {
    
    for (int x=0;x<20;x++)
    {
      show(n);
    }
    n[2]++;
    if (n[2]>9) n[2]=0;
  }
  
  n[3]++;
  while(n[3]!=m2)
  {
    
    for (int x=0;x<20;x++)
    {
      show(n);
    }
    n[3]++;
    if (n[3]>9) n[3]=0;
  }

}
