#ifndef SENSOR_H
#define SENSOR_H

#include "param.h"
#include "spinlock.h"
#include "types.h"

struct sensor {
  int current;
  int history[HISTORY_SIZE];
  int index;
  int predicted; // moving average of recent readings
  struct spinlock lock;
};

void sensor_init(void);
void sensor_update(void);
int get_carbon(void);
int get_predicted_carbon(void);
void update_carbon(int new_value);

#endif
