# Tasks: QNX Timer API Example

**Input**: Design documents from `/specs/001-timer-api-example/`

**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, quickstart.md

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2)
- Include exact file paths in descriptions

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and basic structure

- [X] T001 Create `Timer_API_Example/src` directory structure
- [X] T002 Create initial `Timer_API_Example/Makefile` for QNX compilation

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [X] T003 Create `Timer_API_Example/src/Timer_API_Example.cpp` with basic `main()` function and includes (`<time.h>`, `<sys/siginfo.h>`, `<sys/neutrino.h>`)

**Checkpoint**: Foundation ready - user story implementation can now begin

---

## Phase 3: User Story 1 - Configure and Start a Timer (Priority: P1) 🎯 MVP

**Goal**: As a learner, I want an example that configures and starts a QNX timer using the `ClockPeriod` API so that I can understand how to initialize a timer with specific characteristics.

**Independent Test**: The example can be compiled and executed to successfully print out the timer configuration and start without errors.

### Implementation for User Story 1

- [X] T004 [US1] Implement `ClockPeriod` query to get/set clock resolution in `Timer_API_Example/src/Timer_API_Example.cpp`
- [X] T005 [US1] Define `struct itimerspec` setup (absolute vs relative, periodic interval) in `Timer_API_Example/src/Timer_API_Example.cpp`
- [X] T006 [US1] Implement `timer_create` initialization in `Timer_API_Example/src/Timer_API_Example.cpp`
- [X] T007 [US1] Implement `timer_settime` call to arm the timer in `Timer_API_Example/src/Timer_API_Example.cpp`

**Checkpoint**: At this point, User Story 1 should be fully functional and testable independently (the timer starts, though we might not yet wait for its events).

---

## Phase 4: User Story 2 - Handle Timer Expiration Events (Priority: P2)

**Goal**: As a learner, I want the example to demonstrate event delivery upon trigger (e.g., using a signal or pulse) so that I know how the system notifies the application when the timer expires.

**Independent Test**: The application receives and processes the timer event, logging the occurrence to standard output.

### Implementation for User Story 2

- [X] T008 [US2] Create a channel and connect using `ChannelCreate` and `ConnectAttach` in `Timer_API_Example/src/Timer_API_Example.cpp`
- [X] T009 [US2] Update `timer_create` to use `SIGEV_PULSE_INIT` with `sigevent` to link the pulse to the channel in `Timer_API_Example/src/Timer_API_Example.cpp`
- [X] T010 [US2] Implement a `MsgReceive` loop to catch pulse events in `Timer_API_Example/src/Timer_API_Example.cpp`
- [X] T011 [US2] Implement `timer_delete` on exit condition (e.g., after 5 pulses) in `Timer_API_Example/src/Timer_API_Example.cpp`

**Checkpoint**: At this point, User Stories 1 AND 2 should both work independently and the timer prints periodic messages correctly.

---

## Phase 5: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories

- [X] T012 [P] Create `Timer_API_Example/Timer_API_Usage.md` documentation detailing `timer_create`, `timer_settime`, `timer_delete`, and `ClockPeriod` APIs
- [X] T013 Run compilation test via `make` and execute `./Timer_API_Example` as described in `quickstart.md`
- [X] T014 Code cleanup and add explanatory comments for all QNX-specific calls

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - BLOCKS all user stories
- **User Stories (Phase 3+)**: Depend on Foundational phase completion
- **Polish (Final Phase)**: Depends on all desired user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Starts after Foundational (Phase 2).
- **User Story 2 (P2)**: Builds upon User Story 1 by adding the event handling loop and pulse setup.

### Parallel Opportunities

- All Setup tasks marked [P] can run in parallel (none currently).
- Documentation (T012) can run in parallel with compilation testing and code cleanup (T013, T014) in the final phase.

---

## Implementation Strategy

### Incremental Delivery

1. Complete Setup + Foundational → Foundation ready
2. Add User Story 1 → Initialize the timer
3. Add User Story 2 → Receive pulses and cleanup
4. Polish → Verify and document
5. Each story adds value without breaking previous stories
