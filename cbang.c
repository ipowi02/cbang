//$  gcc -Wall -Wextra  "$SRC" -o "${SRC%.c}" && "${SRC%.c}"
//$c gcc -Wall -Wextra "$SRC" -o "${SRC%.c}"
//$x ./"${SRC%.c}"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>

#define LINELEN 1024

enum { DEFAULT, BUILD, EXEC, NMODES };
static const char *mode_name[NMODES] = { "default", "build", "exec" };
static char cmds[NMODES][LINELEN];

static void usage(FILE *f, const char *prog) {
    fprintf(f, "USAGE: %s [build|exec] <file>\n", prog);
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
    int mode = DEFAULT, i = 1;

    if (i < argc && strcmp(argv[i], "build") == 0) {
        mode = BUILD;
        i++;
    } else if (i < argc && strcmp(argv[i], "exec") == 0) {
        mode = EXEC;
        i++;
    }
    if (i != (argc - 1)) {
        usage(stderr, argv[0]);
        return 2;
    }
    const char *path = argv[i];

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

    
    fprintf(stderr, "+ %s\n", cmd);
    int rc = system(cmd);
    if (rc == -1) {
        perror("system");
        return 1;
    }
    return WIFEXITED(rc) ? WEXITSTATUS(rc) : 1;
}
