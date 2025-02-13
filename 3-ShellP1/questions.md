In this assignment I suggested you use fgets() to get user input in the main while loop. Why is fgets() a good choice for this application?
Answer:
fgets() ensures safe input handling by preventing buffer overflows and allowing multi-word input, unlike scanf(), which stops at whitespace. It properly handles EOF (Ctrl+D) and ensures predictable behavior when reading user input.

You needed to use malloc() to allocate memory for cmd_buff in dsh_cli.c. Can you explain why you needed to do that, instead of allocating a fixed-size array?
Answer:
Using malloc() allows dynamic memory allocation, making the shell flexible to handle varying input sizes. A fixed-size array could either waste memory if too large or risk buffer overflow if too small. Dynamic allocation ensures efficient memory use.

In dshlib.c, the function build_cmd_list() must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?
Answer:
Trimming spaces ensures accurate command parsing by removing unintended spaces that could cause execution errors. Without trimming, commands might be misinterpreted, arguments could shift incorrectly, and piped commands may not function as expected.

For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google "linux shell stdin stdout stderr explained" to get started.
Answer:
Linux shells manage three standard streams: STDIN (0) for input (keyboard), STDOUT (1) for normal command output, and STDERR (2) for errors. Keeping them separate allows better control over redirection and logging.

One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.
Answer:
Output redirection (> and >>) stores command output in a file. Handling append (>>) requires ensuring file access modes are correct.
Input redirection (<) allows reading from a file instead of STDIN. We must properly open and close files to prevent resource leaks.
Error redirection (2>) redirects error messages to a file, requiring separate handling of STDERR from STDOUT.

You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.
Answer:
Redirection controls where input and output go, typically between files and commands, while piping (|) connects the output of one command to the input of another, enabling command chaining in a single execution.

STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?
Answer:
Separating STDERR from STDOUT ensures error messages don’t interfere with expected output, allowing proper logging, debugging, and redirection without affecting normal command results.

How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?
Answer:
The shell should capture exit codes and print error messages from STDERR while keeping STDOUT separate. To merge outputs, users can use 2>&1 to redirect STDERR to STDOUT, ensuring errors appear in the expected output stream when needed.