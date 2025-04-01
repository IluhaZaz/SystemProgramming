#!/bin/bash
mkdir ROOT
cd ROOT

mkdir a_2
cd a_2

touch b_0.bin

touch b_3.txt
echo parrot > b_3.txt

cd ..

ln a_2/b_0.bin a_0.bin

mkdir a_1

cd a_1
mkdir b_1
ln -s a_2/b_3.txt b_2.txt

