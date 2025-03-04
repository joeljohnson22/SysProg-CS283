1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

_My implementation calls waitpid() in a loop for each child process ID stored in the pids array. This ensures all child processes complete before the shell returns to the command prompt. Without waitpid(), orphaned processes would become zombies, consuming system resources and potentially causing a resource leak. The shell would also continue before command execution finished, leading to unpredictable behavior and incorrect output._

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

_Closing unused pipe ends after dup2() is necessary because open file descriptors prevent EOF from being detected. If reading ends remain open, processes waiting for EOF will hang indefinitely. For example, in a pipeline like "cmd1 | cmd2", if cmd1's read end remains open, cmd2 will never receive EOF even when cmd1 terminates. This creates deadlocks and resource leaks as processes wait for input that will never arrive._

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

_The cd command must be a shell built-in because it needs to change the current working directory of the shell process itself. If implemented as an external command, it would change the directory in the child process only, then terminate, leaving the parent shell's directory unchanged. This happens because each process has its own current working directory, and changes in a child process don't affect the parent after the child exits._

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

_To support arbitrary piped commands, I would replace the fixed arrays with dynamically allocated data structures like linked lists or dynamically sized arrays. Initially, I'd allocate a reasonable default size, then reallocate when needed. Trade offs include more complex memory management with potential for leaks, additional overhead from dynamic allocation/reallocation, and the need to handle allocation failures gracefully. However, this approach would provide flexibility to handle commands of any complexity while still maintaining reasonable performance for typical use cases._
