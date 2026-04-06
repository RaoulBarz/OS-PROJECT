#define NPROC        64  // maximum number of processes
#define NCPU          8  // maximum number of CPUs
#define NOFILE       16  // open files per process
#define NFILE       100  // open files per system
#define NINODE       50  // maximum number of active i-nodes
#define NDEV         10  // maximum major device number
#define ROOTDEV       1  // device number of file system root disk
#define MAXARG       32  // max exec arguments
#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define LOGBLOCKS    (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF         (MAXOPBLOCKS*3)  // size of disk block cache
#define FSSIZE       2000  // size of file system in blocks
#define MAXPATH      128   // maximum file path name
#define USERSTACK    1     // user stack pages
#define HISTORY_SIZE 5     // size of carbon history buffer

// GreenOS: carbon-aware scheduling thresholds (tunable)
#define CARBON_MIN 0
#define CARBON_MAX 100
#define GREENOS_HIGH_CARBON 70      // defer LOW urgency at or above this
#define GREENOS_PRED_DROP_GAP 5     // defer if current exceeds predicted by this
#define GREENOS_PRED_RISE_GAP 8     // wake early: predicted above current by this
#define GREENOS_DEFER_FLOOR 50      // min current for "drop coming" defer rule
#define GREENOS_WAKE_LOW_CARBON 55  // wake early when current is below this
#define GREENOS_CARBON_OK_BELOW 55  // wake when current drops below this (clean enough)
#define GREENOS_STARVE_INTERVAL 200 // periodic wake of deferred tasks (ticks)
#define GREENOS_DEADLINE_SLACK 25   // run before deadline within this many ticks

