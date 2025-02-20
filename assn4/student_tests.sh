#!/usr/bin/env bats

# File: student_tests.sh
# 
# This file contains custom unit tests for dsh

@test "Simple command execution" {
    run ./dsh <<EOF
ls
exit
EOF

    [ "$status" -eq 0 ]
}

@test "Check pwd command" {
    run ./dsh <<EOF
pwd
exit
EOF

    [ "$status" -eq 0 ]
}

@test "Change directory - no arguments (should go to HOME)" {
    run ./dsh <<EOF
cd
pwd
exit
EOF

    [[ "$output" == *"$HOME"* ]]
    [ "$status" -eq 0 ]
}

@test "Invalid command handling" {
    run ./dsh <<EOF
nonexistentcmd
exit
EOF

    [[ "$output" == *"execvp"* ]]
    [ "$status" -eq 0 ]
}

@test "Execute command with arguments" {
    run ./dsh <<EOF
echo Hello World
exit
EOF

    [[ "$output" == *"Hello World"* ]]
    [ "$status" -eq 0 ]
}

@test "Handle quoted spaces" {
    run ./dsh <<EOF
echo "  hello    world  "
exit
EOF

    [[ "$output" == *"  hello    world  "* ]]
    [ "$status" -eq 0 ]
}

@test "Check 'which' command" {
    run ./dsh <<EOF
which which
exit
EOF

    [[ "$output" == *"/usr/bin/which"* ]]
    [ "$status" -eq 0 ]
}

@test "Ensure correct loop return message" {
    run ./dsh <<EOF
exit
EOF

    [[ "$output" == *"cmd loop returned 0"* ]]
    [ "$status" -eq 0 ]
}
