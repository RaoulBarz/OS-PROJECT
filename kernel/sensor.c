#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"
#include "types.h"
#include "sensor.h"

struct sensor s;
static int seed = 123456789;

static int rand(void) {
  seed = (seed * 1103515245 + 12345) & 0x7fffffff;
  return seed;
}

void sensor_init(void) {
  initlock(&s.lock, "sensor");
  s.current = 50;
  for (int i = 0; i < 5; i++) {
    s.history[i] = 50;
  }
  s.index = 0;
}

void sensor_update(void) {
  acquire(&s.lock);

  int change = (rand() % 11) - 5; // -5 to +5
  s.current += change;

  if (s.current < 0)
    s.current = 0;
  if (s.current > 100)
    s.current = 100;

  s.history[s.index] = s.current;
  s.index = (s.index + 1) % 5;

  release(&s.lock);
}

int get_carbon(void) {
  int val;
  acquire(&s.lock);
  val = s.current;
  release(&s.lock);
  return val;
}
