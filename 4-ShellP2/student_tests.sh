#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit test suite in this file

setup() {
    chmod +x ./dsh
}

@test "01 - Built-in: exit command should terminate shell" {
    run ./dsh <<EOF
exit
EOF
    [ "$status" -eq 0 ]
}

@test "02 - Built-in: cd to an invalid directory should print an error" {
    run ./dsh <<EOF
cd /nonexistentdir
EOF
    [[ "$output" == *"cd failed"* ]]
}

@test "03 - External: ls should run without errors" {
    run ./dsh <<EOF                
ls
EOF
    [ "$status" -eq 0 ]
}

@test "04 - External: running an invalid command should print an error" {
    run ./dsh <<EOF                
someinvalidcommand
EOF
    [[ "$output" == *"exec failed"* ]]
}

@test "05 - Edge Case: empty input should not crash the shell" {
    run ./dsh <<EOF

EOF
    [ "$status" -eq 0 ]
}

@test "06 - Edge Case: very long command should not crash shell" {
    long_cmd="echo $(head -c 200 < /dev/zero | tr '\0' 'A')"
    run ./dsh <<EOF
$long_cmd
EOF
    [[ "$output" == *"AAAAA"* ]]
}

@test "07 - Edge Case: running command with leading spaces should work" {
    run ./dsh <<EOF
    ls
EOF
    [ "$status" -eq 0 ]
}

@test "08 - Edge Case: executing an empty command should not crash" {
    run ./dsh <<EOF
""
EOF
    [ "$status" -eq 0 ]
}

@test "09 - Edge Case: executing a command with excessive spaces should work" {
    run ./dsh <<EOF
ls                -l
EOF
    [ "$status" -eq 0 ]
}

@test "10 - Edge Case: executing an unknown binary should fail" {
    run ./dsh <<EOF
/bin/unknown_binary
EOF
    [[ "$output" == *"exec failed"* ]]
}
