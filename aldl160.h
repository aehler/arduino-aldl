#ifndef ALDL160_H
#define ALDL160_H
#include <Arduino.h>

int readByte();
void init(int aldlPin, int frameSize, int irq);
void synchronizeBit();
void interrupt();
int readBit();

#endif
