/* p:  print input in chunks */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PAGESIZE 22
#define BUFSIZE 1024
char *progname;  /* program name for error message */

FILE *efopen(char *file, char *mode)
{
    FILE *fp;
    if ((fp = fopen(file, mode)) != NULL) {
        return fp;
    }
    fprintf(stderr, "%s: can't open file %s mode %s\n", progname, file, mode);
    exit(1);
}

void ttyin(char *cmd)
{
    char buf[BUFSIZE];
    static FILE *tty = NULL;

    if (tty == NULL) {
        tty = efopen("/dev/tty", "r");
    }
    for (;;) {
        if (fgets(buf, BUFSIZE, tty) == NULL || buf[0] == 'q') {
            exit(0);
        }
        if (buf[0] == '!') {
            system(buf+1);
            printf("!\n"); /* for split */
        } else {
            memcpy(cmd, buf, BUFSIZE);
            return;
        }
    }
}

void print(FILE *fp, int pagesize)
{
    static int lines = 0;  /* number of lines so far */
    char buf[BUFSIZE] = {0};
    static long stack[BUFSIZE] = {0};
    static int stackNum = 0;
    int actualPageSize = pagesize;

    stack[stackNum++] = ftell(fp);
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (++lines < actualPageSize) {
            fputs(buf, stdout);
            continue;
        }
        buf[strlen(buf)-1] = '\0';
        fputs(buf, stdout);
        fflush(stdout);
        stack[stackNum++] = ftell(fp);
        char cmd[BUFSIZE] = {0};
        ttyin(cmd);
        if (cmd[0] == 'u') {
            int fpNum = stackNum >= 3 ? stackNum - 3 : 0;
            stackNum -= 2;
            fseek(fp, stack[fpNum], SEEK_SET);
        } else if (cmd[0] == ':') {
            actualPageSize = atoi(&cmd[1]);
        }
        lines = 0;
    }
}

int main(int argc, char *argv[])
{
    int i, pagesize = PAGESIZE;
    FILE *fp = NULL;
    progname = argv[0];
    if (argc > 1 && argv[1][0] == '-') {
        pagesize = atoi(&argv[1][1]);  /* string to number */
        argc--;
        argv++;
    }
    if (argc == 1) {
        print(stdin, pagesize);
        exit(0);
    }
    for (i = 1; i < argc; i++) {
        fp = efopen(argv[i], "r");
        print(fp, pagesize);
        fclose(fp);
    }
    exit(0);
}
