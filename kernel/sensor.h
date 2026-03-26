#ifndef SENSOR_H
#define SENSOR_H

#include "spinlock.h"
#include "types.h"

struct sensor {
  int current;
  int history[5];
  int index;
  struct spinlock lock;
};

void sensor_init(void);
void sensor_update(void);
int get_carbon(void);

#endif