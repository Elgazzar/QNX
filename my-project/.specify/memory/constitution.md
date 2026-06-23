<!--
Sync Impact Report:
- Version change: 1.1.2 → 1.1.3
- Modified principles: N/A
- Added sections: N/A
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
Every example folder MUST contain a `.md` file describing the example's usage, an explanation of how the example works conceptually, and the specific APIs utilized.
**Rationale**: Ensures that the intent, usage instructions, and the underlying QNX/Autosar concepts are clearly understood without needing to parse the code immediately.

### IV. Adaptive AUTOSAR Focus
Examples SHOULD primarily target features and APIs relevant to QNX for Adaptive AUTOSAR.
**Rationale**: The repository's main purpose is to build competency specifically in Adaptive AUTOSAR over QNX.

### V. IDE & Build Tooling
All examples MUST be built and compiled using the QNX Momentics IDE.
**Rationale**: Using the officially supported IDE ensures a standardized development environment, guarantees compatibility with QNX toolchains, and allows examples to leverage IDE-generated configurations (like `.cproject` and `Makefile`s).

## Example Structure Requirements

- **Example Folder Naming**: Descriptive and consistent (e.g., `01-thread-creation`, `02-ipc-message-queues`).
- **Documentation (`README.md` or `usage.md`)**: Must include a brief description, an explanation of the underlying concepts and how the example works, an API list, compilation instructions, and execution instructions.
- **Code Comments**: Every single line of code in the example MUST have a comment before it that describes exactly what the code does.

## Development Workflow

- **Creating a new example**: Create a folder, write the code, write the `.md` documentation.
- **Repository Catalog**: The root `README.md` MUST be updated to include a description of the new example and a link to its documentation.
- **Review**: Ensure documentation accurately reflects the code and specifically calls out QNX/Adaptive AUTOSAR APIs.

## Governance

- All additions must adhere to the folder structure and documentation requirements.
- PRs without the accompanying `.md` documentation will not be merged.

**Version**: 1.1.3 | **Ratified**: 2026-06-23 | **Last Amended**: 2026-06-23
