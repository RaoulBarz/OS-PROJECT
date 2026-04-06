#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "sensor.h"

struct sensor s;
static int seed = 123456789;

static int rand(void) {
  seed = (seed * 1103515245 + 12345) & 0x7fffffff;
  return seed;
}

static void recompute_predicted(void) {
  int sum = 0;
  for (int i = 0; i < HISTORY_SIZE; i++)
    sum += s.history[i];
  s.predicted = sum / HISTORY_SIZE;
}

void sensor_init(void) {
  initlock(&s.lock, "sensor");
  s.current = (CARBON_MIN + CARBON_MAX) / 2;
  for (int i = 0; i < HISTORY_SIZE; i++)
    s.history[i] = s.current;
  s.index = 0;
  s.predicted = s.current;
}

// Simulated environmental sensor: bounded random walk + ring buffer +
// moving-average forecast (last HISTORY_SIZE readings).
void sensor_update(void) {
  acquire(&s.lock);

  int change = (rand() % 11) - 5; // -5 .. +5
  s.current += change;
  if (s.current < CARBON_MIN)
    s.current = CARBON_MIN;
  if (s.current > CARBON_MAX)
    s.current = CARBON_MAX;

  s.history[s.index] = s.current;
  s.index = (s.index + 1) % HISTORY_SIZE;
  recompute_predicted();

  release(&s.lock);
}

int get_carbon(void) {
  int val;
  acquire(&s.lock);
  val = s.current;
  release(&s.lock);
  return val;
}

int get_predicted_carbon(void) {
  int val;
  acquire(&s.lock);
  val = s.predicted;
  release(&s.lock);
  return val;
}

void update_carbon(int new_value) {
  acquire(&s.lock);
  if (new_value < CARBON_MIN)
    new_value = CARBON_MIN;
  if (new_value > CARBON_MAX)
    new_value = CARBON_MAX;
  s.current = new_value;
  s.history[s.index] = new_value;
  s.index = (s.index + 1) % HISTORY_SIZE;
  recompute_predicted();
  release(&s.lock);
}
