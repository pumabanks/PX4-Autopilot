uint64 timestamp                 # time since system start (microseconds)

bool enabled                     # true (1) if the SuperAwesomeModule enabled
uint8 update_hz                  # configured update rate in Hz
uint32 update_interval_ms        # configured update interval in ms

float32 HIGH_CPU_LOAD_WARNING = 0.85       # threshold for high cpu load warning

uint32 WARN_MESSAGE_INTERVAL_US = 5000000   # warning message interval sent at this interval (in micro seconds)

bool warning_high_cpu_load       # actively warning user about cpu usage
