#!/bin/sh

ref=/localscratch/Refs/Ovis_aries/Oar_v4.0/OARv40.fasta
bed=sheep.bed

./rap \
	-e HpaII:CCGG:1 \
	-e PstI:CTGCAG:5 \
	-l 100 \
	-u 500 \
	$ref \
	> $bed

../colhist.py \
	-b 0 \
	-c 5 \
	-i \
	-o sheep_RAP_fraglens.pdf \
	-t "Oar4: HpaII/PstI restriction fragment lengths" \
	-x "fragment lengths" \
	$bed
