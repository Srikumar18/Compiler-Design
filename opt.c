#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    int kind;                       // 0 = label/goto, 1 = r = a, 2 = r = a op b, 3 = ifFalse a goto b
    char r[20], a[20], op[4], b[20];
    char raw[40];
} T;
int n = 0;
T c[300];

/* ---------- the "notebook" of known facts (used by methods 4 and 5) ---------- */
char kv[200][20], kc[200][20];
int nk = 0;

int isnum(char *s)  { return isdigit(s[0]) || (s[0] == '-' && isdigit(s[1])); }
int istemp(char *s) { return s[0] == 't' && isdigit(s[1]); }

char *known(char *x)                         // what is x known to equal? (NULL if nothing)
{
    for (int i = 0; i < nk; i++) if (!strcmp(kv[i], x)) return kc[i];
    return NULL;
}
void forget(char *x)                         // x changed: erase every fact that mentions it
{
    for (int i = 0; i < nk; i++)
        if (!strcmp(kv[i], x) || !strcmp(kc[i], x)) { nk--; strcpy(kv[i], kv[nk]); strcpy(kc[i], kc[nk]); i--; }
}
void learn(char *x, char *v) { forget(x); strcpy(kv[nk], x); strcpy(kc[nk], v); nk++; }

int calc(char *op, int x, int y)
{
    if (!strcmp(op, "+"))  return x + y;
    if (!strcmp(op, "-"))  return x - y;
    if (!strcmp(op, "*"))  return x * y;
    if (!strcmp(op, "/"))  return y ? x / y : 0;
    if (!strcmp(op, "<"))  return x < y;
    if (!strcmp(op, ">"))  return x > y;
    if (!strcmp(op, "<=")) return x <= y;
    if (!strcmp(op, ">=")) return x >= y;
    if (!strcmp(op, "==")) return x == y;
    return x != y;
}
void to_copy(T *t, char *v) { strcpy(t->a, v); t->kind = 1; }     // turn  r = a op b  into  r = v


/* ======================================================================
   METHOD 1 : CONSTANT FOLDING            (x = 5 * 2   ->   x = 10)
   To remove: delete this function AND the line marked [M1] in simplify()
   ====================================================================== */
int fold(T *t)
{
    char tmp[20];
    if (isnum(t->a) && isnum(t->b)) {
        sprintf(tmp, "%d", calc(t->op, atoi(t->a), atoi(t->b)));
        to_copy(t, tmp);
        return 1;
    }
    return 0;
}

/* ======================================================================
   METHOD 2 : ALGEBRAIC SIMPLIFICATION    (x+0, x-0, x*1, x/1 -> x ;  x*0 -> 0)
   To remove: delete this function AND the line marked [M2] in simplify()
   ====================================================================== */
int algebra(T *t)
{
    if (!isnum(t->b)) return 0;
    if ((!strcmp(t->op, "+") || !strcmp(t->op, "-")) && !strcmp(t->b, "0")) { to_copy(t, t->a); return 1; }
    if ((!strcmp(t->op, "*") || !strcmp(t->op, "/")) && !strcmp(t->b, "1")) { to_copy(t, t->a); return 1; }
    if (!strcmp(t->op, "*") && !strcmp(t->b, "0"))                          { to_copy(t, "0");  return 1; }
    return 0;
}

/* ======================================================================
   METHOD 3 : STRENGTH REDUCTION          (x * 8   ->   x << 3)
   To remove: delete this function AND the line marked [M3] in simplify()
   ====================================================================== */
void strength(T *t)
{
    if (strcmp(t->op, "*") || !isnum(t->b)) return;
    for (int p = 1; p < 15; p++)                   // is b equal to 2, 4, 8, 16 ... ?
        if (atoi(t->b) == (1 << p)) {
            strcpy(t->op, "<<");
            sprintf(t->b, "%d", p);
            break;
        }
}

