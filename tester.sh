#!/bin/bash
# Proper header for a Bash script.

for input in `ls samples/*.decaf`
do
        echo -ne "Testing ${input}..."
        ./dcc < ${input} 2>&1 | diff -aq \
                `echo ${input} | sed -e "s/\..*/\.out/"` - && echo "PASS"
done
