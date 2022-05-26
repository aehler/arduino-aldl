#include "aldl160.h"

#define BIT_TIME 6250
#define T01_TIME_MIN 125 //Ticks
#define T01_TIME_MAX 875
#define DATA_VALID_TIME_MIN 900
#define DATA_VALID_TIME_MAX 1438
#define DATA_BYTE
#define ALDL_BYTE_SIZE 9

volatile unsigned int bitIndex = 0;
bool bitTime = false;
bool synchronized = false;
unsigned long period = 0;
int bitPartVal = 0;
int irqCount = 0;
int ALDL_PIN;

void init(int aldlPin, int irq) {
  ALDL_PIN = aldlPin;
  pinMode(ALDL_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(irq), interrupt, CHANGE);

    // инициализация Timer1
    noInterrupts();  // отключить глобальные прерывания
    TCCR1A = 0;
    TCCR1B = 0;

    OCR1A = 390; // установка регистра совпадения на 6,25 милисекунды (390.625 точно, но погрешность не страшна)

    TCCR1B |= (1 << WGM12);  // включить CTC режим 
    TCCR1B |= (1 << CS12); // Установить биты на коэффициент деления 256 (62500 тиков в секунду)

    TIMSK1 |= (1 << OCIE1A);  // включить прерывание по совпадению таймера 
    interrupts(); // включить глобальные прерывания
}

ISR(TIMER1_OVF_vect)                    // процедура обработки прерывания переполнения счетчика (добавить смещение к таймеру)
{
  //digitalWrite(ledPin, digitalRead(ledPin) ^ 1);  //включаем и выключаем светодиод
  delayMicroseconds(8);
  TCNT1 = 0; //Обнуляем таймер
  irqCount = 0;
}

void interrupt() {
  if(!synchronized) { //Bit sync block
    synchronizeBit();
    return;
  }
  bitPartVal = readBit();
  Serial.print(micros());
  Serial.print(" ");
  Serial.print(TCNT1);
  Serial.print(" ");
  Serial.print(digitalRead(ALDL_PIN));
  Serial.print(" ");
  Serial.println(bitPartVal);
}

void synchronizeBit() {
  if(irqCount == 0) {
      irqCount++;
      TCNT1 = 0;
      bitPartVal = digitalRead(ALDL_PIN); //expecting LOW
      return;
    }
    else {
      if((T01_TIME_MAX) < TCNT1) { //Sync changed to data
        synchronized = true;
        irqCount = 0;
        return;
      }
      if((T01_TIME_MAX + DATA_VALID_TIME_MAX) < TCNT1) {//Sync and data equal
        synchronized = true;
        irqCount = 0;
        return;
      }
    }
}

int readBit(){
  irqCount++;
  if(irqCount < 1) {
    return -1; //Not a data bit;
  }
  if(irqCount == 1) {
    if((T01_TIME_MAX) < TCNT1) { //This is a sync part of a bit, thus was an early interrupt, consider 0
      return 0;
    }
    if((T01_TIME_MAX + DATA_VALID_TIME_MAX) < TCNT1) { //This is a data part, late second interrupt, must be 1
      return 1;
    }
  }
}
