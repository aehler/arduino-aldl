// C++ code
//

int data_pin = A1;
int irq = 2;
bool sync = false;
byte b1 = 0;
byte b2 = 0;
byte pos = 0;
bool syncByte = false;
bool page = false;


void setup()
{
  Serial.begin(115200);
  init(data_pin, irq);
}

void loop()
{

}

void interrupt() {
  if(!sync) {
    delay(2);
	TCNT1 = 0;
    sync = true;
  }
}

bool fastRead(uint8_t pin) {
  if (pin < 8) return bitRead(PIND, pin);
  else if (pin < 14) return bitRead(PINB, pin - 8);
  else if (pin < 20) return bitRead(PINC, pin - 14);
}

void init(int aldlPin, int irq) {
  pinMode(aldlPin, INPUT);
  pinMode(irq, INPUT);
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

ISR(TIMER1_COMPA_vect)                    // процедура обработки прерывания переполнения счетчика (чтение чанка в 9 бит)
{
  delayMicroseconds(9);
  TCNT1 = 0;
  if(pos == 0) {
	syncByte = fastRead(data_pin);
	b1 = 0;
  } else {
  	if(fastRead(data_pin) == 1) {
		bitSet(b1, pos-1);
	}
  }
  pos++;
  if(pos == 9) {
	pos = 0;
    page = !page;
	Serial.print(syncByte);
	Serial.println(b1, BIN);
	b1 = 0;
	syncByte = false;
  }
}
