#ifndef BUILTINS_H
#define BUILTINS_H

#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char builtin_string_return[1000000];
struct Value val(char* expr);

typedef struct Value (*BuiltinHandler)(char args[][1000], int argCount);

struct BuiltinFunc {
    char name[100];
    int expectedArgs;
    BuiltinHandler handler;
};

static inline void trim_spaces(char *s) {
    char *p = s;
    int l = strlen(p);
    while(l > 0 && isspace(p[l - 1])) p[--l] = 0;
    while(*p && isspace(*p)) ++p, --l;
    memmove(s, p, l + 1);
}

 
static inline void strip_quotes(char *s) {
    if (s[0] == '"') {
        int l = strlen(s);
        if (s[l-1] == '"') {
            s[l-1] = '\0';
            memmove(s, s+1, l-1);
        }
    }
}

static inline struct Value builtin_len(char args[][1000], int argCount) {
    char name[100];
    strcpy(name, args[0]);
    trim_spaces(name);
    strip_quotes(name);
    if (typeFetcher(name) != 0) {
        struct Value retV; retV.type=VAL_FLOAT; retV.f=scopeGetLen(name); return retV;
    }
    char exprBuf[1000];
    snprintf(exprBuf, sizeof(exprBuf), "%s ;", args[0]);
    struct Value v = val(exprBuf);
    struct Value retV; retV.type=VAL_FLOAT; retV.f=0.0;
    if (v.type == VAL_STRING) retV.f = (float)strlen(v.s);
    else if (v.type == VAL_LIST && v.ptr) {
        struct lV *lv = (struct lV*)v.ptr;
        while (lv && lv->ref) lv = lv->ref;
        if (lv) retV.f = (float)lv->size;
    } else if (v.type == VAL_MAP && v.ptr) {
        struct mV *mv = (struct mV*)v.ptr;
        if (mv) retV.f = (float)mv->count;
    } else if (v.type == VAL_INT_ARRAY && v.ptr) {
        struct iA *ia = (struct iA*)v.ptr;
        if (ia) retV.f = (float)ia->size;
    } else if (v.type == VAL_FLOAT_ARRAY && v.ptr) {
        struct fA *fa = (struct fA*)v.ptr;
        if (fa) retV.f = (float)fa->size;
    } else if (v.type == VAL_CHAR_ARRAY && v.ptr) {
        struct cA *ca = (struct cA*)v.ptr;
        if (ca) retV.f = (float)ca->size;
    } else if (v.type == VAL_BOOL_ARRAY && v.ptr) {
        struct bA *ba = (struct bA*)v.ptr;
        if (ba) retV.f = (float)ba->size;
    }
    return retV;
}

static inline void extract_key_str(char *dest, size_t dest_sz, char *arg) {
    struct Value keyVal = val(arg);
    if (keyVal.type == VAL_STRING) {
        strncpy(dest, keyVal.s, dest_sz - 1);
        dest[dest_sz - 1] = '\0';
    } else if (keyVal.type == VAL_FLOAT) {
        if (keyVal.f == (float)(int)keyVal.f) snprintf(dest, dest_sz, "%d", (int)keyVal.f);
        else snprintf(dest, dest_sz, "%f", keyVal.f);
    } else {
        strncpy(dest, arg, dest_sz - 1);
        dest[dest_sz - 1] = '\0';
        trim_spaces(dest);
        strip_quotes(dest);
    }
}

