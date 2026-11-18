//$ gcc "$SRC" -o "$BIN"
//$c gcc -Wall -Wextra "$SRC" -o "$BIN"
//$x "$BIN"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include <assert.h>

#define LINELEN 1024
#define TMPSIZE LINELEN

#define crash(code, ...) do {				\
	  fprintf(stderr, "ERROR:" __VA_ARGS__);	\
	  usage(stderr, argv[0]);			\
	  return (code);				\
     } while (0)

#define OUTPUT_EXT ".o"
static char output_ext[16] = OUTPUT_EXT;

enum { DEFAULT, BUILD, EXEC, NMODES };
static const char *mode_name[NMODES] = { "default", "build", "exec" };
static char cmds[NMODES][LINELEN];

static char tmp[TMPSIZE];
static char tmp2[TMPSIZE + 8];

static void usage(FILE *f, const char *prog) {
     fprintf(f, "USAGE: %s [build|exec] <file>\n", prog);
}

static char *bin(const char *src) {
     snprintf(tmp, sizeof tmp, "%s", src);

     char *base = strrchr(tmp, '/');
     if (base)
	  base++;
     else
	  base = tmp;
     
     char *dot = strrchr(base, '.');
     
     int has_dir = base != tmp;
     int has_ext = dot && dot > base;
     
     if (has_ext)
	  *dot = '\0';

     snprintf(tmp2, sizeof tmp2, "%s%s%s", has_dir ? "" : "./", tmp, output_ext);
     return tmp2;
     
     
}


// Returns 0 if the line is fine (directive or not), -1 if it is a malformed directive.
static int parse_line(const char *line) {
     if (strncmp(line, "//$", 3) != 0)
	  return 0;
     line += 3;

     int mode = DEFAULT;
     if (*line == 'c') {
	  mode = BUILD;
	  line++;
     } else if (*line == 'x') {
	  mode = EXEC;
	  line++;
     }

     if (!isspace(*line))
	  return -1;
     line += strspn(line, " \t");

     snprintf(cmds[mode], LINELEN, "%s", line);
     cmds[mode][strcspn(cmds[mode], "\r\n")] = '\0';
     return 0;
}

int main(int argc, char **argv) {
     int awaiting_file = 0;
     int parsed_ext = 0;
     int mode = DEFAULT;

     const char *path = NULL;

     for (int i = 1; i < argc; i++) {
	  char *arg = argv[i];
	  // We might want to refactor this once we have more flags
	  
	  if (strncmp(arg, "--ext", 5) == 0) {
	       if (parsed_ext) {
		    crash(1, "Parsed flag `--ext` already\n");
	       }
	       if (awaiting_file) {
		    crash(1, "Expected filename, got flag `--ext`\n");
	       }
	       char *cur = arg + 5;
	       if (*cur != '=') {
		    crash(3, "Unknown flag\n");
	       }
	       cur++;

	       if (strlen(cur) > 16) {
		    crash(1, "Output extension too long\n");
	       }
	       snprintf(output_ext, sizeof output_ext, "%s", cur);
	       
	       parsed_ext = 1;
	       continue;
	  }

	  if (awaiting_file) {
	       path = arg;
	       awaiting_file = 0;
	       continue;
	  }
	  if (strcmp(arg, "build") == 0) {
	       mode = BUILD;
	       awaiting_file = 1;
	       continue;
	  } else if (strcmp(arg, "exec") == 0) {
	       mode = EXEC;
	       awaiting_file = 1;
	       continue;
	  } else {
	       path = arg;
	  }
     }
     if (!path) {
	  crash(1, "Expected file path, did not get one\n");
     }
    
     // dont wanna handle memory management
     assert(strlen(path) < 64);
    
     FILE *f = fopen(path, "r");
     if (!f) {
	  perror(path);
	  return 1;
     }

     char line[LINELEN];
     for (int n = 1; fgets(line, sizeof line, f); n++) {
	  if (parse_line(line) < 0) {
	       fprintf(stderr, "%s:%d: malformed directive\n", path, n);
	       return 1;
	  }
     }
     fclose(f);

     const char *cmd = cmds[mode];
     if (!*cmd) {
	  fprintf(stderr, "%s: no %s command found\n", path, mode_name[mode]);
	  return 1;
     }

     // this is so you can use $SRC as part of the command 
     setenv("SRC", path, 1);
     setenv("BIN", bin(path), 1);
    
     fprintf(stderr, "+ %s\n", cmd);
     int rc = system(cmd);
     if (rc == -1) {
	  perror("system");
	  return 1;
     }
     return WIFEXITED(rc) ? WEXITSTATUS(rc) : 1;
}
