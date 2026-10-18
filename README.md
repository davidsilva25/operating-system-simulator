# Operating System & Process Lifecycle Simulator

A discrete-event Operating System simulator implemented in C. This project simulates low-level process and thread lifecycle management, Round-Robin CPU scheduling with time-slice interrupts, an execution engine for numerical instructions, and a paged virtual memory management system.

---

## 📌 Architectural Overview & Features

- **Process & Thread Lifecycle:** Implements a full 5-state lifecycle model (**NEW**, **READY**, **RUN**, **BLOCKED**, and **EXIT**).
- **CPU Scheduling:** Preemptive **Round-Robin** algorithm with time quantum enforcement.
- **Instruction Execution Engine:** Interprets and executes textual assembly-like instructions converted into internal numerical opcodes.
- **Memory Management:** Paged virtual memory system ensuring process isolation and bounded addressing.
- **Concurrency & Hierarchy:** Strict parent-child thread hierarchy management, automatic thread teardown, and isolated variable scope.
- **Deterministic Simulation:** Real-time tracking and logging of 100 discrete simulation clock ticks.

---

## 🛠️ Tech Stack & Tools

- **Language:** C (C99/C11 standard)
- **Build System:** GNU Make (`Makefile`)
- **Environment:** POSIX / Linux

---
