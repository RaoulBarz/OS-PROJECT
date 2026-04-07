#include "kernel/types.h"
#include "user/user.h"

/* xv6 user printf: only %d %x %p %c %s (no field widths like %6d) */

enum { U_LOW, U_MEDIUM, U_HIGH };

static void
divider(void)
{
  printf("--------------------------------------------------\n");
}

static void
banner(const char *title)
{
  printf("\n");
  divider();
  printf("  %s\n", title);
  divider();
}

/*
 * progress_lines: each iteration prints uptime() = global tick count.
 * ticks_since_last = how many timer ticks passed between two prints.
 * - Large jump: this process often was NOT running (e.g. deferred) while ticks advanced.
 * - Same uptime on several lines: the busy loop ran many times within one tick.
 */
static void
progress_lines(const char *tag, int n)
{
  int i;
  int prev_uptime = -1;

  printf("  Legend: uptime = global tick counter. ticks_since_last = uptime - previous uptime.\n");
  printf("  Large ticks_since_last on LOW urgency = time spent NOT running (deferral).\n\n");

  for (i = 0; i < n; i++) {
    int t = uptime();
    printf("  %s step %d  uptime=%d", tag, i, t);
    if (prev_uptime >= 0)
      printf("  ticks_since_last=%d", t - prev_uptime);
    printf("\n");
    prev_uptime = t;
    volatile int j;
    for (j = 0; j < 400000; j++)
      ;
  }
}

int
main(int argc, char *argv[])
{
  int pid;
  int i;
  int dl;

  banner("GreenOS demo (features [1]-[4])");

  printf("KEY IDEAS\n");
  printf("  uptime:     kernel tick count (timer interrupts). Same idea as sys_uptime().\n");
  printf("  cur / pred: simulated carbon now vs moving average of recent samples.\n");
  printf("  deferral:   LOW urgency + high carbon may sleep; timer still advances.\n\n");

  printf("FEATURE MAP\n");
  printf("  [1] Sensor in kernel + history    [2] Moving-average prediction (pred)\n");
  printf("  [3] Urgency + carbon in scheduler [4] Deferred queue + wakeup\n\n");

  /* [1][2] */
  banner("[1][2] Sensor snapshot");
  printf("WHAT THIS SHOWS\n");
  printf("  Each row is one sample. Columns:\n");
  printf("    uptime          = tick when we printed (roughly every 12 ticks).\n");
  printf("    cur             = getcarbon()  (current simulated intensity).\n");
  printf("    pred            = getpredicted() (average of last HISTORY_SIZE readings).\n");
  printf("    cur_minus_pred  = cur - pred (positive = above forecast).\n");
  printf("ROWS\n");
  printf("columns: uptime cur pred cur_minus_pred\n");
  for (i = 0; i < 10; i++) {
    int c = getcarbon();
    int p = getpredicted();
    printf("%d %d %d %d\n", uptime(), c, p, c - p);
    pause(12);
  }
  printf("(Sensor drifts via kernel sensor_update; pred lags/smooths cur.)\n\n");

  /* [3][4] LOW */
  banner("[3][4] LOW urgency + fixed high carbon (80)");
  printf("WHAT THIS SHOWS\n");
  printf("  updatecarbon(80) fixes intensity high. seturgency(LOW) allows deferral.\n");
  printf("  You should see some LARGE ticks_since_last: you were not running that whole time.\n\n");
  updatecarbon(80);
  seturgency(U_LOW);
  progress_lines("LOW", 18);
  printf("END LOW SECTION\n\n");

  /* [3] HIGH */
  banner("[3] HIGH urgency + same carbon (still 80)");
  printf("WHAT THIS SHOWS\n");
  printf("  HIGH urgency is not deferred by the carbon policy. Steps should track ticks\n");
  printf("  more tightly than LOW (fewer huge gaps).\n\n");
  seturgency(U_HIGH);
  progress_lines("HIGH", 18);
  printf("END HIGH SECTION (compare ticks_since_last to LOW block above)\n\n");

  /* Sequential child/parent */
  banner("[3][4] One process at a time: child LOW, then parent HIGH");
  printf("WHAT THIS SHOWS\n");
  printf("  Same high carbon. Child runs alone with pause(1) between lines = ~1 tick/step.\n");
  printf("  Parent runs after child exits. No garbled console. Compare child vs parent speed.\n\n");
  updatecarbon(82);
  pid = fork();
  if (pid == 0) {
    seturgency(U_LOW);
    printf("--- child (LOW), 25 lines, pause(1) each ---\n");
    for (i = 0; i < 25; i++) {
      printf("  child LOW  i=%d uptime=%d\n", i, uptime());
      pause(1);
    }
    exit(0);
  }
  wait(0);

  seturgency(U_HIGH);
  updatecarbon(82);
  printf("--- parent (HIGH), 25 lines, pause(1) each ---\n");
  for (i = 0; i < 25; i++) {
    printf("  parent HIGH i=%d uptime=%d\n", i, uptime());
    pause(1);
  }
  printf("END SEQUENTIAL SECTION\n\n");

  /* Deadline */
  banner("[3][4] Deadline while LOW + high carbon");
  printf("WHAT THIS SHOWS\n");
  printf("  LOW could defer you, but setdeadline() says run before tick D.\n");
  printf("  Kernel wakes deferred processes when the deadline is within slack ticks.\n");
  dl = uptime() + 40;
  printf("  Setting deadline at uptime tick %d (about 40 ticks from now).\n\n", dl);
  updatecarbon(80);
  seturgency(U_LOW);
  setdeadline(dl);
  for (i = 0; i < 12; i++) {
    printf("  line %d  uptime=%d  (deadline was %d)\n", i, uptime(), dl);
    pause(4);
  }
  setdeadline(0);
  printf("(First gap may be large if you were deferred; later lines every ~4 ticks from pause.)\n\n");

  updatecarbon(50);
  seturgency(U_MEDIUM);

  banner("Done");
  printf("  Carbon set back toward mid; urgency MEDIUM.\n");
  printf("  Commands: greentest   sh   carbon\n\n");
  exit(0);
}
