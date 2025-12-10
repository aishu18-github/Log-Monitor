#!/bin/sh

if [ "$#" -ne 1 ]; then
    echo "USAGE: $0 <filename>"
    exit 1
fi

FILENAME="$1"

if [ ! -f "$FILENAME" ]; then
    echo "Error: File not found: $FILENAME"
    exit 1
fi

echo "Analyzing file: $FILENAME..."

TOTAL_LINES=$(wc -l < "$FILENAME")
ERROR_LINES=$(grep -c -i 'error' "$FILENAME")
WARNING_LINES=$(grep -c -i 'warning' "$FILENAME")

echo ""
echo "--- Analysis Complete ---"
printf "Total lines processed: %s\n" "$TOTAL_LINES"
printf "Lines with 'error':  %s\n" "$ERROR_LINES"
printf "Lines with 'warning':%s\n" "$WARNING_LINES"
echo "-------------------------"