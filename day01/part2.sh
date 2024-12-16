#!/usr/bin/env bash

declare -A counter

col1=()
while read -r n1 n2; do
    col1+=("$n1")
    ((counter[$n2]++))
done < input

total=0
for num in "${col1[@]}"; do
    total=$((total + counter[$num]*num))
done

echo $total
