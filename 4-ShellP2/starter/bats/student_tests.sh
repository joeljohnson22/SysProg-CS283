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

@test "Built-in cd changes directory" {
  # Change directory to /tmp and verify that pwd returns /tmp.
  run "./dsh" <<'EOF'
cd /tmp
pwd
EOF
  # Expect /tmp to appear in output
  [[ "$output" == *"/tmp"* ]]
  [ "$status" -eq 0 ]
}

@test "Built-in cd with no arguments does nothing" {
  # Record current working directory
  current=$(pwd)
  run "./dsh" <<EOF
cd
pwd
EOF
  # Expect output to contain the original working directory
  [[ "$output" == *"$current"* ]]
  [ "$status" -eq 0 ]
}

@test "External command execution: which echo" {
  run "./dsh" <<'EOF'
which echo
EOF
  # Expect the output to contain a common path for echo (e.g. /bin/echo or /usr/bin/echo)
  if [[ "$output" != *"/bin/echo"* ]] && [[ "$output" != *"/usr/bin/echo"* ]]; then
      false
  fi
  [ "$status" -eq 0 ]
}

@test "Handles quoted spaces" {
  run "./dsh" <<'EOF'
echo " hello     world     "
EOF
  # The argument should preserve the embedded spaces.
  # We check that the output includes the exact quoted string (whitespace preserved)
  [[ "$output" == *" hello     world     "* ]]
  [ "$status" -eq 0 ]
}

@test "Exit command terminates shell" {
  # Run exit to confirm that the dsh shell terminates.
  run "./dsh" <<'EOF'
exit
EOF
  [ "$status" -eq 0 ]
}

@test "cd with invalid directory reports error" {
  run "./dsh" <<'EOF'
cd /nonexistent_directory
EOF
  # The error message may vary; look for message containing "cd:" or "No such file"
  [[ "$output" == *"cd:"* ]] || [[ "$output" == *"No such file"* ]]
  [ "$status" -eq 0 ]
}

@test "Handles extra spaces outside of quotes" {
  run "./dsh" <<'EOF'
    echo      extra    spaces   test
EOF
  # Expect the output to properly separate tokens "extra", "spaces", "test" with a single space.
  # Depending on your echo behavior, simply check that "extra spaces test" is present.
  [[ "$output" == *"extra spaces test"* ]]
  [ "$status" -eq 0 ]
}

@test "Handles mixed quoted and unquoted arguments" {
  run "./dsh" <<'EOF'
echo first "second with   spaces" third
EOF
  # Expect output to include the second argument as a single token with preserved inner spaces.
  [[ "$output" == *"first second with   spaces third"* ]]
  [ "$status" -eq 0 ]
}

@test "Invalid external command fails" {
  run "./dsh" <<'EOF'
nonexistentcommand
EOF
  # Expect to see an error message from execvp or similar.
  [[ "$output" == *"execvp"* ]] || [[ "$output" == *"not found"* ]]
  # The child process exits with non-zero but shell loop might return 0. Check that error message is printed.
}

