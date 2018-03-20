# Restriction Associated Probes (RAPs)
RAP simulatess restriction digestion of a reference sequence and outputs a
bed file containing restriction associated intervals.

# Prerequisites
RAP requires a C compiler.  The plotting script requires **python** and
**matplotlib**.

# Installation
Clone the git repository, then build RAP with `make`.

# Usage
Digest `OARv40.fa` with the HpaII and PstI restriction enzymes.  Restriction
enzymes are specified as NAME:SEQ:POS, where SEQ is the motif recognised
by the enzyme and POS is the restriction site within the motif (0 is the
position before the frist base). At least two restriction enzymes must be
specified.

```
./rap -e HpaII:CCGG:1 -e PstI:CTGCAG:5 -l 100 -u 500 OARv40.fa > out.bed
```

The output columns are:
1. chromosome/scaffold name
2. start position
3. end position
4. 5' restriction enzyme
5. interval length
6. 3' restriction enzyme

Example output:
```
1       1150    1323    PstI    172     HpaII
1       1530    1650    PstI    119     HpaII
1       1681    1814    PstI    132     HpaII
1       8920    9033    HpaII   112     PstI
1       9483    9721    PstI    237     HpaII
1       9881    10080   PstI    198     HpaII
1       10098   10451   PstI    352     HpaII
1       10450   10792   HpaII   341     PstI
1       12099   12292   HpaII   192     PstI
1       13886   14349   PstI    462     HpaII
```

Note that no intervals begin and end with the same restriction enzyme.
Typically, RAD-seq requires different restriction enzymes to have cut at
either end of the molecule in order that molecule be amplified.

Plot the fragment length histogram:
```
./colhist.py -c 5 -o fraglens.pdf \
	-t "Oar4: HpaII/PstI restriction fragment lengths" \
	-x "fragment lengths" \
	out.bed
```

An example script `bed2fasta.sh` is provided which extracts the RAPs from
the reference sequence, outputting a fasta file.  This script will need to
be modified if you use restriction enzymes other than HpaII/PstI.
