#!/bin/bash

for i in {1..10} 
do 
    ./test.exe >> res.txt
done

echo "Fin prog"