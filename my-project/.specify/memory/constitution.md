<!--
Sync Impact Report:
- Version change: [NEW_INIT] → 1.0.0
- Modified principles: N/A (Initial Setup)
- Added sections: Core Principles, Example Structure Requirements, Development Workflow, Governance
- Removed sections: N/A
- Templates requiring updates: 
  ✅ .specify/templates/plan-template.md
  ✅ .specify/templates/spec-template.md
  ✅ .specify/templates/tasks-template.md
  ✅ .specify/templates/commands/*.md
- Follow-up TODOs: N/A
-->
# QNX Adaptive AUTOSAR Learning Repository Constitution

## Core Principles

### I. Example-Driven Learning
Each QNX concept MUST be applied as an independent, runnable example.
**Rationale**: Practical application is the most effective way to learn system programming for adaptive Autosar.

### II. Consistent Folder Structure
Every example MUST reside in its own dedicated folder.
**Rationale**: Keeps examples isolated and prevents cross-contamination of code, making it easier for learners to focus on one concept at a time.

### III. Mandatory Documentation
Every example folder MUST contain a `.md` file describing the example's usage and the specific APIs utilized.
**Rationale**: Ensures that the intent, usage instructions, and the underlying QNX/Autosar concepts are clearly understood without needing to parse the code immediately.

### IV. Adaptive AUTOSAR Focus
Examples SHOULD primarily target features and APIs relevant to QNX for Adaptive AUTOSAR.
**Rationale**: The repository's main purpose is to build competency specifically in Adaptive AUTOSAR over QNX.

## Example Structure Requirements

- **Example Folder Naming**: Descriptive and consistent (e.g., `01-thread-creation`, `02-ipc-message-queues`).
- **Documentation (`README.md` or `usage.md`)**: Must include a brief description, API list, compilation instructions, and execution instructions.
- **Code Comments**: Complex QNX APIs must be annotated with explanatory comments.

## Development Workflow

- **Creating a new example**: Create a folder, write the code, write the `.md` documentation.
- **Review**: Ensure documentation accurately reflects the code and specifically calls out QNX/Adaptive AUTOSAR APIs.

## Governance

- All additions must adhere to the folder structure and documentation requirements.
- PRs without the accompanying `.md` documentation will not be merged.

**Version**: 1.0.0 | **Ratified**: 2026-06-23 | **Last Amended**: 2026-06-23
