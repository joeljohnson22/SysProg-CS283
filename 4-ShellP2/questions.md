1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  _Using fork creates a new child process. The child then calls execvp to replace its image with the external command. This approach lets the parent process (the shell) continue running and remain in control while the command executes in a separate process. Without forking, the shell itself would be replaced by the command, terminating the shell._

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  _If the fork() system call fails, it returns a negative value. In my implementation, 
    I check for a negative return from fork and print an error message (using perror) so that the user is informed, then continue the loop. This prevents the shell from crashing when a fork cannot be created._

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  _The execvp() function searches for the command in the directories listed in the PATH environment variable. This variable contains a colon-separated list of directories, and execvp() checks each in turn until it locates an executable file that matches the given command._

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  _Calling wait() or waitpid() in the parent process causes it to block until the child process completes. This is important to avoid zombie processes and to allow the shell to retrieve the exit status of the command. Without waiting, the parent might continue running and reap no information about the child’s termination, or cause resource leaks._

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  _The WEXITSTATUS() macro extracts the exit status from the status value returned by wait()/waitpid(). This status indicates how the child process terminated (i.e. success or error code). Knowing the exit status is important for debugging and for scripts that depend on the outcome of executed commands._

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  _In my implementation, when a token begins with a double quote, the parser collects characters until the closing quote. This means that whitespace within the quotes is preserved as part of a single argument. This is necessary so that commands like echo "hello world" treat the content inside the quotes as one argument, preserving the intended grouping._

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  _Previously, the parsing logic handled multiple commands separated by pipes. In this assignment, the focus is on a single command buffer. I refactored the code to handle only one command line input at a time instead of splitting by pipe characters. Also, added logic to correctly identify quoted strings and preserve embedded spaces while collapsing extra white spaces elsewhere. An unexpected challenge was ensuring that the quoting mechanism worked consistently across different input scenarios while still eliminating duplicate spaces outside of quotes._

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  _Signals are a mechanism by which processes receive asynchronous notifications from the kernel or other processes. They differ from other IPC methods (like pipes, message queues, shared memory) in that signals are interrupt-based, are used to notify processes about events (e.g., termination requests, timer expirations), and do not carry data payloads except for minimal information (like the signal number)._

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  _SIGKILL is a non-catchable, non-ignorable signal that immediately terminates a process. It’s used when a process must be terminated unconditionally.
SIGTERM is a termination signal that can be caught or ignored. It allows processes to perform cleanup before exiting. It is the default signal sent by commands like kill without options.
SIGINT is issued typically when the user presses Ctrl+C in the terminal, this signal interrupts a process. Processes can catch it to allow for graceful shutdown or to ignore it temporarily._

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  _Its suspended by the kernel. Unlike signals such as SIGINT, SIGSTOP cannot be caught, blocked, or ignored. This behavior ensures that processes can always be forcefully paused, which is important for system control._
