## Fork/Exec and Process Management Questions

### 1. Why use `fork/execvp` instead of just `execvp`?
> **Answer**: fork() creates a child process, allowing the parent to continue running independently. Without fork(), execvp() would replace the current process, terminating the shell

### 2. What happens if `fork()` fails?
> **Answer**: If fork() fails, no child process is created due to resource exhaustion. The implementation should check the return value and print an error message if fork() returns -1

### 3. How does `execvp()` find the command?
> **Answer**: execvp() searches for the command in directories listed in the PATH environment variable

### 4. Purpose of `wait()` after forking?
> **Answer**: wait() prevents zombie processes by ensuring the parent process collects the child's exit status. Without wait(), terminated child processes remain in the process table

### 5. What does `WEXITSTATUS()` provide?
> **Answer**: It extracts the exit code of a terminated child process, allowing the parent to determine if execution was successful or if errors occurred

### 6. Handling quoted arguments in `build_cmd_buff()`?
> **Answer**: It treats text inside quotes as a single argument, preserving spaces. This ensures commands like echo "hello world" pass "hello world"` as one argument instead of two.

### 7. Parsing changes and challenges?
> **Answer**: The new parsing logic handles quoted arguments and trims extra spaces. Challenges included ensuring compatibility with pipes and edge cases in command formatting.

### Linux Signals Questions

* 8. Purpose of signals in Linux?
> **Answer**: Signals provide asynchronous communication between processes, allowing tasks like termination, pausing, or handling specific events.

* 9. Three common signals and their use cases?
> **Answer**:
> - **SIGKILL (9)**: Forces a process to terminate immediately (cannot be caught or ignored).
> - **SIGTERM (15)**: Politely requests a process to terminate, allowing cleanup.
> - **SIGINT (2)**: Interrupts a process, typically sent when pressing Ctrl+C

* 10. What happens when a process receives `SIGSTOP`?
> **Answer**: SIGSTOP pauses a process until SIGCONT is received. Unlike SIGINT, it **cannot** be caught or ignored, ensuring reliable process suspension






