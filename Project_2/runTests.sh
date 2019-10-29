#!/bin/bash
#Criado por Nelson e Matheus

if [ ! -d "tecnicofs-*" ]; then
	make all | grep !"";
fi

if [ ! -d "$2" ]; then
	mkdir $2; # Control will enter here if $DIRECTORY exists.
fi

for file in "$1"/*
do
	echo "InputFile=​ $file NumThreads= 1"
	./tecnicofs-nosync $file ${file/"inputs"/$2} 1 | grep "TecnicoFS completed in";

	for i in $(seq 2 $3)
	do
		echo "InputFile=​ $file NumThreads= $i"
		auxfile=${file/"inputs"/$2}
		outfile=${auxfile/".txt"/"-$i.txt"}
		./tecnicofs-mutex $file $outfile $i | grep "TecnicoFS completed in";
	done
done
make clean | grep !"";
