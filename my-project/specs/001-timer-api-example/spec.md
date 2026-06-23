# Feature Specification: QNX Timer API Example

**Feature Branch**: `001-timer-api-example`

**Created**: 2026-06-23

**Status**: Draft

**Input**: User description: "create an example on timer which includes ClockPeriod API and what kind of timer periodic or one shot, what clock drive the timer normal system time or increasing timer, inital fire time absloute or relative, timer reload an repeat value event to deliver upon trigger"

## Clarifications

### Session 2026-06-23
- Q: Should specific timer APIs like timer_create, timer_delete, and timer_settime be explicitly described in the documentation? → A: Yes, explicitly describe timer_create, timer_delete, and timer_settime.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Configure and Start a Timer (Priority: P1)

As a learner, I want an example that configures and starts a QNX timer using the `ClockPeriod` API so that I can understand how to initialize a timer with specific characteristics.

**Why this priority**: Timer initialization is the foundational step for any timed behavior in QNX systems.

**Independent Test**: The example can be compiled and executed to successfully print out the timer configuration and start without errors.

**Acceptance Scenarios**:

1. **Given** the timer example source code, **When** compiled and executed, **Then** it successfully invokes `ClockPeriod` to set or get the clock resolution.
2. **Given** the timer initialization parameters, **When** configuring the timer, **Then** it clearly defines whether it's driven by `CLOCK_REALTIME` or `CLOCK_MONOTONIC`.
3. **Given** the timer setup, **When** specifying the initial fire time, **Then** it explicitly defines whether the time is absolute or relative.

---

### User Story 2 - Handle Timer Expiration Events (Priority: P2)

As a learner, I want the example to demonstrate event delivery upon trigger (e.g., using a signal or pulse) so that I know how the system notifies the application when the timer expires.

**Why this priority**: Starting a timer is useless if the application doesn't know how to handle the expiration event.

**Independent Test**: The application receives and processes the timer event, logging the occurrence to standard output.

**Acceptance Scenarios**:

1. **Given** a running timer, **When** the timer fires, **Then** the application receives a predefined event (e.g., pulse or signal) and handles it appropriately.
2. **Given** a periodic timer setup, **When** the reload value is non-zero, **Then** the application receives multiple continuous events at the specified interval.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The example MUST include the use of the `ClockPeriod` API to query or set the system clock resolution.
- **FR-002**: The example MUST configure a timer, explicitly demonstrating whether it is periodic (continuous) or one-shot.
- **FR-003**: The example MUST specify the driving clock, choosing between `CLOCK_REALTIME` (normal system time) and `CLOCK_MONOTONIC` (increasing timer).
- **FR-004**: The example MUST define the initial fire time, distinguishing whether it is an absolute time or a relative delay.
- **FR-005**: The example MUST configure a timer reload/repeat value for periodic behavior.
- **FR-006**: The example MUST set up an event (e.g., `sigevent` structure) to be delivered to the application upon the timer's trigger.
- **FR-007**: The example documentation MUST explicitly describe the `timer_create`, `timer_settime`, and `timer_delete` APIs.

### Key Entities

- **Timer Configuration (struct itimerspec)**: Defines the initial expiration time and the reload interval.
- **Event Notification (struct sigevent)**: Defines how the OS notifies the process (e.g., pulse delivery or signal).

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: The example source code successfully compiles without errors on a QNX toolchain.
- **SC-002**: Upon execution, the example prints the clock period information.
- **SC-003**: Upon execution, the example successfully receives at least one timer expiration event and prints a notification to the console.
- **SC-004**: The documentation (`.md` file) clearly explains the APIs used (`ClockPeriod`, `timer_create`, `timer_settime`) and their parameters.

## Assumptions

- The example is intended to be run in a terminal/console environment to print outputs.
- The default event delivery mechanism used for demonstration will be a pulse (common in QNX) unless specified otherwise.