/* ---------- simplify(): calls the methods above, one line each ---------- */
void simplify(T *t)
{
    char tmp[20];
    if (fold(t)) return;                                                   // [M1]

    if ((t->op[0] == '+' || t->op[0] == '*') && isnum(t->a) && !isnum(t->b)) {   // [SWAP] 2*x -> x*2  (helps methods 2 and 3;
        strcpy(tmp, t->a); strcpy(t->a, t->b); strcpy(t->b, tmp);                //  delete this whole if-block only if you
    }                                                                            //  remove BOTH method 2 and method 3)

    if (algebra(t)) return;                                                // [M2]
    strength(t);                                                           // [M3]
}

/* ======================================================================
   METHOD 4 : CONSTANT PROPAGATION        (a = 10 ; b = a + 3   ->   b = 10 + 3 -> 13)
   METHOD 5 : COPY PROPAGATION (temps)    (t1 = x ; a = t1      ->   a = x)
   Both work through the notebook.  Each is ONE line below, marked [M4] / [M5].
   To remove a method: delete its one line.
   ====================================================================== */
int worth_remembering(T *t)                  // after this line, do we know something about t->r ?
{
    if (t->kind != 1) return 0;
    if (isnum(t->a))  return 1;              // [M4]  r = 10
    if (istemp(t->r)) return 1;              // [M5]  t1 = x
    return 0;
}

/* ======================================================================
   METHOD 6 : DEAD TEMPORARY REMOVAL      (delete  t1 = 10  when nobody reads t1)
   To remove: delete this function AND the line marked [M6] in main()
   ====================================================================== */
void remove_unused_temps()
{
    for (int i = 0; i < n; i++) {
        T *t = &c[i];
        if (t->kind == 0 || t->kind == 3 || !istemp(t->r)) continue;
        int used = 0;
        for (int j = 0; j < n; j++) {
            if (j == i || c[j].kind == 0) continue;
            if (!strcmp(c[j].a, t->r) || (c[j].kind != 1 && !strcmp(c[j].b, t->r)))
                used = 1;
        }
        if (!used) t->kind = -1;             // -1 = deleted
    }
}


int main()
{
    char line[100];

    /* ---- read the three-address code ---- */
    while (fgets(line, sizeof(line), stdin)) {
        T *t = &c[n++];
        if (sscanf(line, "%s = %s %s %s", t->r, t->a, t->op, t->b) == 4)
            t->kind = 2;
        else if (sscanf(line, "%s = %s", t->r, t->a) == 2)
            t->kind = 1;
        else if (sscanf(line, "ifFalse %s goto %s", t->a, t->b) == 2)
            t->kind = 3;
        else {
            sscanf(line, "%[^\n]", t->raw);
            t->kind = 0;
        }
    }

    /* ---- go through the lines once, in order ---- */
    for (int i = 0; i < n; i++) {
        T *t = &c[i];
        if (t->kind == 0 && strchr(t->raw, ':')) { nk = 0; continue; }     // label: forget everything we knew
        if (t->kind == 0) continue;

        char *v;
        if ((v = known(t->a)) != NULL) strcpy(t->a, v);                    // use what we know (methods 4 and 5)

        if (t->kind == 3) continue;

        if (t->kind == 2) {
            if ((v = known(t->b)) != NULL) strcpy(t->b, v);                // use what we know (methods 4 and 5)
            simplify(t);                                                   // methods 1, 2, 3
        }

        if (worth_remembering(t)) learn(t->r, t->a);                       // methods 4 and 5
        else                      forget(t->r);
    }

    remove_unused_temps();                                                 // [M6]

    /* ---- print ---- */
    for (int i = 0; i < n; i++) {
        T *t = &c[i];
        if (t->kind == 0) printf("%s\n", t->raw);
        if (t->kind == 1) printf("%s = %s\n", t->r, t->a);
        if (t->kind == 2) printf("%s = %s %s %s\n", t->r, t->a, t->op, t->b);
        if (t->kind == 3) printf("ifFalse %s goto %s\n", t->a, t->b);
    }
    return 0;
}