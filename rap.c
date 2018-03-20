/*
 * Get restriction associated probes, via in-silico restriction digestion
 * of a reference sequence.
 *
 * Copyright (c) 2017 Graham Gower <graham.gower@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "kseq.h"
KSEQ_INIT(int, read);

typedef struct {
	char *name; // label for restriction enzyme
	char *seq;
	int len; // sequence length
	int pos0; // cut position on top strand
	int pos1; // cut position on bottom strand
} enzyme_t;

typedef struct {
	enzyme_t *enzymes;
	int n_enzymes;
	int lower_size;
	int upper_size;
	char *fasta_fn;
} opt_t;

int
restrict_sites(opt_t *opt)
{
	int fd;
	int ret;
	kseq_t *ks;
	int i, j;


	fd = open(opt->fasta_fn, O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "%s: open: %s\n", opt->fasta_fn, strerror(errno));
		ret = -1;
		goto err0;
	}

	ks = kseq_init(fd);
	if (ks == NULL) {
		fprintf(stderr, "%s: kseq_init: %s\n", opt->fasta_fn, strerror(errno));
		ret = -2;
		goto err1;
	}

	while (kseq_read(ks) >= 0) {
		enzyme_t *e_last = 0;
		int i_last = -1;

		for (i=0; i<ks->seq.l; i++) {
			for (j=0; j<opt->n_enzymes; j++) {
				enzyme_t *e = opt->enzymes + j;

				if (strncasecmp(e->seq, ks->seq.s+i, e->len))
					continue;


				/*
				 * RAD-seq typically requires different
				 * restriction enzymes to have cut at either
				 * end of the molecule in order that the
				 * molecule be amplified.  So we mandate that
				 * the current enzyme match is different to
				 * the previous match.
				 */
				if (e_last && e_last != e) {
					int pos5 = i_last + e_last->pos0;
					int pos3 = i + e->pos0;
					int len = pos3-pos5;
					if (len > opt->lower_size && len < opt->upper_size)
						printf("%s\t%d\t%d\t%s\t%d\t%s\n",
							ks->name.s,
							pos5,
							pos3+1,
							e_last->name,
							len,
							e->name);
				}

				e_last = e;
				i_last = i;
			}
		}

	}

	ret = 0;
//err2:
	kseq_destroy(ks);
err1:
	close(fd);
err0:
	return ret;
}


int
parse_enzyme(char *s, enzyme_t *e)
{
	char *p = s;

	while (*p != 0 && *p != ':')
		p++;

	if (*p != ':') {
		fprintf(stderr, "`%s`: invalid enzyme, expected 3 colon "
				"separated fields\n", s);
		return -1;
	}

	*p = 0;
	e->name = s;
	p++;
	e->seq = p;

	while (*p != 0 && *p != ':')
		p++;

	if (*p != ':') {
		fprintf(stderr, "`%s`: invalid enzyme, expected 3 colon "
				"separated fields\n", s);
		return -2;
	}

	*p = 0;
	e->len = strlen(e->seq);
	p++;

	for (p=e->seq; *p != 0; p++) {
		*p = toupper(*p);
		switch (*p) {
		case 'A':
		case 'C':
		case 'G':
		case 'T':
			continue;
		default:
			fprintf(stderr, "`%s`: invalid enzyme sequence, "
				"expected string of ACGTs in 2nd field\n",
				s);
			return -3;
		}
	}

	p++;

	e->pos0 = strtoul(p, NULL, 0);
	if (e->pos0 < 0 || e->pos0 > e->len) {
		fprintf(stderr, "`%s`: cut position not in sequence\n", s);
		return -4;
	}

	e->pos1 = e->len - e->pos0;

	return 0;
}

void
usage(char *argv0, opt_t *opt)
{
    fprintf(stderr, "usage: %s [OPTS] ref.fa\n\n", argv0);
    fprintf(stderr, "    -l LOWER            Lower size limit [%d].\n", opt->lower_size);
    fprintf(stderr, "    -u UPPER            Upper size limit [%d].\n", opt->upper_size);
    fprintf(stderr, "    -e NAME:SEQ:POS     Restriction enzyme. This option \n"
		    "                        may be provided more than once.\n"
		    "                        NAME is a label for the enzyme,\n"
		    "                        SEQ is a sequence of ACGTs, and \n"
		    "                        POS is the position where cutting\n"
		    "                        occurs with 0 corresponding to the\n"
		    "                        position before the first base.\n"
	   );
}

int
main(int argc, char **argv)
{
	int c;
	int ret;
	opt_t opt = {0,};

	opt.upper_size = 10000;

	while ((c = getopt(argc, argv, "e:l:u:")) >= 0) {
		switch (c) {
		case 'e':
			opt.n_enzymes++;
			opt.enzymes = realloc(opt.enzymes,
					sizeof(enzyme_t)*opt.n_enzymes);
			if (opt.enzymes == NULL) {
				perror("realloc");
				return -2;
			}
			if (parse_enzyme(optarg,
					&opt.enzymes[opt.n_enzymes-1])) {
				return -3;
			}
			break;
		case 'l':
			opt.lower_size = strtoul(optarg, NULL, 0);
			if (opt.lower_size < 0 || opt.lower_size > 10000) {
				fprintf(stderr, "-l `%s` is invalid\n",
						optarg);
				return -4;
			}
			break;
		case 'u':
			opt.upper_size = strtoul(optarg, NULL, 0);
			if (opt.upper_size < 0 || opt.upper_size > 10000) {
				fprintf(stderr, "-l `%s` is invalid\n",
						optarg);
				return -5;
			}
			break;
		default:
			usage(argv[0], &opt);
			return -1;
		}
	}

	if (argc - optind != 1 || opt.n_enzymes == 0) {
		usage(argv[0], &opt);
		return -1;
	}

	opt.fasta_fn = argv[optind];

	ret = restrict_sites(&opt);

	free(opt.enzymes);

	return ret;
}
