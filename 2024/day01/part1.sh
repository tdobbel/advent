#!/usr/bin/env bash

col1=()
col2=()

while read -r n1 n2; do
    col1+=("$n1")
    col2+=("$n2")
done < input

readarray -t sorted1< <(printf "%s\n" "${col1[@]}" | sort)
sorted2=($(printf "%s\n" "${col2[@]}" | sort))

total=0

for i in $(seq 0 ${#col1[@]}); do
    a=${sorted1[i]}
    b=${sorted2[i]}
    inc=$((b-a))
    total=$((total+${inc#-}))
done

echo $total
