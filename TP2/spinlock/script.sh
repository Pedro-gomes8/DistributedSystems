#!/bin/bash
make all
csv_file="output.csv"
# Write the header row to the CSV file
echo "Power,Threads,Mean" > "$csv_file"

# Define the values of k and N to loop over
ns="1 2 4 8 16 32 64 128 256"
ks="7 8 9"

# Loop over all combinations of k and N
for k in $ks; do
  for N in $ns; do
    sum=0
    # Run the C++ program 10 times and calculate the mean
    for i in $(seq 1 10); do
      output=$(./spinlocks $k $N) 
      sum=$(echo "scale=2; $sum + $output" | bc)
    done
    mean=$(echo "scale=2; $sum / 10" | bc)
    # Write the output to the CSV file
    echo "$k,$N,$mean" >> "$csv_file"
  done
done

make clean