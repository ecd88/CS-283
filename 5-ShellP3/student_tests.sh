#!/usr/bin/env bats

# ===============================
# student_tests.sh for dsh
# ===============================

# 1 Test: Basic `ls` command
@test "Basic ls command" {
    run "./dsh" <<EOF
ls
exit
EOF
    echo "Captured output: $output"  # Debugging output
    [ "$status" -eq 0 ]
    [[ "$output" =~ "dshlib.c" ]] || [[ "$output" =~ "makefile" ]]  # Look for expected file
}

# 2 Test: Running `pwd`
@test "Check present working directory" {
    run "./dsh" <<EOF
pwd
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "/" ]]  # Expecting a valid path output
}

# 3 Test: `cd` to parent directory
@test "Change directory to parent" {
    run "./dsh" <<EOF
cd ..
pwd
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "/" ]]  # Expecting output of pwd after changing directory
}

# 4 Test: `ls | grep ".c"` (Basic Piping)
@test "Pipe ls output to grep .c" {
    run "./dsh" <<EOF
ls | grep ".c"
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "dshlib.c" ]]  # Ensure expected file appears
}

# 5 Test: `ls | grep "dsh"`
@test "Pipe ls output to grep dsh" {
    run "./dsh" <<EOF
ls | grep "dsh"
exit
EOF
    echo "Captured output: $output"  # Debugging output
    [ "$status" -eq 0 ]
    [[ "$output" =~ "dshlib.c" ]] || [[ "$output" =~ "dsh_cli.c" ]]  # Expect grep to return these files
}

# 6 Test: Running `echo Hello`
@test "Echo test" {
    run "./dsh" <<EOF
echo Hello, world!
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "Hello, world!" ]]
}

# 7 Test: Running `whoami`
@test "Run whoami command" {
    run "./dsh" <<EOF
whoami
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "$USER" ]]  # Expect the username to be part of output
}

# 8 Test: `ls -l | grep ".c"` (Advanced Piping)
@test "Pipe ls -l output to grep .c" {
    run "./dsh" <<EOF
ls -l | grep ".c"
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "-rw" ]]  # Expect file permissions in output
}

# 9 Test: Multiple Pipes (`ls | grep .c | wc -l`)
@test "Chaining multiple pipes" {
    run "./dsh" <<EOF
ls | grep ".c" | wc -l
exit
EOF
    [ "$status" -eq 0 ]
}

# 10 Test: `cat` Redirection
@test "Cat test with input redirection" {
    run "./dsh" <<EOF
cat < dshlib.c
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "#include <stdlib.h>" ]]  # Expect file content to appear
}

# 11 Test: `echo` with Output Redirection
@test "Echo test with output redirection" {
    run "./dsh" <<EOF
echo "Testing output" > test_output.txt
cat test_output.txt
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "Testing output" ]]
}

# 12 Test: `echo` with Append Redirection
@test "Echo test with append redirection" {
    run "./dsh" <<EOF
echo "First Line" > test_output.txt
echo "Second Line" >> test_output.txt
cat test_output.txt
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "First Line" ]] && [[ "$output" =~ "Second Line" ]]
}

# 13 Test: Long Command Execution
@test "Long command execution test" {
    run "./dsh" <<EOF
ls -l -a -h
exit
EOF
    [ "$status" -eq 0 ]
}
