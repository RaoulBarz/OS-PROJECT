#include "kernel/types.h"
#include "user/user.h"

enum { U_LOW, U_MEDIUM, U_HIGH };

static int failures = 0;

static void
ok(const char *name)
{
  printf("[PASS] %s\n", name);
}

static void
fail(const char *name, int a, int b)
{
  printf("[FAIL] %s (a=%d b=%d)\n", name, a, b);
  failures++;
}

static void
test_feature1_sensor_bounds(void)
{
  int c;

  updatecarbon(-999);
  c = getcarbon();
  if (c >= 0 && c <= 5)
    ok("feature1: clamp low bound");
  else
    fail("feature1: clamp low bound", c, 0);

  updatecarbon(999);
  c = getcarbon();
  if (c >= 95 && c <= 100)
    ok("feature1: clamp high bound");
  else
    fail("feature1: clamp high bound", c, 100);
}

static void
test_feature1_manual_set_roundtrip(void)
{
  int c;

  updatecarbon(37);
  c = getcarbon();
  if (c == 37)
    ok("feature1: manual updatecarbon roundtrip");
  else
    fail("feature1: manual updatecarbon roundtrip", c, 37);
}

static void
test_feature2_prediction_trend(void)
{
  int i, p;

  for (i = 0; i < 8; i++)
    updatecarbon(100);
  p = getpredicted();
  if (p >= 80)
    ok("feature2: predicted rises after high samples");
  else
    fail("feature2: predicted rises after high samples", p, 80);

  for (i = 0; i < 8; i++)
    updatecarbon(0);
  p = getpredicted();
  if (p <= 20)
    ok("feature2: predicted falls after low samples");
  else
    fail("feature2: predicted falls after low samples", p, 20);
}

static int
elapsed_ticks_for_pause_loop(int loops)
{
  int i;
  int start = uptime();

  for (i = 0; i < loops; i++)
    pause(1);

  return uptime() - start;
}

static void
test_feature2_prediction_midpoint(void)
{
  int i, p;

  for (i = 0; i < 10; i++)
    updatecarbon(100);
  for (i = 0; i < 10; i++)
    updatecarbon(0);

  updatecarbon(10);
  updatecarbon(20);
  updatecarbon(30);
  updatecarbon(40);
  updatecarbon(50);
  p = getpredicted();
  if (p >= 28 && p <= 32)
    ok("feature2: moving average matches known sequence");
  else
    fail("feature2: moving average matches known sequence", p, 30);
}

static void
test_feature3_urgency_policy(void)
{
  int low_elapsed = 0, high_elapsed = 0, trial;

  updatecarbon(85);
  for (trial = 0; trial < 3; trial++) {
    seturgency(U_LOW);
    low_elapsed += elapsed_ticks_for_pause_loop(12);
    seturgency(U_HIGH);
    high_elapsed += elapsed_ticks_for_pause_loop(12);
  }

  if (low_elapsed >= high_elapsed + 2)
    ok("feature3: LOW takes longer than HIGH at high carbon");
  else
    fail("feature3: LOW takes longer than HIGH at high carbon", low_elapsed, high_elapsed);
}

static void
test_feature4_deadline_wakeup(void)
{
  int pid;
  int deadline;
  int start;
  int status = -1;
  int done = 0;

  updatecarbon(85);
  pid = fork();
  if (pid < 0) {
    fail("feature4: fork child", pid, 0);
    return;
  }

  if (pid == 0) {
    int i;
    seturgency(U_LOW);
    deadline = uptime() + 35;
    setdeadline(deadline);
    for (i = 0; i < 8; i++)
      pause(4);
    setdeadline(0);
    exit(0);
  }

  start = uptime();
  while (uptime() - start < 140) {
    if (wait(&status) == pid) {
      done = 1;
      break;
    }
  }

  if (done && status == 0)
    ok("feature4: deferred task wakes and completes before timeout");
  else {
    kill(pid);
    fail("feature4: deferred task wakes and completes before timeout", done, status);
  }
}

static void
test_feature4_low_progress_under_starvation_guard(void)
{
  int start, end;

  updatecarbon(85);
  seturgency(U_LOW);
  start = uptime();
  pause(220);
  end = uptime();

  if (end - start >= 200)
    ok("feature4: LOW process still makes progress over time");
  else
    fail("feature4: LOW process still makes progress over time", end - start, 200);
}

int
main(void)
{
  printf("greenunittests: start\n");

  test_feature1_sensor_bounds();
  test_feature1_manual_set_roundtrip();
  test_feature2_prediction_trend();
  test_feature2_prediction_midpoint();
  test_feature3_urgency_policy();
  test_feature4_deadline_wakeup();
  test_feature4_low_progress_under_starvation_guard();

  seturgency(U_MEDIUM);
  updatecarbon(50);
  setdeadline(0);

  if (failures == 0) {
    printf("greenunittests: ALL TESTS PASSED\n");
    exit(0);
  }

  printf("greenunittests: FAILED (%d)\n", failures);
  exit(1);
}
