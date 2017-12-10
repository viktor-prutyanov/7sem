#!/bin/bash

./ode
cat out.txt | gnuplot -p
