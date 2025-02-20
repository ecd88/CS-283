1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**: If we just called execvp(), it would replace our shell with whatever command we’re running, which is not what we want. fork() creates a separate process for the command, so our shell can keep running and take more input after the command finishes. It basically keeps things from breaking.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**: If fork() fails, it usually means the system is out of resources, like memory or process slots. In my code, I check if fork() returns -1, and if it does, I print an error message with perror("fork failed") and return an error code. This way, the shell doesn’t crash, and we know something went wrong.

3. How does `execvp()` find the command to execute? What system environment variable plays a role in this process?

    > **Answer**: execvp() looks for the command in the directories listed in the PATH environment variable. It basically goes through each directory in PATH, checking if the command exists there. If it finds it, it runs it; if not, it throws an error.

4. What is the purpose of calling `wait()` in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**: wait() makes sure the parent (our shell) waits for the child process (the command) to finish before moving on. If we didn’t call it, the child process would finish and turn into a zombie, which just wastes system resources until it gets cleaned up.

5. In the referenced demo code we used `WEXITSTATUS()`. What information does this provide, and why is it important?

    > **Answer**: WEXITSTATUS() grabs the exit code of the child process after it’s done. This tells us whether the command ran successfully or failed. It’s useful because some programs return different exit codes for different errors, so we can check what happened.

6. Describe how your implementation of `build_cmd_buff()` handles quoted arguments. Why is this necessary?

    > **Answer**: It makes sure that stuff inside quotes stays together as a single argument. Without this, something like "hello world" would be treated as two separate arguments instead of one, which would mess up how commands are run.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**: I had to tweak the way I handle spaces and quotes to make sure arguments get parsed correctly. One tricky part was handling edge cases, like weird spacing or multiple quotes in a row. Also, refactoring older code is always a bit annoying because you have to untangle stuff you wrote earlier.

8. For this question, you need to do some research on Linux signals. 

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**: Signals are like little messages the OS sends to processes to tell them something happened. They’re different from other IPC methods like pipes or shared memory because they’re more like interrupts—they just happen, and the process has to deal with them right away.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:
    > 1. SIGKILL (9) – Instantly kills a process. The process can’t ignore it, and it doesn’t get a chance to clean up.
    > 2. SIGTERM (15) – Nicely asks a process to shut down. It can be ignored or handled, so the process has a chance to clean up before exiting.
    > 3. SIGINT (2) – Sent when you hit `Ctrl+C` in the terminal. It’s a way to interrupt a running process.

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**: SIGSTOP completely pauses a process, and unlike SIGINT, the process can’t ignore or handle it. It just freezes until you send SIGCONT to resume it. The OS forces it to stop, no matter what.