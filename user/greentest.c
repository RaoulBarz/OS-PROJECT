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
 * progress_lines: one print per scheduling round-trip via pause(1).
 * ticks_since_last = timer ticks between prints (often ~1 on HIGH; larger on LOW when deferred).
 * The old busy-spin printed many lines inside one tick, hiding deferral.
 */
static void
progress_lines(const char *tag, int n)
{
  int i;
  int prev_uptime = -1;

  if (n > 1) {
    printf("  Legend: uptime = global tick counter. ticks_since_last = uptime - previous uptime.\n");
    printf("  Large ticks_since_last on LOW urgency = time spent NOT running (deferral).\n\n");
  }

  for (i = 0; i < n; i++) {
    int t = uptime();
    printf("  %s step %d  uptime=%d", tag, i, t);
    if (prev_uptime >= 0)
      printf("  ticks_since_last=%d", t - prev_uptime);
    printf("\n");
    prev_uptime = t;
    pause(1);
  }
}

int
main(int argc, char *argv[])
{
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
  printf("  updatecarbon(80) sets high intensity; seturgency(LOW) may be deferred under policy.\n");
  printf("  Two lines: step 1 shows ticks_since_last after pause(1) between prints.\n\n");
  updatecarbon(80);
  seturgency(U_LOW);
  progress_lines("LOW", 2);
  printf("END LOW SECTION\n\n");

  /* [3] HIGH */
  banner("[3] HIGH urgency + same carbon (still 80)");
  printf("WHAT THIS SHOWS\n");
  printf("  Same carbon; HIGH urgency is not deferred the same way as LOW.\n");
  printf("  Two lines: compare step 1 ticks_since_last to the LOW block.\n\n");
  seturgency(U_HIGH);
  progress_lines("HIGH", 2);
  printf("END HIGH SECTION (compare step 1 ticks_since_last to LOW)\n\n");

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
