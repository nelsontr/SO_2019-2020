#!/bin/bash
#Criado por Nelson e Matheus, grupo22

#Checks if there are 4 elements after the executable
if [ ! $# -eq 4 ];then
	echo "Wrong format!";
	echo "Curret: ./runTests.sh inputFolder outputFolder numThreads numHash";
	exit 1;
fi

#Inputs
inputdir=$1;
outputdir=$2;
maxthreads=$3;
numbuckets=$4;

#Scrpit
if [ ! -d "tecnicofs-*" ]; then
	make all | grep !"";
fi

#if [ ! -d "$outputdir" ]; then
mkdir -p $2;
#fi

for inputFile in "$inputdir"/*
do
	echo "InputFile = ​${inputFile/"$inputdir/"/""}" "NumThreads = 1"
	./tecnicofs-nosync $inputFile ${inputFile/"$inputdir"/$outputdir} 1 1 | grep "TecnicoFS completed in";

	for thread in $(seq 2 $maxthreads)
	do
		echo "InputFile =​ ${inputFile/"$inputdir/"/""}" "NumThreads = $thread"
		auxFile=${inputFile/"$inputdir"/$outputdir}
		outFile=${auxFile/".txt"/"-$thread.txt"}
		./tecnicofs-mutex $inputFile $outFile $thread $numbuckets | grep "TecnicoFS completed in";
	done

done

make clean | grep !"";
exit 0;