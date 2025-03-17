#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit test suite in this file

@test "Basic command: ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF
    [ "$status" -eq 0 ]
}

@test "Piping: ls | grep dshlib.c" {
    run ./dsh <<EOF                
ls | grep dshlib.c
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dshlib.cdsh3>dsh3>cmdloopreturned0"

    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Change Directory: cd and pwd" {
    mkdir -p test_dir
    run ./dsh <<EOF
cd test_dir
pwd
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "test_dir" ]] # Ensure the shell is inside test_dir
}

@test "Exit Command: should terminate shell" {
    run ./dsh <<EOF
exit
EOF
    [ "$status" -eq 0 ] # Shell should exit cleanly
}

@test "Complex Piping: ls | grep dsh | wc -l" {
    run ./dsh <<EOF
ls | grep dsh | wc -l
EOF
    [ "$status" -eq 0 ] # Ensure command executes successfully
}

@test "Redirect Input and Output: sort < test_output.txt > sorted_output.txt" {
    run ./dsh <<EOF
sort < test_output.txt > sorted_output.txt
EOF
    [ "$status" -eq 0 ]
    [ -f sorted_output.txt ] # Ensure file was created
}

@test "Multiple Pipes: ls | grep dsh | awk '{print \$1}'" {
    run ./dsh <<EOF
ls | grep dsh | awk '{print \$1}'
EOF
    [ "$status" -eq 0 ]
}

@test "Background Execution: sleep 1 &" {
    run ./dsh <<EOF
sleep 1 &
EOF
    [ "$status" -eq 0 ]
}

@test "Handling Special Characters: printf '*?[]{}'" {
    run ./dsh <<EOF
printf '*?[]{}'
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ \*\?\[\]\{\} ]] # Ensure special characters are printed correctly
}


@test "Command Substitution: echo \$(ls)" {
    run ./dsh <<EOF
echo \$(ls)
EOF
    [ "$status" -eq 0 ]
}

@test "Chained Commands: mkdir test1 && cd test1 && pwd" {
    mkdir -p test1
    run ./dsh <<EOF
mkdir test1 && cd test1 && pwd
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "test1" ]]
}

@test "Print Working Directory: pwd" {
    run ./dsh <<EOF
pwd
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "$PWD" ]] # Ensure the output contains the current directory
}

@test "Check Current User: whoami" {
    current_user=$(whoami)
    run ./dsh <<EOF
whoami
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "$current_user" ]] # Ensure the output matches the current user
}
