your_roll=1605066
clear
g++ -D _REENTRANT $your_roll.cpp -o $your_roll -lpthread
./$your_roll > $your_roll.txt
rm $your_roll
