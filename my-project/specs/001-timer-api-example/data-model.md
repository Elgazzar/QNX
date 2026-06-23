# Data Model

## Entities

### `struct itimerspec`
- **Fields**:
  - `it_value`: Initial expiration time (`struct timespec`).
  - `it_interval`: Reload value for periodic timer (`struct timespec`).
- **Role**: Defines when the timer first fires and how often it repeats.

### `struct sigevent`
- **Fields**:
  - `sigev_notify`: Notification type (e.g., `SIGEV_PULSE`).
  - `sigev_coid`: Connection ID to send the pulse.
  - `sigev_value`: Optional value passed with the pulse.
  - `sigev_priority`: Priority of the pulse.
- **Role**: Defines the OS event to trigger upon timer expiration.
