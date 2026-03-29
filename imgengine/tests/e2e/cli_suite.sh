# tests/e2e/cli_suite.py 

#!/bin/bash
BINARY=$1
OUT="tests/output/test.png"

# Test 1: Basic Generation
$BINARY --input tests/assets/input.jpg --output $OUT --cols 2 --rows 2
if [ $? -eq 0 ] && [ -f "$OUT" ]; then
    echo "✅ CLI Basic Success"
else
    echo "❌ CLI Basic Failed"
    exit 1
fi

# Test 2: Invalid Input Handling
$BINARY --input non_existent.jpg --output $OUT 2>/dev/null
if [ $? -ne 0 ]; then
    echo "✅ CLI Error Handling Success"
fi
