#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  int i;
  for (i = 0; i < 10; i++) {
    int val = getcarbon();
    printf("Carbon Intensity: %d\n", val);
    pause(10);
  }
  exit(0);
}
