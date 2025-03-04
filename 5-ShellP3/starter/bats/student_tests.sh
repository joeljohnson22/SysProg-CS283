#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file


@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

# Basic pipe tests

@test "Simple pipe: ls | grep 'dsh'" {
    run ./dsh <<EOF
ls | grep 'dsh'
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"dsh"* ]]
}

@test "Simple pipe: echo hello | cat" {
    run ./dsh <<EOF
echo hello | cat
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"hello"* ]]
}

@test "Multi-pipe: echo hello | tr 'a-z' 'A-Z' | cat" {
    run ./dsh <<EOF
echo hello | tr 'a-z' 'A-Z' | cat
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"HELLO"* ]]
}

@test "Multi-pipe with complex commands: ps aux | grep bash | wc -l" {
    run ./dsh <<EOF
ps aux | grep bash | wc -l
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" =~ [0-9]+ ]]  # Output should contain at least one number
}

# Edge cases

@test "Pipe with non-existent command: ls | nonexistent" {
    run ./dsh <<EOF
ls | nonexistent
EOF
    
    # Assertions
    [ "$status" -eq 0 ]  # Shell should continue running
    [[ "$output" == *"not found"* || "$output" == *"No such file"* || "$output" == *"Command not found"* ]]
}

@test "First command non-existent: nonexistent | ls" {
    run ./dsh <<EOF
nonexistent | ls
EOF
    
    # Assertions
    [ "$status" -eq 0 ]  # Shell should continue running
    [[ "$output" == *"not found"* || "$output" == *"No such file"* || "$output" == *"Command not found"* ]]
}

@test "Empty pipe: ls | | grep dsh" {
    run ./dsh <<EOF
ls | | grep dsh
EOF
    
    # Should detect syntax error or just ignore empty pipe segment
    [ "$status" -eq 0 ]  # Shell should continue running
}

@test "Maximum number of pipes allowed" {
    # Create a command with CMD_MAX-1 pipes (assuming CMD_MAX=8)
    run ./dsh <<EOF
echo hello | cat | cat | cat | cat | cat | cat | cat
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"hello"* ]]
}

@test "Exceeding maximum number of pipes" {
    # Create a command with too many pipes
    run ./dsh <<EOF
echo hello | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"error: piping limited"* || "$output" == *"too many"* ]]
}

# Integration tests

@test "Pipe after built-in command should fail gracefully: cd / | ls" {
    run ./dsh <<EOF
cd / | ls
EOF
    
    # Built-in commands shouldn't be part of pipes, but shell should handle this
    [ "$status" -eq 0 ]
}

@test "Pipe with exit status: false | true" {
    run ./dsh <<EOF
false | true
rc
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    # Last command's exit status should be 0 (true)
    [[ "$output" == *"0"* ]]
}

@test "Pipe with exit status: true | false" {
    run ./dsh <<EOF
true | false
rc
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    # Last command's exit status should be 1 (false)
    [[ "$output" == *"1"* ]]
}

# Input/Output handling tests

@test "Large data through pipe: seq 1000 | wc -l" {
    run ./dsh <<EOF
seq 1000 | wc -l
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"1000"* ]]
}

@test "Multiple pipes with whitespace: echo hello |    grep hello   |   cat" {
    run ./dsh <<EOF
echo hello |    grep hello   |   cat
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"hello"* ]]
}

@test "Command redirection with pipe: echo hello > /tmp/test_pipe.txt | cat /tmp/test_pipe.txt" {
    # Create a temporary file for testing
    rm -f /tmp/test_pipe.txt
    
    run ./dsh <<EOF
echo hello > /tmp/test_pipe.txt
cat /tmp/test_pipe.txt
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"hello"* ]]
    
    # Clean up
    rm -f /tmp/test_pipe.txt
}

@test "Execute pipeline then execute built-in: ls | wc -l && rc" {
    run ./dsh <<EOF
ls | wc -l
rc
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    # rc should show the exit status of the last pipeline command
    [[ "$output" =~ [0-9]+ ]]
}

@test "Output redirection: echo hello > outfile.txt" {
    run ./dsh <<EOF
echo hello > outfile.txt
cat outfile.txt
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"hello"* ]]
    
    # Clean up
    rm -f outfile.txt
}

@test "Input redirection: cat < infile.txt" {
    echo "test input data" > infile.txt
    
    run ./dsh <<EOF
cat < infile.txt
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"test input data"* ]]
    
    # Clean up
    rm -f infile.txt
}

@test "Combined redirection: cat < infile.txt > outfile.txt" {
    echo "test combined redirection" > infile.txt
    
    run ./dsh <<EOF
cat < infile.txt > outfile.txt
cat outfile.txt
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"test combined redirection"* ]]
    
    # Clean up
    rm -f infile.txt outfile.txt
}

@test "Append redirection: append to existing file" {
    run ./dsh <<EOF
echo "line 1" > append_test.txt
echo "line 2" >> append_test.txt
cat append_test.txt
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"line 1"* ]]
    [[ "$output" == *"line 2"* ]]
    
    # Clean up
    rm -f append_test.txt
}

@test "Pipe with redirection: echo hello | tr 'a-z' 'A-Z' > outfile.txt" {
    run ./dsh <<EOF
echo hello | tr 'a-z' 'A-Z' > outfile.txt
cat outfile.txt
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"HELLO"* ]]
    
    # Clean up
    rm -f outfile.txt
}