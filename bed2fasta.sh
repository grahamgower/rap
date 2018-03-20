#!/bin/sh
#
# Create fasta file containing the sequences of the RAPs.
# This is specific to the HpaII and PstI enzymes.

ref=$1
bed=$2

awk '{
	if ($4=="HpaII") {
		printf "%s:%d-%d\n",$1,$2+4,$3-6
	} else if ($4=="PstI") {
		printf "%s:%d-%d\n",$1,$2+2,$3-2
	}
	}' $bed \
	| xargs samtools faidx $ref
