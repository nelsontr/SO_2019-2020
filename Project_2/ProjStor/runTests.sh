#!/bin/bash
#Criado por Nelson e Matheus, grupo22

#Checks if there are 4 elements after the executable
if [ ! $# -eq 4 ];then
	echo "Wrong format!";
	echo "Curret: ./runTests.sh inputFolder outputFolder numThreads numHash";
	exit 1;
fi

#Inputs
inputFolder=$1;
outputFolder=$2;
numThreads=$3;
hashMax=$4;

#Scrpit
if [ ! -d "tecnicofs-*" ]; then
	make all | grep !"";
fi

if [ ! -d "$outputFolder" ]; then
	mkdir $2;
fi

for inputFile in "$1"/*
do
	echo "InputFile=​ $inputFile NumThreads= 1"
	./tecnicofs-nosync $inputFile ${inputFile/"inputs"/$2} 1 | grep "TecnicoFS completed in";

	for i in $(seq 2 $numThreads)
	do
		echo "InputFile=​ $inputFile NumThreads= $i"
		auxFile=${inputFile/"inputs"/$2}
		outFile=${auxFile/".txt"/"-$i.txt"}
		./tecnicofs-mutex $inputFile $outFile $i | grep "TecnicoFS completed in";
	done

done

make clean | grep !"";
exit 0;
