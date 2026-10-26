#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX 512
#define LEN 256

char code[MAX][LEN];
int n;

int number(char *s, int *v)
{
    char extra;
    if (sscanf(s, "%d%c", v, &extra) == 1)
        return 1;
    return 0;
}

int main(void)
{
    char line[LEN];

    while (n < MAX && fgets(line, LEN, stdin))
        strcpy(code[n++], line);

    /* Constant folding, algebraic identities, and strength reduction */
    for (int i = 0; i < n; i++) {
        char lhs[128], a[128], b[128], op;
        char *end_a, *end_b;
        double x, y, r;

        if (sscanf(code[i], "%127s = %127s %c %127s",
                   lhs, a, &op, b) != 4)
            continue;

        /* Check whether both operands are numeric constants. */
        x = strtod(a, &end_a);
        y = strtod(b, &end_b);

        if (a[0] != '\0' && *end_a == '\0' &&
            b[0] != '\0' && *end_b == '\0') {
            if (op == '+')
                r = x + y;
            else if (op == '-')
                r = x - y;
            else if (op == '*')
                r = x * y;
            else if (op == '/' && y != 0.0)
                r = x / y;
            else if (op == '%' && (int)y != 0)
                r = (int)x % (int)y;
            else if (op == '^')
                r = (int)x ^ (int)y;
            else
                continue;

            snprintf(code[i], LEN, "%s = %.10g\n", lhs, r);
            continue;
        }

        /* Algebraic identities: x+0, 0+x, x-0, x*1, 1*x,
           x*0, 0*x, and x/1. */
        if (op == '+' && !strcmp(b, "0"))
            snprintf(code[i], LEN, "%s = %s\n", lhs, a);
        else if (op == '+' && !strcmp(a, "0"))
            snprintf(code[i], LEN, "%s = %s\n", lhs, b);
        else if (op == '-' && !strcmp(b, "0"))
            snprintf(code[i], LEN, "%s = %s\n", lhs, a);
        else if (op == '*' && !strcmp(b, "1"))
            snprintf(code[i], LEN, "%s = %s\n", lhs, a);
        else if (op == '*' && !strcmp(a, "1"))
            snprintf(code[i], LEN, "%s = %s\n", lhs, b);
        else if (op == '*' && !strcmp(b, "0"))
            snprintf(code[i], LEN, "%s = 0\n", lhs);
        else if (op == '*' && !strcmp(a, "0"))
            snprintf(code[i], LEN, "%s = 0\n", lhs);
        else if (op == '/' && !strcmp(b, "1"))
            snprintf(code[i], LEN, "%s = %s\n", lhs, a);

        /* Strength reduction: multiplication by 2 becomes addition. */
        else if (op == '*' && !strcmp(b, "2"))
            snprintf(code[i], LEN, "%s = %s + %s\n", lhs, a, a);

        /* Multiplication by 4 can be represented as a left shift by 2. */
        else if (op == '*' && !strcmp(b, "4"))
            snprintf(code[i], LEN, "%s = %s << 2\n", lhs, a);

        /* Leave all other instructions unchanged. */
    }

    /* Copy propagation within basic blocks */
    {
        char names[MAX][128], values[MAX][128];
        int count = 0;

        for (int i = 0; i < n; i++) {
            char lhs[128], a[128], b[128], op;
            char *eq;

            if (strchr(code[i], ':') ||
                strstr(code[i], "goto") ||
                strstr(code[i], "ifFalse")) {
                count = 0;
                continue;
            }

            if (sscanf(code[i], "%127s = %127s %c %127s",
                       lhs, a, &op, b) == 4) {
                for (int j = count - 1; j >= 0; j--) {
                    if (!strcmp(a, names[j]))
                        strcpy(a, values[j]);
                    if (!strcmp(b, names[j]))
                        strcpy(b, values[j]);
                }

                eq = strchr(code[i], '=');
                if (eq) {
                    snprintf(code[i], LEN, "%s = %s %c %s\n",
                             lhs, a, op, b);
                }

                for (int j = 0; j < count; ) {
                    if (!strcmp(names[j], lhs)) {
                        for (int k = j; k < count - 1; k++) {
                            strcpy(names[k], names[k + 1]);
                            strcpy(values[k], values[k + 1]);
                        }
                        count--;
                    } else {
                        j++;
                    }
                }

                continue;
            }

            if (sscanf(code[i], "%127s = %127s", lhs, a) != 2)
                continue;

            for (int j = count - 1; j >= 0; j--) {
                if (!strcmp(a, names[j])) {
                    strcpy(a, values[j]);
                    break;
                }
            }

            snprintf(code[i], LEN, "%s = %s\n", lhs, a);

            for (int j = 0; j < count; ) {
                if (!strcmp(names[j], lhs)) {
                    for (int k = j; k < count - 1; k++) {
                        strcpy(names[k], names[k + 1]);
                        strcpy(values[k], values[k + 1]);
                    }
                    count--;
                } else {
                    j++;
                }
            }

            if (count < MAX) {
                strcpy(names[count], lhs);
                strcpy(values[count], a);
                count++;
            }
        }
    }

    for (int i = 0; i < n; i++)
        if (code[i][0])
            printf("%s", code[i]);

    return 0;
}
