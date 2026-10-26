#include <stdio.h>
#include <string.h>

int main(void)
{
    char line[512];
    char lhs[128], a[128], b[128], op[16];
    int label;

    while (fgets(line, sizeof(line), stdin)) {

        if (sscanf(line, "L%d:", &label) == 1) {
            printf("L%d:\n", label);
            continue;
        }

        if (sscanf(line, "goto L%d", &label) == 1) {
            printf("    JMP L%d\n", label);
            continue;
        }

        if (sscanf(line, "ifFalse %127s %15s %127s goto L%d",
                   a, op, b, &label) == 4) {

            printf("    CMP %s, %s\n", a, b);

            if (!strcmp(op, "<"))  printf("    JGE L%d\n", label);
            else if (!strcmp(op, ">")) printf("    JLE L%d\n", label);
            else if (!strcmp(op, "<=")) printf("    JG L%d\n", label);
            else if (!strcmp(op, ">=")) printf("    JL L%d\n", label);
            else if (!strcmp(op, "==")) printf("    JNE L%d\n", label);
            else if (!strcmp(op, "!=")) printf("    JE L%d\n", label);

            continue;
        }

        if (sscanf(line, "%127s = %127s %15s %127s",
                   lhs, a, op, b) == 4) {

            printf("    LOAD R0, %s\n", a);

            if (!strcmp(op, "+")) printf("    ADD R0, %s\n", b);
            else if (!strcmp(op, "-")) printf("    SUB R0, %s\n", b);
            else if (!strcmp(op, "*")) printf("    MUL R0, %s\n", b);
            else if (!strcmp(op, "/")) printf("    DIV R0, %s\n", b);
            else continue;

            printf("    STORE %s, R0\n", lhs);
            continue;
        }

        if (sscanf(line, "%127s = %127s", lhs, a) == 2) {
            printf("    MOV %s, %s\n", lhs, a);
        }
    }

    return 0;
}
