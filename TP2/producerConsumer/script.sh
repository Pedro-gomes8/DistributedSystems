#!/bin/bash
make clean
make
buffers="1 10 100 1000"
producersList=(1 1 1 1 2 4 8)
consumersList=(1 2 4 8 1 1 1)

for bufferSize in $buffers; do
    for index in "${!producersList[@]}"
    do
        p=${producersList[index]}
        c=${consumersList[index]}
        for i in $(seq 1 10); do
            ./producerConsumer $p $c $bufferSize
        done
    done
done

make clean