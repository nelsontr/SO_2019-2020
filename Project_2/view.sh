#!/bin/bash

outdir=out/;

for outFile in "$outdir"/*
do
    cat $outFile
done