#!/usr/bin/env bash

# Test `zenity --list` with named pipes.
#
# Based on a script written by Falan Orbiplanax, reported in Zenity issue #71
# on GitLab. I gave up on getting the script to run correctly from the Zenity
# test-suite, because no matter what I tried to modify it, it would almost
# always result in a stale process that would not die. At this time, it should
# be run manually. The test is considered successful if the program runs to
# termination without a segfault.

ZENITY=${1:-zenity}

# Pesky script won't DIE!! This is what finally gets it to do so. Since SIGTERM
# exit status is 143 typically, we'll test for that exit status to see if the
# test "passed". Grrr....

trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

fifo=myfifo
[[ -p $fifo ]] && rm "$fifo"
mkfifo "$fifo"

write_fifo() {
    while [[ -e $fifo ]]; do
        printf "Test\n" >> "$fifo"
        sleep 1s
    done
}

write_fifo &

${ZENITY} \
    --timeout=5 \
    --list \
    --column=AAA \
    --width=640 \
    --height=480 \
    < <(while [[ -e $fifo ]]; do cat "$fifo"; done)

rm "$fifo"
