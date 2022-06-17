#!/bin/bash  
echo "Shell script for testing"

alphas=(0.01 0.001 0.0001 0.00001 0.000001)
for alpha in "${alphas[@]}"
do 
	echo "$alpha"
done