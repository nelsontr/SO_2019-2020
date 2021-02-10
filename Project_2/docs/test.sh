if [ $1 -eq 1 ];then 
	echo nosync;
	for i in $(seq 1 $2)
	do
		./tecnicofs-nosync inputs/test1.txt outF 1 1;
		cat outF
		echo ---
	done

elif [ $1 -eq 2 ];then
	echo mutex;
	for i in $(seq 1 $2)
	do
		./tecnicofs-mutex inputs/test1.txt outF 4 4;
		cat outF;
		echo ---
	done
fi

