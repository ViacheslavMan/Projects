#!/bin/bash

printf "apple\nAPPLE\n   apple with spaces\n\nBANANA\ncherry\nlast line no newline" > test_complex.txt
printf "apple in second file\nno match here\n" > test_second.txt
printf "apple\ncherry\n" > test_patterns.txt

MISSING_FILE="nonexistent_file.txt"
FILE="test_complex.txt"
FILE2="test_second.txt"
PATTERN="apple"
PATTERN_FILE="test_patterns.txt"
MY_GREP="./s21_grep"
LOG_FILE="fail.log"

flags=("i" "v" "n" "c" "l" "s" "h")

PASS=0
FAIL=0
rm -f "$LOG_FILE"

run_test() {
    local args="$1"

    eval "grep $args > orig.txt 2>&1"
    eval "$MY_GREP $args > mine.txt 2>&1"


    sed 's/^\.\/s21_grep/grep/' mine.txt > mine2.txt
    mv mine2.txt mine.txt

    if diff orig.txt mine.txt > /dev/null 2>&1; then
        echo "OK: $args"
        ((PASS++))
    else
        echo "FAIL: $args"
        ((FAIL++))
        echo "FAIL: $args" >> "$LOG_FILE"
        diff -u orig.txt mine.txt >> "$LOG_FILE"
        echo "" >> "$LOG_FILE"
    fi
}


run_test "$PATTERN $FILE"
run_test "$PATTERN $FILE $FILE2"
run_test "$PATTERN $MISSING_FILE"
run_test "$PATTERN $FILE $MISSING_FILE"

for (( i=0; i<${#flags[@]}; i++ )); do
    for (( j=i+1; j<${#flags[@]}; j++ )); do
        f1=${flags[$i]}; f2=${flags[$j]}
        for combined in "-${f1}${f2}" "-${f2}${f1}"; do
            run_test "$combined $PATTERN $FILE"
            run_test "$combined $PATTERN $FILE $FILE2"
            run_test "$combined $PATTERN $MISSING_FILE"
        done
    done
done


for f in "${flags[@]}"; do
    for variant in "-e $PATTERN -$f" "-${f}e $PATTERN"; do
        run_test "$variant $FILE"
        run_test "$variant $FILE $FILE2"
    done
    
    for variant in "-f $PATTERN_FILE -$f" "-${f}f $PATTERN_FILE"; do
        run_test "$variant $FILE"
        run_test "$variant $FILE $FILE2"
        run_test "$variant $MISSING_FILE"
    done
done

run_test "-e $PATTERN -f $PATTERN_FILE $FILE"
run_test "-e $PATTERN -f $PATTERN_FILE $FILE $FILE2"
for f in "${flags[@]}"; do
    run_test "-e $PATTERN -f $PATTERN_FILE -$f $FILE"
done


for f in "${flags[@]}"; do
    for combined in "-o$f" "-${f}o"; do
        run_test "$combined $PATTERN $FILE"
        run_test "$combined $PATTERN $FILE $FILE2"
    done
done

run_test "-o $PATTERN $FILE"
run_test "-o $PATTERN $FILE $FILE2"

for f in "${flags[@]}"; do
    run_test "-o -e $PATTERN -$f $FILE"
    run_test "-o -f $PATTERN_FILE -$f $FILE"
done

run_test "-h $PATTERN $FILE"
run_test "-h $PATTERN $FILE $FILE2"
run_test "-s $PATTERN $FILE"
run_test "-s $PATTERN $MISSING_FILE"
run_test "-s $PATTERN $FILE $MISSING_FILE"

echo "-------------------"
echo "PASS: $PASS"
echo "FAIL: $FAIL"


rm -f orig.txt mine.txt test_complex.txt test_second.txt test_patterns.txt