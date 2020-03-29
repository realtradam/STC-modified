#ifndef CGETOPT_H
#define CGETOPT_H

#include <string.h>

enum {
    c_optarg_none     = 0,
    c_optarg_required = 1,
    c_optarg_optional = 2
};
typedef struct {
	int ind;   /* equivalent to optind */
	int opt;   /* equivalent to optopt */
	char *arg; /* equivalent to optarg */
	int longidx; /* index of a long option; or -1 if short */
	/* private variables not intended for external uses */
	int i, pos, n_args;
} c_getopt_t;

typedef struct {
	char *name;
	int has_arg;
	int val;
} c_longopt_t;

static const c_getopt_t c_getopt_init = {1, 0, 0, -1, 1, 0, 0};

static void c_getopt_permute(char *argv[], int j, int n) /* move argv[j] over n elements to the left */
{
	int k;
	char *p = argv[j];
	for (k = 0; k < n; ++k)
		argv[j - k] = argv[j - k - 1];
	argv[j - k] = p;
}

/**
 * Parse command-line options and arguments
 *
 * This fuction has a similar interface to GNU's getopt_long(). Each call
 * parses one option and returns the option name.  s->arg points to the option
 * argument if present. The function returns -1 when all command-line arguments
 * are parsed. In this case, s->ind is the index of the first non-option
 * argument.
 *
 * @param s         status; shall be initialized to c_getopt_init on the first call
 * @param argc      length of argv[]
 * @param argv      list of command-line arguments; argv[0] is ignored
 * @param permute   non-zero to move options ahead of non-option arguments
 * @param ostr      option string
 * @param longopts  long options
 *
 * @return ASCII for a short option; c_longopt_t::val for a long option; -1 if
 *         argv[] is fully processed; '?' for an unknown option or an ambiguous
 *         long option; ':' if an option argument is missing
 */
static int c_getopt(c_getopt_t *s, int argc, char *argv[], int permute, const char *ostr, const c_longopt_t *longopts)
{
	int opt = -1, i0, j;
	if (permute) {
		while (s->i < argc && (argv[s->i][0] != '-' || argv[s->i][1] == '\0'))
			++s->i, ++s->n_args;
	}
	s->arg = 0, s->longidx = -1, i0 = s->i;
	if (s->i >= argc || argv[s->i][0] != '-' || argv[s->i][1] == '\0') {
		s->ind = s->i - s->n_args;
		return -1;
	}
	if (argv[s->i][0] == '-' && argv[s->i][1] == '-') { /* "--" or a long option */
		if (argv[s->i][2] == '\0') { /* a bare "--" */
			c_getopt_permute(argv, s->i, s->n_args);
			++s->i, s->ind = s->i - s->n_args;
			return -1;
		}
		s->opt = 0, opt = '?', s->pos = -1;
		if (longopts) { /* parse long options */
			int k, n_exact = 0, n_partial = 0;
			const c_longopt_t *o = 0, *o_exact = 0, *o_partial = 0;
			for (j = 2; argv[s->i][j] != '\0' && argv[s->i][j] != '='; ++j) {} /* find the end of the option name */
			for (k = 0; longopts[k].name != 0; ++k)
				if (strncmp(&argv[s->i][2], longopts[k].name, j - 2) == 0) {
					if (longopts[k].name[j - 2] == 0) ++n_exact, o_exact = &longopts[k];
					else ++n_partial, o_partial = &longopts[k];
				}
			if (n_exact > 1 || (n_exact == 0 && n_partial > 1)) return '?';
			o = n_exact == 1? o_exact : n_partial == 1? o_partial : 0;
			if (o) {
				s->opt = opt = o->val, s->longidx = o - longopts;
				if (argv[s->i][j] == '=') s->arg = &argv[s->i][j + 1];
				if (o->has_arg == 1 && argv[s->i][j] == '\0') {
					if (s->i < argc - 1) s->arg = argv[++s->i];
					else opt = ':'; /* missing option argument */
				}
			}
		}
	} else { /* a short option */
		const char *p;
		if (s->pos == 0) s->pos = 1;
		opt = s->opt = argv[s->i][s->pos++];
		p = strchr((char*)ostr, opt);
		if (p == 0) {
			opt = '?'; /* unknown option */
		} else if (p[1] == ':') {
			if (argv[s->i][s->pos] == 0) {
				if (s->i < argc - 1) s->arg = argv[++s->i];
				else opt = ':'; /* missing option argument */
			} else s->arg = &argv[s->i][s->pos];
			s->pos = -1;
		}
	}
	if (s->pos < 0 || argv[s->i][s->pos] == 0) {
		++s->i, s->pos = 0;
		if (s->n_args > 0) /* permute */
			for (j = i0; j < s->i; ++j)
				c_getopt_permute(argv, j, s->n_args);
	}
	s->ind = s->i - s->n_args;
	return opt;
}

/* // demo:
    int main(int argc, char *argv[])
    {
      static c_longopt_t longopts[] = {
        { "foo", c_optarg_none,     301 },
        { "bar", c_optarg_required, 302 },
        { "opt", c_optarg_optional, 303 },
        { NULL, 0, 0 }
      };
      c_getopt_t opt = c_getopt_init;
      int i, c;
      while ((c = c_getopt(&opt, argc, argv, 1, "xy:", longopts)) >= 0) {
        if (c == 'x') printf("-x\n");
        else if (c == 'y') printf("-y %s\n", opt.arg);
        else if (c == 301) printf("--foo\n");
        else if (c == 302) printf("--bar %s\n", opt.arg? opt.arg : "(null)");
        else if (c == 303) printf("--opt %s\n", opt.arg? opt.arg : "(null)");
        else if (c == '?') printf("unknown opt: -%c\n", opt.opt? opt.opt : ':');
        else if (c == ':') printf("missing arg: -%c\n", opt.opt? opt.opt : ':');
      }
      printf("Non-option arguments:");
      for (i = opt.ind; i < argc; ++i)
        printf(" %s", argv[i]);
      putchar('\n');
      return 0;
    }
*/


#endif