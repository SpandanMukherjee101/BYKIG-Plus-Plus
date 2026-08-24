#ifndef BUILTINS_H
#define BUILTINS_H

#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char builtin_string_return[1000000];
float val(char* expr);

typedef float (*BuiltinHandler)(char args[][1000], int argCount);

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

// In BYKIG v2 string literals often come with quotes in args
static inline void strip_quotes(char *s) {
    if (s[0] == '"') {
        int l = strlen(s);
        if (s[l-1] == '"') {
            s[l-1] = '\0';
            memmove(s, s+1, l-1);
        }
    }
}

static inline float builtin_len(char args[][1000], int argCount) {
    char name[100];
    strcpy(name, args[0]);
    trim_spaces(name);
    return scopeGetLen(name);
}

static inline float builtin_push(char args[][1000], int argCount) {
    char name[100];
    strcpy(name, args[0]);
    trim_spaces(name);
    float valData = val(args[1]);
    scopePushL(name, valData);
    return 0.0;
}

static inline float builtin_pop(char args[][1000], int argCount) {
    char name[100];
    strcpy(name, args[0]);
    trim_spaces(name);
    return scopePopL(name);
}

static inline float builtin_map_exists(char args[][1000], int argCount) {
    char name[100];
    strcpy(name, args[0]);
    trim_spaces(name);
    char key[1000];
    strcpy(key, args[1]);
    trim_spaces(key);
    strip_quotes(key);
    return scopeMapExists(name, key);
}

static inline float builtin_map_delete(char args[][1000], int argCount) {
    char name[100];
    strcpy(name, args[0]);
    trim_spaces(name);
    char key[1000];
    strcpy(key, args[1]);
    trim_spaces(key);
    strip_quotes(key);
    scopeMapDelete(name, key);
    return 0.0;
}

static inline float builtin_file_exists(char args[][1000], int argCount) {
    char filename[1000];
    strcpy(filename, args[0]);
    trim_spaces(filename);
    strip_quotes(filename);
    FILE *f = fopen(filename, "r");
    if (f) {
        fclose(f);
        return 1.0;
    }
    return 0.0;
}

static inline float builtin_file_write(char args[][1000], int argCount) {
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
        return 1.0;
    }
    return 0.0;
}

static inline float builtin_file_append(char args[][1000], int argCount) {
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
        return 1.0;
    }
    return 0.0;
}

static inline float builtin_file_read(char args[][1000], int argCount) {
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
        return 1.0;
    }
    strcpy(builtin_string_return, "");
    return 0.0;
}

static struct BuiltinFunc builtins[] = {
    {"len", 1, builtin_len},
    {"push", 2, builtin_push},
    {"pop", 1, builtin_pop},
    {"exists", 2, builtin_map_exists},
    {"delete", 2, builtin_map_delete},
    {"file_exists", 1, builtin_file_exists},
    {"file_write", 2, builtin_file_write},
    {"file_append", 2, builtin_file_append},
    {"file_read", 1, builtin_file_read},
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