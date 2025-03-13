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

setup_server() {
    ./dsh -s -p 8765 &
    # Store PID
    SERVER_PID=$!
    sleep 1
}

teardown_server() {
    if [ -n "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null || true
        wait $SERVER_PID 2>/dev/null || true
    fi
    pkill -f "./dsh -s -p 8765" || true
    sleep 1
}


@test "Local: simple pipe with echo and grep" {
    run ./dsh <<EOF
echo "hello world" | grep hello
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"hello world"* ]]
}

@test "Local: built-in exit command works" {
    run ./dsh <<EOF
exit
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
}

# Tests for server - these require starting/stopping the server

@test "Server: Server starts and accepts connections" {
    # Start server in background
    setup_server
    
    # Try connecting with client
    run timeout 2 ./dsh -c -p 8765 <<EOF
exit
EOF
    
    # Stop server
    teardown_server
    
    # Assertions
    [ "$status" -eq 0 ]
}

@test "Server: Basic command execution" {
    setup_server
    
    # Run a simple command through client
    run ./dsh -c -p 8765 <<EOF
echo "remote hello"
exit
EOF
    
    teardown_server
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"remote hello"* ]]
}

@test "Server: Directory listing via remote shell" {
    setup_server
    
    run ./dsh -c -p 8765 <<EOF
ls
exit
EOF
    
    teardown_server
    
    # Assertions
    [ "$status" -eq 0 ]
    # Should see some files in listing
    [[ "$output" == *"dsh"* || "$output" == *"bats"* ]]
}

@test "Server: Pipe commands on remote shell" {
    setup_server
    
    run ./dsh -c -p 8765 <<EOF
echo "hello remote world" | grep remote
exit
EOF
    
    teardown_server
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"hello remote world"* ]]
}

@test "Server: Multiple commands execution" {
    setup_server
    
    run ./dsh -c -p 8765 <<EOF
echo "first command"
echo "second command"
exit
EOF
    
    teardown_server
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"first command"* ]]
    [[ "$output" == *"second command"* ]]
}

@test "Server: Non-existent command handling" {
    setup_server
    
    run ./dsh -c -p 8765 <<EOF
nonexistentcommand
exit
EOF
    
    teardown_server
    
    # Assertions
    [ "$status" -eq 0 ]
    # Should see some kind of error message
    [[ "$output" == *"not found"* || "$output" == *"No such file"* || "$output" == *"command not found"* ]]
}

@test "Server: Server stop command works" {
    # Start the server
    ./dsh -s -p 8765 &
    SERVER_PID=$!
    sleep 2  # Give server more time to start
    
    # Execute stop-server command
    run ./dsh -c -p 8765 <<EOF
stop-server
EOF
    
    # Wait a bit longer to let server stop completely
    sleep 2
    
    # Check server exit by trying to connect again
    if timeout 1 bash -c "echo '' > /dev/tcp/localhost/8765" 2>/dev/null; then
        # If we can connect, server is still running (test failed)
        SERVER_STOPPED=1
    else
        # If connection fails, server is stopped (test passed)
        SERVER_STOPPED=0
    fi
    
    # Try to clean up in case server is still running
    kill $SERVER_PID 2>/dev/null || true
    
    # Assertions
    [ "$status" -eq 0 ]
    [ "$SERVER_STOPPED" -eq 0 ]  # Server should not accept connections
}

@test "Server: Large output handling" {
    setup_server
    
    run ./dsh -c -p 8765 <<EOF
seq 1 1000
exit
EOF
    
    teardown_server
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"1"* ]]
    [[ "$output" == *"500"* ]]
    [[ "$output" == *"1000"* ]]
}

# Tests for multi-threaded server (if implemented)

@test "Threaded server: Multiple clients can connect simultaneously" {
    # Start server in threaded
    ./dsh -s -p 8765 -x &
    SERVER_PID=$!
    sleep 2  # Give server more time to start
    
    # Start multiple client processes in background
    ./dsh -c -p 8765 <<EOF > /tmp/client1.out &
echo "from client 1"
sleep 2
exit
EOF
    CLIENT1_PID=$!
    
    ./dsh -c -p 8765 <<EOF > /tmp/client2.out &
echo "from client 2"
sleep 1
exit
EOF
    CLIENT2_PID=$!
    
    # Wait for clients to finish
    wait $CLIENT1_PID 2>/dev/null || true
    wait $CLIENT2_PID 2>/dev/null || true
    
    # Read results
    CLIENT1_OUTPUT=$(cat /tmp/client1.out 2>/dev/null || echo "")
    CLIENT2_OUTPUT=$(cat /tmp/client2.out 2>/dev/null || echo "")
    
    # Stop server more safely
    kill $SERVER_PID 2>/dev/null || true
    
    # Clean up temp files
    rm -f /tmp/client1.out /tmp/client2.out
    
    # Assertions - check that we got some output from both clients
    [[ -n "$CLIENT1_OUTPUT" ]]
    [[ -n "$CLIENT2_OUTPUT" ]]
    [[ "$CLIENT1_OUTPUT" == *"client 1"* || "$CLIENT1_OUTPUT" == *"from client"* ]]
    [[ "$CLIENT2_OUTPUT" == *"client 2"* || "$CLIENT2_OUTPUT" == *"from client"* ]]
}

# I/O redirection tests - extra credit

@test "Remote I/O: Output redirection" {
    setup_server
    
    run ./dsh -c -p 8765 <<EOF
echo "hello redirection" > /tmp/rsh_test_out.txt
cat /tmp/rsh_test_out.txt
rm /tmp/rsh_test_out.txt
exit
EOF
    
    teardown_server
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"hello redirection"* ]]
}

@test "Remote I/O: Append redirection" {
    setup_server
    
    run ./dsh -c -p 8765 <<EOF
echo "line 1" > /tmp/rsh_append_test.txt
echo "line 2" >> /tmp/rsh_append_test.txt
cat /tmp/rsh_append_test.txt
rm /tmp/rsh_append_test.txt
exit
EOF
    
    teardown_server
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" == *"line 1"* ]]
    [[ "$output" == *"line 2"* ]]
}
