#!/bin/bash
#Modified by Matheus and Nelson, group 22

#Checks if there are 4 elements after the executable
if [ ! $# -eq 4 ];then
	echo "Wrong format!"
	echo "Correct form: ./runTests.sh inputFolder outputFolder numThreads numHash"
	exit 1
fi

#Inputs
inputdir=$1
outputdir=$2
maxthreads=$3
numbuckets=$4

#Scrpit
if [ ! -d "tecnicofs-*" ]; then
	make all | grep !""
fi

mkdir -p $2


for inputFile in "$inputdir"/*
do
	echo "InputFile = ​${inputFile/"$inputdir/"/""}" "NumThreads = 1"
	auxFile=${inputFile/"$inputdir"/$outputdir}
	auxFile=${auxFile%.*}
	outFile="$auxFile-1.txt"
	./tecnicofs-nosync $inputFile $outFile 1 1 | grep "TecnicoFS completed in"

	for thread in $(seq 2 $maxthreads)
	do
		echo "InputFile =​ ${inputFile/"$inputdir/"/""}" "NumThreads = $thread"
		auxFile=${inputFile/"$inputdir"/$outputdir}
		auxFile=${auxFile%.*}
		outFile="$auxFile-$thread.txt"
		./tecnicofs-mutex $inputFile $outFile $thread $numbuckets | grep "TecnicoFS completed in"
	done

done

#make clean | grep !""
exit 0
