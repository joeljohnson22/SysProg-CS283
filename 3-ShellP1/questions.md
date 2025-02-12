1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  _because it reads input line by line and automatically handles newline characters as delimiters. It prevents buffer overruns by allowing us to specify the maximum number of characters to read, and it works well in interactive shells as well as scripts when input might come from redirection or pipes._

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  _When using malloc() to allocate memory for cmd_buff, it allows for dynamic sizing if we need to adjust the buffer based on input requirements. This can help to avoid wasting stack space and gives flexibility in handling varying input sizes. In contrast, a fixed-size array limits the command length to a compile-time constant, which can be problematic if users enter very long lines or if the limits need to be adjusted later._


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  _ Trimmin spaces is needed because extra whitespace can lead to incorrect parsing of command tokens. For example, if leading spaces are not trimmed, the first token might include unwanted blank characters, causing errors when executing the command. Similarly, trailing spaces can result in extra empty arguments that might mislead the command parser or cause unexpected behavior when commands are executed in the shell._

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  _Standard Output Redirection (>): For example, ls > output.txt writes the output of ls into a file. Input Redirection (<): For example, sort < unsorted.txt takes input from a file instead of the keyboard. Appending Output (>>): For example, echo "new data" >> log.txt appends output to an existing file. Challenges I may face is handling multiple redirections in a single command and ensuring the order is correctly interpreted. Managing file permissions as well as cases when files do not exist or cannot be created. Adjusting the file descriptors properly so that the command receives the correct input/output streams._

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  _Redirection involves rerouting the input and output streams of a command to or from files (or other resources), whereas piping (|) takes the standard output of one command and feeds it directly as the standard input for another command._

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  _Keeping STDOUT and STDERR separate is important so that error messages (STDERR) do not become mixed with regular output (STDOUT). This separation makes it easier for users and programs to handle errors vs. normal output._

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  _Our custom shell should display error messages on STDERR so that they are clearly separated from the normal output. In cases where a command produces both STDOUT and STDERR, the shell could support options to merge them (for example, with a syntax similar to 2>&1) or allow the user to redirect them separately. This way gives users the flexibility needed for debugging and handling command failures._