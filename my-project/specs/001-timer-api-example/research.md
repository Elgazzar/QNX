# Phase 0: Outline & Research

## Decision: Chosen Timer APIs
- **Decision**: Use standard POSIX timer APIs (`timer_create`, `timer_settime`, `timer_delete`) and QNX specific `ClockPeriod`.
- **Rationale**: Strict compliance to feature requirements and QNX Adaptive AUTOSAR best practices where standard POSIX APIs are preferred for portability, while `ClockPeriod` is required for clock resolution queries specific to QNX.
- **Alternatives considered**: `nanosleep` or `delay()` functions (rejected because they block the thread and do not set up event-driven periodic timers).

## Decision: Event Delivery Mechanism
- **Decision**: Use QNX Pulses via `MsgReceive` with a `sigevent` structure.
- **Rationale**: Pulses are the most efficient, non-blocking notification mechanism in QNX, heavily utilized in system programming over signals which carry more overhead.
- **Alternatives considered**: POSIX Signals (rejected due to higher context switching overhead and being less idiomatic for modern QNX architectures).