static inline struct Value builtin_push(char args[][1000], int argCount) {
    char name[100];
    strcpy(name, args[0]);
    trim_spaces(name);
    strip_quotes(name);
    if (typeFetcher(name) == 11) {
        if (argCount == 3) {
            char key[1000] = "";
            extract_key_str(key, sizeof(key), args[1]);
            setM_Val(NULL, name, key, val(args[2]));
        }
    } else if (typeFetcher(name) == 10) {
        if (argCount == 2) {
            scopePushL(name, val(args[1]));
        }
    } else {
        struct Value target = val(args[0]);
        if (target.type == VAL_MAP && target.ptr && argCount == 3) {
            struct mV *mv = (struct mV*)target.ptr;
            char key[1000] = "";
            extract_key_str(key, sizeof(key), args[1]);
            struct Value valV = val(args[2]);
            int found = 0;
            for (int e = 0; e < mv->count; e++) {
                if (!strcmp(mv->entries[e].key, key)) {
                    mv->entries[e].value = valV;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                if (mv->count >= mv->capacity) {
                    int newCap = (mv->capacity == 0) ? 4 : mv->capacity * 2;
                    mv->entries = (struct mapEntry*)realloc(mv->entries, sizeof(struct mapEntry) * newCap);
                    mv->capacity = newCap;
                }
                strcpy(mv->entries[mv->count].key, key);
                mv->entries[mv->count].value = valV;
                mv->count++;
            }
        } else if (target.type == VAL_LIST && target.ptr && argCount == 2) {
            struct lV *lv = (struct lV*)target.ptr;
            while (lv && lv->ref) lv = lv->ref;
            if (lv) {
                if (lv->size >= lv->capacity) {
                    int newCap = (lv->capacity == 0) ? 4 : lv->capacity * 2;
                    lv->data = (struct Value*)realloc(lv->data, sizeof(struct Value) * newCap);
                    lv->capacity = newCap;
                }
                lv->data[lv->size++] = val(args[1]);
            }
        }
    }
    struct Value retV; retV.type=VAL_FLOAT; retV.f=0.0; return retV;
}

static inline struct Value builtin_pop(char args[][1000], int argCount) {
    char name[100];
    strcpy(name, args[0]);
    trim_spaces(name);
    strip_quotes(name);
    struct Value retV; retV.type=VAL_FLOAT; retV.f=0.0;
    if (typeFetcher(name) == 11) {
        if (argCount == 2) {
            char key[1000] = "";
            extract_key_str(key, sizeof(key), args[1]);
            retV = getM_Val(NULL, name, key);
            scopeMapDelete(name, key);
        }
    } else if (typeFetcher(name) == 10) {
        if (argCount == 1) {
            retV = scopePopL(name);
        }
    } else {
        struct Value target = val(args[0]);
        if (target.type == VAL_LIST && target.ptr && argCount == 1) {
            struct lV *lv = (struct lV*)target.ptr;
            while (lv && lv->ref) lv = lv->ref;
            if (lv && lv->size > 0) {
                retV = lv->data[--lv->size];
            }
        } else if (target.type == VAL_MAP && target.ptr && argCount == 2) {
            struct mV *mv = (struct mV*)target.ptr;
            char key[1000] = "";
            extract_key_str(key, sizeof(key), args[1]);
            if (mv) {
                for (int e = 0; e < mv->count; e++) {
                    if (!strcmp(mv->entries[e].key, key)) {
                        retV = mv->entries[e].value;
                        for (int k = e; k < mv->count - 1; k++) {
                            mv->entries[k] = mv->entries[k + 1];
                        }
                        mv->count--;
                        break;
                    }
                }
            }
        }
    }
    return retV;
}

static inline struct Value builtin_map_exists(char args[][1000], int argCount) {
    char name[100];
    strcpy(name, args[0]);
    trim_spaces(name);
    strip_quotes(name);
    struct Value retV; retV.type=VAL_FLOAT; retV.f=0.0;
    if (typeFetcher(name) != 0) {
        if (argCount == 1) {
            retV.f = scopeValueExists(name);
        } else if (argCount == 2) {
            char key[1000] = "";
            extract_key_str(key, sizeof(key), args[1]);
            retV.f = scopeMapExists(name, key);
        }
    } else {
        struct Value target = val(args[0]);
        if (target.type == VAL_MAP && target.ptr && argCount == 2) {
            struct mV *mv = (struct mV*)target.ptr;
            char key[1000] = "";
            extract_key_str(key, sizeof(key), args[1]);
            if (mv) {
                for (int e = 0; e < mv->count; e++) {
                    if (!strcmp(mv->entries[e].key, key)) {
                        retV.f = 1.0;
                        break;
                    }
                }
            }
        }
    }
    return retV;
}

static inline struct Value builtin_map_delete(char args[][1000], int argCount) {
    char name[100];
    strcpy(name, args[0]);
    trim_spaces(name);
    strip_quotes(name);
    if (typeFetcher(name) != 0) {
        if (argCount == 1) {
            scopeDeleteValue(name);
        } else if (argCount == 2) {
            char key[1000] = "";
            extract_key_str(key, sizeof(key), args[1]);
            scopeMapDelete(name, key);
        }
    } else {
        struct Value target = val(args[0]);
        if (target.type == VAL_MAP && target.ptr && argCount == 2) {
            struct mV *mv = (struct mV*)target.ptr;
            char key[1000] = "";
            extract_key_str(key, sizeof(key), args[1]);
            if (mv) {
                for (int e = 0; e < mv->count; e++) {
                    if (!strcmp(mv->entries[e].key, key)) {
                        for (int k = e; k < mv->count - 1; k++) {
                            mv->entries[k] = mv->entries[k + 1];
                        }
                        mv->count--;
                        break;
                    }
                }
            }
        }
    }
    struct Value retV; retV.type=0; retV.f=0.0; return retV;
}

static inline struct Value builtin_file_exists(char args[][1000], int argCount) {
    char filename[1000];
    strcpy(filename, args[0]);
    trim_spaces(filename);
    strip_quotes(filename);
    FILE *f = fopen(filename, "r");
    if (f) {
        fclose(f);
        struct Value retV; retV.type=VAL_FLOAT; retV.f=1.0; return retV;
    }
    struct Value retV; retV.type=VAL_FLOAT; retV.f=0.0; return retV;
}

static inline struct Value builtin_file_write(char args[][1000], int argCount) {
    char filename[1000];
    strcpy(filename, args[0]);
    trim_spaces(filename);
    strip_quotes(filename);
    char content[1000];
    strcpy(content, args[1]);
    trim_spaces(content);
    strip_quotes(content);
    FILE *f = fopen(filename, "w");
    if (f) {
        fprintf(f, "%s", content);
        fclose(f);
        struct Value retV; retV.type=VAL_FLOAT; retV.f=1.0; return retV;
    }
    struct Value retV; retV.type=VAL_FLOAT; retV.f=0.0; return retV;
}

static inline struct Value builtin_file_append(char args[][1000], int argCount) {
    char filename[1000];
    strcpy(filename, args[0]);
    trim_spaces(filename);
    strip_quotes(filename);
    char content[1000];
    strcpy(content, args[1]);
    trim_spaces(content);
    strip_quotes(content);
    FILE *f = fopen(filename, "a");
    if (f) {
        fprintf(f, "%s", content);
        fclose(f);
        struct Value retV; retV.type=VAL_FLOAT; retV.f=1.0; return retV;
    }
    struct Value retV; retV.type=VAL_FLOAT; retV.f=0.0; return retV;
}

static inline struct Value builtin_file_read(char args[][1000], int argCount) {
    char filename[1000];
    strcpy(filename, args[0]);
    trim_spaces(filename);
    strip_quotes(filename);
    FILE *f = fopen(filename, "r");
    if (f) {
        char content[1000] = "";
        char line[200];
        while (fgets(line, sizeof(line), f)) {
            strcat(content, line);
        }
        fclose(f);
        strcpy(builtin_string_return, content);
        struct Value retV; retV.type=VAL_FLOAT; retV.f=1.0; return retV;
    }
    strcpy(builtin_string_return, "");
    struct Value retV; retV.type=VAL_FLOAT; retV.f=0.0; return retV;
}

static FILE *scanner_fp = NULL;
static int scanner_last_was_str = 0;

static inline struct Value builtin_file_scan_is_str(char args[][1000], int argCount) {
    struct Value retV;
    retV.type = VAL_FLOAT;
    retV.f = (float)scanner_last_was_str;
    retV.s[0] = '\0';
    return retV;
}

static inline struct Value builtin_file_scan_open(char args[][1000], int argCount) {
    scanner_last_was_str = 0;
    if (scanner_fp != NULL) {
        fclose(scanner_fp);
        scanner_fp = NULL;
    }
    struct Value argVal = val(args[0]);
    char filename[1000] = "";
    if (argVal.type == 1) {  
        strcpy(filename, argVal.s);
    } else {
        strcpy(filename, args[0]);
        trim_spaces(filename);
        strip_quotes(filename);
    }
    scanner_fp = fopen(filename, "rb");
    struct Value retV; retV.type=VAL_FLOAT; 
    retV.f = (scanner_fp != NULL) ? 1.0 : 0.0;
    return retV;
}

static inline struct Value builtin_file_scan_next(char args[][1000], int argCount) {
    if (scanner_fp != NULL) {
        int c;
        while ((c = fgetc(scanner_fp)) != EOF) {
            if (isspace(c)) continue;
            if (c == '/') {
                int next_c = fgetc(scanner_fp);
                if (next_c == '/') {
                    while ((c = fgetc(scanner_fp)) != EOF && c != '\n');
                    continue;
                } else if (next_c != EOF) {
                    ungetc(next_c, scanner_fp);
                }
            }
            if (c == '"') {
                char token[1000];
                int idx = 0;
                while ((c = fgetc(scanner_fp)) != EOF && c != '"' && idx < 995) {
                    token[idx++] = (char)c;
                }
                token[idx] = '\0';
                scanner_last_was_str = 1;
                strcpy(builtin_string_return, token);
                struct Value retV; retV.type=VAL_STRING; strcpy(retV.s, token); retV.f=1.0; return retV;
            }
            scanner_last_was_str = 0;
            if (c == '[' || c == ']' || c == '{' || c == '}' || c == '(' || c == ')' || c == ',' || c == ';') {
                char token[2];
                token[0] = (char)c;
                token[1] = '\0';
                strcpy(builtin_string_return, token);
                struct Value retV; retV.type=VAL_STRING; strcpy(retV.s, token); retV.f=1.0; return retV;
            }
            char token[1000];
            int idx = 0;
            token[idx++] = (char)c;
            while ((c = fgetc(scanner_fp)) != EOF && !isspace(c) && idx < 995) {
                if (c == '[' || c == ']' || c == '{' || c == '}' || c == '(' || c == ')' || c == ',' || c == ';' || c == '"') {
                    ungetc(c, scanner_fp);
                    break;
                }
                token[idx++] = (char)c;
            }
            token[idx] = '\0';
            strcpy(builtin_string_return, token);
            struct Value retV; retV.type=VAL_STRING; strcpy(retV.s, token); retV.f=1.0; return retV;
        }
    }
    scanner_last_was_str = 0;
    strcpy(builtin_string_return, "EOF");
    struct Value retV; retV.type=VAL_STRING; strcpy(retV.s, "EOF"); retV.f=0.0; return retV;
}

static inline struct Value builtin_eval(char args[][1000], int argCount) {
    struct Value retV;
    retV.type = VAL_FLOAT;
    retV.f = 0.0;
    retV.s[0] = '\0';
    if (argCount > 0) {
        char expr[2000] = "";
        char *raw = args[0];
        while (*raw == ' ' || *raw == '\t') raw++;
        struct Value argVal = val(raw);
        if (argVal.type == VAL_STRING) {
            snprintf(expr, sizeof(expr), "%s ;", argVal.s);
            retV = val(expr);
        } else {
            retV = argVal;
        }
        if (retV.type == VAL_STRING) {
            strcpy(builtin_string_return, retV.s);
        } else if (retV.type == VAL_FLOAT) {
            if (retV.f == (float)(int)retV.f) snprintf(builtin_string_return, sizeof(builtin_string_return), "%d", (int)retV.f);
            else snprintf(builtin_string_return, sizeof(builtin_string_return), "%f", retV.f);
        }
    }
    return retV;
}

static inline struct Value builtin_strip_quotes(char args[][1000], int argCount) {
    char s[1000] = "";
    if (argCount > 0) {
        char exprBuf[1000];
    snprintf(exprBuf, sizeof(exprBuf), "%s ;", args[0]);
    struct Value v = val(exprBuf);
        if (v.type == VAL_STRING) {
            strcpy(s, v.s);
        } else {
            strcpy(s, args[0]);
            trim_spaces(s);
        }
        strip_quotes(s);
    }
    strcpy(builtin_string_return, s);
    struct Value retV;
    retV.type = VAL_STRING;
    strcpy(retV.s, s);
    return retV;
}

static inline struct Value builtin_quote(char args[][1000], int argCount) {
    char s[1000] = "";
    if (argCount > 0) {
        char arg[1000];
        strcpy(arg, args[0]);
        int l = strlen(arg);
        if (!(l >= 2 && arg[0] == '"' && arg[l-1] == '"')) {
            while (l > 0 && isspace((unsigned char)arg[l-1])) arg[--l] = '\0';
        }
        char *sv = scopeGetSV(arg);
        if (sv) {
            strcpy(s, sv);
        } else {
            strcpy(s, arg);
            strip_quotes(s);
        }
    }
    char res[1000];
    snprintf(res, sizeof(res), "\"%s\"", s);
    strcpy(builtin_string_return, res);
    struct Value retV;
    retV.type = VAL_STRING;
    strcpy(retV.s, res);
    return retV;
}

static inline struct Value builtin_is_num(char args[][1000], int argCount) {
    struct Value retV;
    retV.type = VAL_FLOAT;
    retV.f = 0.0;
    if (argCount > 0) {
        char exprBuf[1000];
    snprintf(exprBuf, sizeof(exprBuf), "%s ;", args[0]);
    struct Value v = val(exprBuf);
        char s[1000] = "";
        if (v.type == VAL_STRING) strcpy(s, v.s);
        else if (v.type == VAL_FLOAT) { retV.f = 1.0; return retV; }
        else { strcpy(s, args[0]); trim_spaces(s); strip_quotes(s); }
        trim_spaces(s);
        if (s[0] != '\0') {
            int i = 0;
            if (s[0] == '-' || s[0] == '+') i++;
            int hasDigits = 0;
            int hasDot = 0;
            int ok = 1;
            for (; s[i] != '\0'; i++) {
                if (isdigit(s[i])) hasDigits = 1;
                else if (s[i] == '.' && !hasDot) hasDot = 1;
                else { ok = 0; break; }
            }
            if (ok && hasDigits) retV.f = 1.0;
        }
    }
    return retV;
}

static inline struct Value builtin_is_str(char args[][1000], int argCount) {
    struct Value retV; retV.type = VAL_FLOAT; retV.f = 0.0;
    if (argCount > 0) {
        char *raw = args[0];
        while (*raw == ' ' || *raw == '\t') raw++;
        if (*raw == '"') {
            retV.f = 1.0;
            return retV;
        }
        struct Value v = val(raw);
        if (v.type == VAL_STRING) {
            retV.f = 1.0;
            return retV;
        }
    }
    return retV;
}

static inline struct Value builtin_starts_with(char args[][1000], int argCount) {
    struct Value retV;
    retV.type = VAL_FLOAT;
    retV.f = 0.0;
    if (argCount >= 2) {
        struct Value v1 = val(args[0]);
        struct Value v2 = val(args[1]);
        char s1[1000] = "", s2[1000] = "";
        if (v1.type == VAL_STRING) strcpy(s1, v1.s);
        else { strcpy(s1, args[0]); trim_spaces(s1); }
        if (v2.type == VAL_STRING) strcpy(s2, v2.s);
        else { strcpy(s2, args[1]); trim_spaces(s2); }
        if (strlen(s2) > 0 && strncmp(s1, s2, strlen(s2)) == 0) {
            retV.f = 1.0;
        }
    }
    return retV;
}

static inline struct Value builtin_file_scan_close(char args[][1000], int argCount) {
    if (scanner_fp != NULL) {
        fclose(scanner_fp);
        scanner_fp = NULL;
    }
    struct Value retV; retV.type=VAL_FLOAT; retV.f=1.0; return retV;
}

static inline struct Value builtin_file_scan_get_pos(char args[][1000], int argCount) {
    float pos = -1.0;
    if (scanner_fp != NULL) {
        pos = (float)ftell(scanner_fp);
    }
    struct Value retV; retV.type=VAL_FLOAT; retV.f=pos; return retV;
}

static inline struct Value builtin_file_scan_set_pos(char args[][1000], int argCount) {
    if (scanner_fp != NULL) {
        float pos = val(args[0]).f;
        fseek(scanner_fp, (long)pos, SEEK_SET);
    }
    struct Value retV; retV.type=VAL_FLOAT; retV.f=1.0; return retV;
}

static inline struct Value builtin_parse_float(char args[][1000], int argCount) {
    struct Value arg = val(args[0]);
    float parsed = 0.0;
    if (arg.type == 1) {  
        parsed = atof(arg.s);
    } else if (arg.type == 0) {  
        parsed = arg.f;
    }
    struct Value retV; retV.type=0; retV.f=parsed; return retV;
}

static inline struct Value builtin_class_exists(char args[][1000], int argCount) {
    struct Value retV; retV.type = VAL_FLOAT; retV.f = 0.0;
    if (argCount >= 1) {
        char name[100];
        strncpy(name, args[0], sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        trim_spaces(name);
        strip_quotes(name);
        if (findClass(CLASS_REGISTRY, name) != NULL) {
            retV.f = 1.0;
        }
    }
    return retV;
}

static inline struct Value builtin_object_id(char args[][1000], int argCount) {
    struct Value retV; retV.type = VAL_FLOAT; retV.f = 0.0;
    if (argCount >= 1) {
        char exprBuf[1000];
        snprintf(exprBuf, sizeof(exprBuf), "%s ;", args[0]);
        struct Value v = val(exprBuf);
        if (v.type == VAL_OBJECT && v.ptr != NULL) {
            struct objInstance *inst = (struct objInstance*)v.ptr;
            retV.f = (float)inst->id;
        }
    }
    return retV;
}

static inline struct Value builtin_is_null(char args[][1000], int argCount) {
    struct Value retV; retV.type = VAL_FLOAT; retV.f = 0.0;
    if (argCount >= 1) {
        char exprBuf[1000];
        snprintf(exprBuf, sizeof(exprBuf), "%s ;", args[0]);
        struct Value v = val(exprBuf);
        if (v.type == VAL_OBJECT && v.ptr == NULL) {
            retV.f = 1.0;
        }
    }
    return retV;
}

static inline struct Value builtin_is_instance(char args[][1000], int argCount) {
    struct Value retV;
    retV.type = VAL_FLOAT;
    retV.f = 0.0f;
    retV.ptr = NULL;
    retV.s[0] = '\0';

    if (argCount < 2) {
        return retV;
    }

    // 1. Resolve and validate target class name
    char targetClassName[100] = "";
    char clsExpr[1000];
    snprintf(clsExpr, sizeof(clsExpr), "%s ;", args[1]);
    struct Value cVal = val(clsExpr);
    if (cVal.type == VAL_STRING && cVal.s[0] != '\0') {
        snprintf(targetClassName, sizeof(targetClassName), "%.99s", cVal.s);
    } else {
        snprintf(targetClassName, sizeof(targetClassName), "%.99s", args[1]);
    }
    targetClassName[sizeof(targetClassName) - 1] = '\0';
    trim_spaces(targetClassName);
    strip_quotes(targetClassName);
    trim_spaces(targetClassName);

    if (targetClassName[0] == '\0') {
        return retV;
    }

    struct classDef *targetCls = findClass(CLASS_REGISTRY, targetClassName);
    if (!targetCls) {
        // Unknown target class name returns 0 (false) safely
        return retV;
    }

    // 2. Evaluate object expression
    char objExpr[1000];
    snprintf(objExpr, sizeof(objExpr), "%s ;", args[0]);
    struct Value oVal = val(objExpr);

    // 3. Non-object values and null references return 0 (false)
    if (oVal.type != VAL_OBJECT || oVal.ptr == NULL) {
        return retV;
    }

    struct objInstance *inst = (struct objInstance*)oVal.ptr;
    if (!inst->cls) {
        return retV;
    }

    // 4. Check if inst->cls is targetCls or a subclass of targetCls
    if (isSubclassOf(inst->cls, targetCls)) {
        retV.f = 1.0f;
    }

    return retV;
}

static struct BuiltinFunc builtins[] = {
    {"len", 1, builtin_len},
    {"push", 0, builtin_push},
    {"pop", 0, builtin_pop},
    {"exists", 0, builtin_map_exists},
    {"class_exists", 1, builtin_class_exists},
    {"object_id", 1, builtin_object_id},
    {"is_null", 1, builtin_is_null},
    {"is_instance", 2, builtin_is_instance},
    {"delete", 0, builtin_map_delete},
    {"file_exists", 1, builtin_file_exists},
    {"file_write", 2, builtin_file_write},
    {"file_append", 2, builtin_file_append},
    {"file_read", 1, builtin_file_read},
    {"file_scan_open", 1, builtin_file_scan_open},
    {"file_scan_next", 0, builtin_file_scan_next},
    {"file_scan_close", 0, builtin_file_scan_close},
    {"file_scan_get_pos", 0, builtin_file_scan_get_pos},
    {"file_scan_set_pos", 1, builtin_file_scan_set_pos},
    {"parse_float", 1, builtin_parse_float},
    {"eval", 1, builtin_eval},
    {"strip_quotes", 1, builtin_strip_quotes},
    {"quote", 1, builtin_quote},
    {"is_num", 1, builtin_is_num},
    {"starts_with", 2, builtin_starts_with},
    {"is_str", 1, builtin_is_str},
    {"file_scan_is_str", 0, builtin_file_scan_is_str},
    {"", 0, NULL}
};

static inline struct BuiltinFunc* findBuiltin(const char *name) {
    for (int i = 0; builtins[i].handler != NULL; i++) {
        if (!strcmp(builtins[i].name, name)) {
            return &builtins[i];
        }
    }
    return NULL;
}
#endif
