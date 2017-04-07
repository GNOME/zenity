#!/bin/bash
find src -type f \( -iname '*.c' -or -iname '*.h' \) -exec clang-format -i -style=file {} \;

