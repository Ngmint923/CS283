## Assignment 2 Questions - Answers

### 1. Is `get_student(...)` a good design choice?
> It is a good design choice because it improves code organization, reduces duplication, and makes the program easier to maintain by centralizing student lookups

### 2. Why is returning a local `student_t` bad?
> The function returns a pointer to a local variable, which is deleted when the function exits. Using that pointer later will cause errors or crashes

### 3. Why is using `malloc()` in `get_student()` risky?
> It works but can cause memory leaks.** If the caller forgets to free the memory, the program will use more and more RAM, leading to slower performance or crashes

### 4. Why does `ls` show different sizes than `du`?
- `ls` shows logical file size (including empty spaces from `lseek()`)
- `du` shows actual disk usage, which grows only when real data is written

### 5. Why didn't storage increase until ID=64?
> Linux uses sparse files. The file had holes (empty spaces), so storage wasn’t used until ID=64 filled a new block

### 6. Why did adding ID=99999 make `ls` show 6.4MB but `du` only 12K?
> A huge hole was created in the file. `ls` sees the highest offset used, but Linux doesn’t allocate disk space for empty regions, so `du` reports only real storage used
