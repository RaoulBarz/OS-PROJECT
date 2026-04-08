GreenOS

GreenOS extends xv6-riscv with a simulated carbon-intensity value in the kernel, a moving-average prediction over recent samples, and scheduler logic that may defer low-urgency processes when intensity is high while the global tick counter still advances. Deadlines and periodic wakeups limit starvation. Upstream xv6 is the MIT teaching OS for RISC-V; this repository adds the GreenOS coursework changes on top of that base.

Build with make, then run make qemu. At the xv6 shell you can run greentest for a guided demo, greenunittests for automated checks, and carbon for a small sampling loop. Exit QEMU with Ctrl-a then x.

Feature 1 is a kernel sensor with a lock and a ring-buffer history; see kernel/sensor.c, kernel/sensor.h, and HISTORY_SIZE in kernel/param.h. Feature 2 is the moving average forecast exposed as getpredicted in user space and get_predicted_carbon in the kernel. Feature 3 extends the process control block with urgency and deadline and applies should_defer in kernel/proc.c. Feature 4 implements the deferred queue: runnable processes may be moved to sleeping on DEFERRED_CHAN with wakeups from defer_periodic_check and related helpers. A longer file-by-file description is in GREENOS_TA_NOTES.md.

Each scheduler iteration runs sensor_update and defer_periodic_check. When a runnable process has low urgency and the carbon policy treats the environment as dirty enough, it may sleep on the deferred channel instead of running. Thresholds named GREENOS_* and history size are tunable in kernel/param.h.

Important paths include kernel/sensor.c and kernel/sensor.h for the sensor and prediction, kernel/proc.c and kernel/proc.h for deferral and scheduling, kernel/sysproc.c and kernel/syscall.c and kernel/syscall.h for system calls, user/user.h and user/usys.pl for the user stubs, and user/greentest.c, user/greenunittests.c, and user/carbon.c for demos and tests.

New system calls share the same tick domain as uptime where applicable: getcarbon reads the current value, updatecarbon sets a clamped value, getpredicted returns the moving average, seturgency takes LOW, MEDIUM, or HIGH (0, 1, 2 as in enum urgency_level in kernel/proc.h), setdeadline sets an absolute tick deadline or zero to clear, and pause sleeps for about n ticks.
