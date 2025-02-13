# Shell Questions

## 1. Why is `fgets()` a good choice?
> It prevents buffer overflow, handles newlines, and detects EOF, making it ideal for reading user input safely.

## 2. Why use `malloc()` for `cmd_buff` instead of a fixed-size array?
> It allows dynamic memory allocation, preventing wasted space or truncation issues with fixed-size buffers.

## 3. Why trim spaces in `build_cmd_list()`?
> Ensures correct command parsing; avoids execution issues due to unnecessary whitespace.

## 4. Redirection examples & challenges:
- **`ls > output.txt` (redirect stdout)**: Requires detecting `>` and redirecting output.
- **`sort < input.txt` (redirect stdin)**: Needs file reading before command execution.
- **`gcc program.c 2> errors.txt` (redirect stderr)**: Must handle error output separately.

## 5. Difference between redirection & piping:
> Redirection deals with files (e.g., `ls > file.txt`), while piping (`|`) connects commands (`ls | grep .c`).

## 6. Why separate STDERR from STDOUT?
> Keeps error messages independent for debugging and avoids mixing errors with normal output.

## 7. Handling errors in the shell:
> Check `execvp()` return values, print errors to `STDERR`, and allow merging `STDOUT` and `STDERR` via `2>&1` using `dup2()`.
