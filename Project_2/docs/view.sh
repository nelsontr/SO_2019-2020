#!/bin/bash

outdir=out/;

for outFile in "$outdir"/*
do
    echo »» $outFile:
    cat $outFile
    echo
done