
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum ValueType {
    VAL_FLOAT,
    VAL_STRING,
    VAL_INT_ARRAY,
    VAL_FLOAT_ARRAY,
    VAL_CHAR_ARRAY,
    VAL_BOOL_ARRAY,
    VAL_LIST,
    VAL_MAP,
    VAL_OBJECT,
    VAL_ERROR
};

struct Value {
    enum ValueType type;
    float f;
    char s[1000];
    void *ptr;
};

struct iV { char name[100]; int data; struct iV *next; };
struct fV { char name[100]; float data; struct fV *next; };
struct cV { char name[100]; char data; struct cV *next; };
struct bV { char name[100]; int data; struct bV *next; };
struct sV { char name[100]; char data[1000]; struct sV *next; };

struct iA { char name[100]; int size; int *data; struct iA *next; };
struct fA { char name[100]; int size; float *data; struct fA *next; };
struct cA { char name[100]; int size; char *data; struct cA *next; };
struct bA { char name[100]; int size; int *data; struct bA *next; };

struct mapEntry {
    char key[100];
    struct Value value;
};

struct mV {
    char name[100];
    int count;
    int capacity;
    struct mapEntry *entries;
    struct mV *next;
};

struct funcParam { int type; char name[100]; };
struct funcDef { char name[100]; int paramCount; struct funcParam params[10]; long bodyPos; char filepath[1000]; struct funcDef *next; };

#define MAX_CLASS_FIELDS 64
#define MAX_CLASS_METHODS 64

struct classDef;

struct classField {
    char name[100];
    int type;          // 1=int, 2=float, 3=char, 4=bool, 5=string, 6=intarr, 7=floatarr, 8=chararr, 9=boolarr, 10=list, 11=map, 12=object
    int isPrivate;     // 0=public, 1=private
    struct classDef *declaringClass;
};

struct classMethod {
    char name[100];
    int isPrivate;     // 0=public, 1=private
    struct classDef *declaringClass;
    struct funcDef *func;
};

struct classDef {
    char name[100];
    char parentName[100];
    struct classDef *parent;
    
    struct classField fields[MAX_CLASS_FIELDS];
    int fieldCount;
    
    struct classMethod methods[MAX_CLASS_METHODS];
    int methodCount;
    
    struct funcDef *constructor;
    struct classDef *next;
};

extern struct classDef *CLASS_REGISTRY;

struct objFieldVal {
    char name[100];
    int type;          // 1=int, 2=float, 3=char, 4=bool, 5=string, 6=intarr, 7=floatarr, 8=chararr, 9=boolarr, 10=list, 11=map, 12=object
    int isPrivate;     // 0=public, 1=private
    struct classDef *declaringClass;
    struct Value val;  // Unified runtime Value representation
};

struct objInstance {
    int id;                     // Unique integer instance ID
    struct classDef *cls;       // Class definition
    int fieldCount;
    struct objFieldVal *fields; // Dynamically allocated field array
    struct objInstance *next;   // Linked list in global OBJECT_HEAP
};

extern struct objInstance *OBJECT_HEAP;
extern int NEXT_OBJECT_ID;

struct oV {
    char name[100];
    char className[100];
    struct objInstance *instance;
    struct oV *next;
};

struct savedVar { char name[100]; int type; int intVal; float floatVal; char charVal; char strVal[1000]; };

struct lV { char name[100]; int size; int capacity; struct Value *data; struct lV *ref; struct lV *next; };

static inline void value_to_string(struct Value v, char *out, size_t max_len) {
    if (max_len == 0 || !out) return;
    out[0] = '\0';
    if (v.type == VAL_FLOAT) {
        if (v.f == (float)(int)v.f) snprintf(out, max_len, "%d", (int)v.f);
        else snprintf(out, max_len, "%f", v.f);
    } else if (v.type == VAL_STRING) {
        snprintf(out, max_len, "%s", v.s);
    } else if (v.type == VAL_LIST) {
        struct lV *lv = (struct lV*)v.ptr;
        while (lv && lv->ref) lv = lv->ref;
        if (!lv) { snprintf(out, max_len, "[]"); return; }
        snprintf(out, max_len, "[");
        for (int i = 0; i < lv->size; i++) {
            char item[512] = "";
            value_to_string(lv->data[i], item, sizeof(item));
            if (i > 0) strncat(out, ", ", max_len - strlen(out) - 1);
            strncat(out, item, max_len - strlen(out) - 1);
        }
        strncat(out, "]", max_len - strlen(out) - 1);
    } else if (v.type == VAL_MAP) {
        struct mV *mv = (struct mV*)v.ptr;
        if (!mv) { snprintf(out, max_len, "{}"); return; }
        snprintf(out, max_len, "{");
        for (int i = 0; i < mv->count; i++) {
            char item[512] = "";
            value_to_string(mv->entries[i].value, item, sizeof(item));
            if (i > 0) strncat(out, ", ", max_len - strlen(out) - 1);
            char entryBuf[600];
            snprintf(entryBuf, sizeof(entryBuf), "\"%s\": %s", mv->entries[i].key, item);
            strncat(out, entryBuf, max_len - strlen(out) - 1);
        }
        strncat(out, "}", max_len - strlen(out) - 1);
    } else if (v.type == VAL_INT_ARRAY) {
        struct iA *ia = (struct iA*)v.ptr;
        snprintf(out, max_len, "[");
        if (ia) {
            for (int i = 0; i < ia->size; i++) {
                char item[32];
                snprintf(item, sizeof(item), "%d", ia->data[i]);
                if (i > 0) strncat(out, ", ", max_len - strlen(out) - 1);
                strncat(out, item, max_len - strlen(out) - 1);
            }
        }
        strncat(out, "]", max_len - strlen(out) - 1);
    } else if (v.type == VAL_FLOAT_ARRAY) {
        struct fA *fa = (struct fA*)v.ptr;
        snprintf(out, max_len, "[");
        if (fa) {
            for (int i = 0; i < fa->size; i++) {
                char item[32];
                if (fa->data[i] == (float)(int)fa->data[i]) snprintf(item, sizeof(item), "%d", (int)fa->data[i]);
                else snprintf(item, sizeof(item), "%f", fa->data[i]);
                if (i > 0) strncat(out, ", ", max_len - strlen(out) - 1);
                strncat(out, item, max_len - strlen(out) - 1);
            }
        }
        strncat(out, "]", max_len - strlen(out) - 1);
    } else if (v.type == VAL_CHAR_ARRAY) {
        struct cA *ca = (struct cA*)v.ptr;
        snprintf(out, max_len, "[");
        if (ca) {
            for (int i = 0; i < ca->size; i++) {
                char item[8];
                snprintf(item, sizeof(item), "'%c'", ca->data[i]);
                if (i > 0) strncat(out, ", ", max_len - strlen(out) - 1);
                strncat(out, item, max_len - strlen(out) - 1);
            }
        }
        strncat(out, "]", max_len - strlen(out) - 1);
    } else if (v.type == VAL_BOOL_ARRAY) {
        struct bA *ba = (struct bA*)v.ptr;
        snprintf(out, max_len, "[");
        if (ba) {
            for (int i = 0; i < ba->size; i++) {
                if (i > 0) strncat(out, ", ", max_len - strlen(out) - 1);
                strncat(out, ba->data[i] ? "true" : "false", max_len - strlen(out) - 1);
            }
        }
        strncat(out, "]", max_len - strlen(out) - 1);
    } else if (v.type == VAL_OBJECT) {
        struct objInstance *inst = (struct objInstance*)v.ptr;
        if (!inst) {
            snprintf(out, max_len, "null");
        } else {
            snprintf(out, max_len, "<Object %s#%d>", (inst->cls && inst->cls->name[0]) ? inst->cls->name : "Unknown", inst->id);
        }
    } else {
        snprintf(out, max_len, "null");
    }
}

struct callFrame {
    struct iV *IV; struct fV *FV; struct cV *CV; struct bV *BV; struct sV *SV;
    struct iA *IA; struct fA *FA; struct cA *CA; struct bA *BA;
    struct lV *LV; struct mV *MV;
    struct oV *OV;
    struct objInstance *thisObj;
    struct classDef *executingClass;
    long returnPos; 
    int hasReturn; 
    struct Value returnValue;
    int saved_p;
    int saved_k;
    int saved_envTop;
    int savedVarCount;
    int assignType;
    char assignTarget[100];
};

extern struct callFrame callStack[10000];
extern int callStackTop;
extern struct callFrame envStack[10000];
extern int envTop;
extern struct funcDef *FN;

void initEnvironmentFrame(struct callFrame *frame) {
    memset(frame, 0, sizeof(*frame));
}

void cleanupFrame(struct callFrame *frame) {
    struct iV *iv = frame->IV; while(iv) { struct iV *n = iv->next; free(iv); iv = n; } frame->IV = NULL;
    struct fV *fv = frame->FV; while(fv) { struct fV *n = fv->next; free(fv); fv = n; } frame->FV = NULL;
    struct cV *cv = frame->CV; while(cv) { struct cV *n = cv->next; free(cv); cv = n; } frame->CV = NULL;
    struct bV *bv = frame->BV; while(bv) { struct bV *n = bv->next; free(bv); bv = n; } frame->BV = NULL;
    struct sV *sv = frame->SV; while(sv) { struct sV *n = sv->next; free(sv); sv = n; } frame->SV = NULL;
    struct iA *ia = frame->IA; while(ia) { struct iA *n = ia->next; free(ia->data); free(ia); ia = n; } frame->IA = NULL;
    struct fA *fa = frame->FA; while(fa) { struct fA *n = fa->next; free(fa->data); free(fa); fa = n; } frame->FA = NULL;
    struct cA *ca = frame->CA; while(ca) { struct cA *n = ca->next; free(ca->data); free(ca); ca = n; } frame->CA = NULL;
    struct bA *ba = frame->BA; while(ba) { struct bA *n = ba->next; free(ba->data); free(ba); ba = n; } frame->BA = NULL;
    struct lV *lv = frame->LV; while(lv) { struct lV *n = lv->next; if (!lv->ref) free(lv->data); free(lv); lv = n; } frame->LV = NULL;
    struct mV *mv = frame->MV; while(mv) { struct mV *n = mv->next; if (mv->entries) free(mv->entries); free(mv); mv = n; } frame->MV = NULL;
    struct oV *ov = frame->OV; while(ov) { struct oV *n = ov->next; free(ov); ov = n; } frame->OV = NULL;
    frame->thisObj = NULL;
    frame->executingClass = NULL;
}

void appendI(struct iV **q, char *name, int n) {
    struct iV *temp = *q;
    while (temp) { if (!strcmp(temp->name, name)) { temp->data = n; return; } temp = temp->next; }
    struct iV *nnode = (struct iV*)malloc(sizeof(struct iV));
    strcpy(nnode->name, name); nnode->data = n; nnode->next = *q; *q = nnode;
}
void appendF(struct fV **q, char *name, float n) {
    struct fV *temp = *q;
    while (temp) { if (!strcmp(temp->name, name)) { temp->data = n; return; } temp = temp->next; }
    struct fV *nnode = (struct fV*)malloc(sizeof(struct fV));
    strcpy(nnode->name, name); nnode->data = n; nnode->next = *q; *q = nnode;
}
void appendC(struct cV **q, char *name, char n) {
    struct cV *temp = *q;
    while (temp) { if (!strcmp(temp->name, name)) { temp->data = n; return; } temp = temp->next; }
    struct cV *nnode = (struct cV*)malloc(sizeof(struct cV));
    strcpy(nnode->name, name); nnode->data = n; nnode->next = *q; *q = nnode;
}
void appendB(struct bV **q, char *name, int n) {
    struct bV *temp = *q;
    while (temp) { if (!strcmp(temp->name, name)) { temp->data = n; return; } temp = temp->next; }
    struct bV *nnode = (struct bV*)malloc(sizeof(struct bV));
    strcpy(nnode->name, name); nnode->data = n; nnode->next = *q; *q = nnode;
}
void appendS(struct sV **q, char *name, char *n) {
    struct sV *temp = *q;
    while (temp) { if (!strcmp(temp->name, name)) { strcpy(temp->data, n); return; } temp = temp->next; }
    struct sV *nnode = (struct sV*)malloc(sizeof(struct sV));
    strcpy(nnode->name, name); strcpy(nnode->data, n); nnode->next = *q; *q = nnode;
}
void appendIA(struct iA **q, char *name, int size) {
    struct iA *nnode = (struct iA*)malloc(sizeof(struct iA));
    strcpy(nnode->name, name); nnode->size = size; nnode->data = (int*)calloc(size, sizeof(int)); nnode->next = *q; *q = nnode;
}

int setI_scope(char *name, int val) {
    for (int idx = envTop; idx >= 0; idx--) {
        struct iV *iv = envStack[idx].IV;
        while (iv) { if (!strcmp(iv->name, name)) { iv->data = val; return 1; } iv = iv->next; }
    }
    return 0;
}
int setF_scope(char *name, float val) {
    for (int idx = envTop; idx >= 0; idx--) {
        struct fV *fv = envStack[idx].FV;
        while (fv) { if (!strcmp(fv->name, name)) { fv->data = val; return 1; } fv = fv->next; }
    }
    return 0;
}
int setC_scope(char *name, char val) {
    for (int idx = envTop; idx >= 0; idx--) {
        struct cV *cv = envStack[idx].CV;
        while (cv) { if (!strcmp(cv->name, name)) { cv->data = val; return 1; } cv = cv->next; }
    }
    return 0;
}
int setB_scope(char *name, int val) {
    for (int idx = envTop; idx >= 0; idx--) {
        struct bV *bv = envStack[idx].BV;
        while (bv) { if (!strcmp(bv->name, name)) { bv->data = val; return 1; } bv = bv->next; }
    }
    return 0;
}
int setS_scope(char *name, char *val) {
    for (int idx = envTop; idx >= 0; idx--) {
        struct sV *sv = envStack[idx].SV;
        while (sv) { if (!strcmp(sv->name, name)) { strcpy(sv->data, val); return 1; } sv = sv->next; }
    }
    return 0;
}

static inline void appendO(struct oV **q, const char *name, const char *className, struct objInstance *inst) {
    struct oV *temp = *q;
    while (temp) {
        if (!strcmp(temp->name, name)) {
            temp->instance = inst;
            if (className && className[0]) {
                snprintf(temp->className, sizeof(temp->className), "%s", className);
            }
            return;
        }
        temp = temp->next;
    }
    struct oV *nnode = (struct oV*)malloc(sizeof(struct oV));
    snprintf(nnode->name, sizeof(nnode->name), "%s", name);
    if (className && className[0]) {
        snprintf(nnode->className, sizeof(nnode->className), "%s", className);
    } else {
        nnode->className[0] = '\0';
    }
    nnode->instance = inst;
    nnode->next = *q;
    *q = nnode;
}

static inline int setO_scope(const char *name, struct objInstance *inst) {
    for (int idx = envTop; idx >= 0; idx--) {
        struct oV *ov = envStack[idx].OV;
        while (ov) {
            if (!strcmp(ov->name, name)) {
                ov->instance = inst;
                return 1;
            }
            ov = ov->next;
        }
    }
    return 0;
}

static inline struct oV* findObj_scope(const char *name) {
    for (int idx = envTop; idx >= 0; idx--) {
        struct oV *ov = envStack[idx].OV;
        while (ov) {
            if (!strcmp(ov->name, name)) return ov;
            ov = ov->next;
        }
    }
    return NULL;
}

static inline struct objInstance* allocateObject(struct classDef *cls) {
    if (!cls) return NULL;
    struct objInstance *inst = (struct objInstance*)malloc(sizeof(struct objInstance));
    if (!inst) return NULL;
    inst->id = NEXT_OBJECT_ID++;
    inst->cls = cls;
    inst->fieldCount = cls->fieldCount;
    if (cls->fieldCount > 0) {
        inst->fields = (struct objFieldVal*)calloc(cls->fieldCount, sizeof(struct objFieldVal));
        if (!inst->fields) {
            free(inst);
            return NULL;
        }
        for (int i = 0; i < cls->fieldCount; i++) {
            snprintf(inst->fields[i].name, sizeof(inst->fields[i].name), "%s", cls->fields[i].name);
            inst->fields[i].type = cls->fields[i].type;
            inst->fields[i].isPrivate = cls->fields[i].isPrivate;
            inst->fields[i].declaringClass = cls->fields[i].declaringClass ? cls->fields[i].declaringClass : cls;
            if (cls->fields[i].type == 5) {
                inst->fields[i].val.type = VAL_STRING;
                inst->fields[i].val.s[0] = '\0';
                inst->fields[i].val.f = 0.0f;
                inst->fields[i].val.ptr = NULL;
            } else if (cls->fields[i].type == 10) {
                inst->fields[i].val.type = VAL_LIST;
                inst->fields[i].val.ptr = NULL;
                inst->fields[i].val.s[0] = '\0';
                inst->fields[i].val.f = 0.0f;
            } else if (cls->fields[i].type == 11) {
                inst->fields[i].val.type = VAL_MAP;
                inst->fields[i].val.ptr = NULL;
                inst->fields[i].val.s[0] = '\0';
                inst->fields[i].val.f = 0.0f;
            } else if (cls->fields[i].type == 12) {
                inst->fields[i].val.type = VAL_OBJECT;
                inst->fields[i].val.ptr = NULL;
                inst->fields[i].val.s[0] = '\0';
                inst->fields[i].val.f = 0.0f;
            } else if (cls->fields[i].type >= 6 && cls->fields[i].type <= 9) {
                inst->fields[i].val.type = (enum ValueType)(cls->fields[i].type - 4);
                inst->fields[i].val.ptr = NULL;
                inst->fields[i].val.s[0] = '\0';
                inst->fields[i].val.f = 0.0f;
            } else {
                inst->fields[i].val.type = VAL_FLOAT;
                inst->fields[i].val.f = 0.0f;
                inst->fields[i].val.s[0] = '\0';
                inst->fields[i].val.ptr = NULL;
            }
        }
    } else {
        inst->fields = NULL;
    }
    inst->next = OBJECT_HEAP;
    OBJECT_HEAP = inst;
    return inst;
}

static inline void freeAllObjects(struct objInstance **heap) {
    if (!heap) return;
    struct objInstance *cur = *heap;
    while (cur) {
        struct objInstance *nxt = cur->next;
        if (cur->fields) {
            free(cur->fields);
            cur->fields = NULL;
        }
        free(cur);
        cur = nxt;
    }
    *heap = NULL;
}

static inline struct Value getFieldValue(struct objInstance *inst, const char *fieldName, int *found) {
    struct Value v;
    memset(&v, 0, sizeof(v));
    v.type = VAL_ERROR;
    *found = 0;
    if (!inst) return v;
    for (int f = 0; f < inst->fieldCount; f++) {
        if (!strcmp(inst->fields[f].name, fieldName)) {
            if (inst->fields[f].isPrivate) {
                struct classDef *callerContext = (callStackTop >= 0) ? callStack[callStackTop].executingClass : NULL;
                struct classDef *declaringClass = inst->fields[f].declaringClass ? inst->fields[f].declaringClass : inst->cls;
                if (callerContext != declaringClass) {
                    printf("Error: Cannot access private member '%s' of class '%s' from context '%s'\n",
                           fieldName,
                           declaringClass ? declaringClass->name : "<unknown>",
                           callerContext ? callerContext->name : "<global>");
                    *found = -1;
                    return v;
                }
            }
            *found = 1;
            return inst->fields[f].val;
        }
    }
    return v;
}

static inline int assignFieldValue(struct objInstance *inst, const char *fieldName, struct Value rhsVal) {
    if (!inst) {
        printf("Error: Null reference: cannot assign to field '%s' on null receiver\n", fieldName);
        return 0;
    }
    int fieldIdx = -1;
    for (int f = 0; f < inst->fieldCount; f++) {
        if (!strcmp(inst->fields[f].name, fieldName)) {
            fieldIdx = f;
            break;
        }
    }
    if (fieldIdx == -1) {
        printf("Error: Field '%s' not found in class '%s'\n", fieldName, inst->cls ? inst->cls->name : "<unknown>");
        return 0;
    }
    
    struct objFieldVal *field = &inst->fields[fieldIdx];
    
    if (field->isPrivate) {
        struct classDef *callerContext = (callStackTop >= 0) ? callStack[callStackTop].executingClass : NULL;
        struct classDef *declaringClass = field->declaringClass ? field->declaringClass : inst->cls;
        if (callerContext != declaringClass) {
            printf("Error: Cannot access private member '%s' of class '%s' from context '%s'\n",
                   fieldName,
                   declaringClass ? declaringClass->name : "<unknown>",
                   callerContext ? callerContext->name : "<global>");
            return 0;
        }
    }
    
    if (field->type == 12) {
        if (rhsVal.type == VAL_OBJECT) {
            field->val.type = VAL_OBJECT;
            field->val.ptr = rhsVal.ptr;
            field->val.f = (rhsVal.ptr != NULL) ? 1.0f : 0.0f;
            return 1;
        } else {
            printf("Error: Cannot assign non-object value to object field '%s' in class '%s'\n", fieldName, inst->cls ? inst->cls->name : "");
            return 0;
        }
    } else if (field->type == 5) {
        field->val.type = VAL_STRING;
        if (rhsVal.type == VAL_STRING) {
            snprintf(field->val.s, sizeof(field->val.s), "%s", rhsVal.s);
        } else {
            value_to_string(rhsVal, field->val.s, sizeof(field->val.s));
        }
        return 1;
    } else if (field->type >= 1 && field->type <= 4) {
        if (rhsVal.type == VAL_STRING || rhsVal.type == VAL_OBJECT) {
            printf("Error: Type mismatch: cannot assign non-numeric value to numeric field '%s' in class '%s'\n", fieldName, inst->cls ? inst->cls->name : "");
            return 0;
        }
        field->val.type = VAL_FLOAT;
        if (field->type == 1) {
            field->val.f = (float)(int)rhsVal.f;
        } else if (field->type == 2) {
            field->val.f = rhsVal.f;
        } else if (field->type == 3) {
            field->val.f = (float)(char)rhsVal.f;
        } else if (field->type == 4) {
            field->val.f = (rhsVal.f != 0.0f) ? 1.0f : 0.0f;
        }
        return 1;
    } else if (field->type == 10) {
        if (rhsVal.type == VAL_LIST) {
            field->val = rhsVal;
            return 1;
        } else {
            printf("Error: Type mismatch: cannot assign non-list value to list field '%s' in class '%s'\n", fieldName, inst->cls ? inst->cls->name : "");
            return 0;
        }
    } else if (field->type == 11) {
        if (rhsVal.type == VAL_MAP) {
            field->val = rhsVal;
            return 1;
        } else {
            printf("Error: Type mismatch: cannot assign non-map value to map field '%s' in class '%s'\n", fieldName, inst->cls ? inst->cls->name : "");
            return 0;
        }
    } else if (field->type >= 6 && field->type <= 9) {
        field->val = rhsVal;
        return 1;
    }
    
    field->val = rhsVal;
    return 1;
}

int typeFetcher(char *name) {
    if (!strcmp(name, "this")) return 12;
    for (int idx=envTop; idx>=0; idx--) {
        int i = idx;
        struct iV *iv = envStack[i].IV; while (iv) { if (!strcmp(iv->name, name)) return 1; iv = iv->next; }
        struct fV *fv = envStack[i].FV; 
        while (fv) { 
            if (!strcmp(fv->name, name)) return 2; 
            fv = fv->next; 
        }
        struct cV *cv = envStack[i].CV; while (cv) { if (!strcmp(cv->name, name)) return 3; cv = cv->next; }
        struct bV *bv = envStack[i].BV; while (bv) { if (!strcmp(bv->name, name)) return 4; bv = bv->next; }
        struct sV *sv = envStack[i].SV; while (sv) { if (!strcmp(sv->name, name)) return 5; sv = sv->next; }
        struct iA *ia = envStack[i].IA; while (ia) { if (!strcmp(ia->name, name)) return 6; ia = ia->next; }
        struct fA *fa = envStack[i].FA; while (fa) { if (!strcmp(fa->name, name)) return 7; fa = fa->next; }
        struct cA *ca = envStack[i].CA; while (ca) { if (!strcmp(ca->name, name)) return 8; ca = ca->next; }
        struct bA *ba = envStack[i].BA; while (ba) { if (!strcmp(ba->name, name)) return 9; ba = ba->next; }
        struct lV *lv = envStack[i].LV; while (lv) { if (!strcmp(lv->name, name)) return 10; lv = lv->next; }
        struct mV *mv = envStack[i].MV; while (mv) { if (!strcmp(mv->name, name)) return 11; mv = mv->next; }
        struct oV *ov = envStack[i].OV; while (ov) { if (!strcmp(ov->name, name)) return 12; ov = ov->next; }
    }
    return 0;
}

struct Value valFetcherGlobal(char *name) {
    struct Value v;
    v.type = VAL_ERROR;
    v.f = 0.0;
    v.ptr = NULL;

    if (!strcmp(name, "this")) {
        if (callStackTop >= 0 && callStack[callStackTop].thisObj != NULL) {
            v.type = VAL_OBJECT;
            v.ptr = (void*)callStack[callStackTop].thisObj;
            v.f = 1.0f;
            return v;
        } else {
            printf("Error: 'this' used outside of an instance method\n");
            v.type = VAL_ERROR;
            return v;
        }
    }

    for (int idx=envTop; idx>=0; idx--) {
        int i = idx;
        struct iV *iv = envStack[i].IV; while (iv) { if (!strcmp(iv->name, name)) { v.type=VAL_FLOAT; v.f=(float)iv->data; return v; } iv = iv->next; }
        struct fV *fv = envStack[i].FV; while (fv) { if (!strcmp(fv->name, name)) { v.type=VAL_FLOAT; v.f=fv->data; return v; } fv = fv->next; }
        struct cV *cv = envStack[i].CV; while (cv) { if (!strcmp(cv->name, name)) { v.type=VAL_FLOAT; v.f=(float)cv->data; return v; } cv = cv->next; }
        struct bV *bv = envStack[i].BV; while (bv) { if (!strcmp(bv->name, name)) { v.type=VAL_FLOAT; v.f=(float)bv->data; return v; } bv = bv->next; }
        struct sV *sv = envStack[i].SV; while (sv) { if (!strcmp(sv->name, name)) { v.type=VAL_STRING; strcpy(v.s, sv->data); return v; } sv = sv->next; }
        struct iA *ia = envStack[i].IA; while (ia) { if (!strcmp(ia->name, name)) { v.type=VAL_INT_ARRAY; v.ptr=(void*)ia; return v; } ia = ia->next; }
        struct fA *fa = envStack[i].FA; while (fa) { if (!strcmp(fa->name, name)) { v.type=VAL_FLOAT_ARRAY; v.ptr=(void*)fa; return v; } fa = fa->next; }
        struct cA *ca = envStack[i].CA; while (ca) { if (!strcmp(ca->name, name)) { v.type=VAL_CHAR_ARRAY; v.ptr=(void*)ca; return v; } ca = ca->next; }
        struct bA *ba = envStack[i].BA; while (ba) { if (!strcmp(ba->name, name)) { v.type=VAL_BOOL_ARRAY; v.ptr=(void*)ba; return v; } ba = ba->next; }
        struct lV *lv = envStack[i].LV; while (lv) { if (!strcmp(lv->name, name)) { v.type=VAL_LIST; v.ptr=(void*)lv; return v; } lv = lv->next; }
        struct mV *mv = envStack[i].MV; while (mv) { if (!strcmp(mv->name, name)) { v.type=VAL_MAP; v.ptr=(void*)mv; return v; } mv = mv->next; }
        struct oV *ov = envStack[i].OV; while (ov) {
            if (!strcmp(ov->name, name)) {
                v.type = VAL_OBJECT;
                v.ptr = (void*)ov->instance;
                v.f = (ov->instance != NULL) ? 1.0f : 0.0f;
                return v;
            }
            ov = ov->next;
        }
    }
    
    return v;
}


int getIA(struct iA *IA, char *name, int idx) {
    for (int scope = envTop; scope >= 0; scope--) {
        struct iA *ia = envStack[scope].IA;
        while (ia) {
            if (!strcmp(name, ia->name)) {
                if (idx >= 0 && idx < ia->size) return ia->data[idx];
                return 0;
            }
            ia = ia->next;
        }
    }
    return 0;
}
float getFA(struct fA *FA, char *name, int idx) {
    for (int scope = envTop; scope >= 0; scope--) {
        struct fA *fa = envStack[scope].FA;
        while (fa) {
            if (!strcmp(name, fa->name)) {
                if (idx >= 0 && idx < fa->size) return fa->data[idx];
                return 0.0f;
            }
            fa = fa->next;
        }
    }
    return 0.0f;
}
char getCA(struct cA *CA, char *name, int idx) {
    for (int scope = envTop; scope >= 0; scope--) {
        struct cA *ca = envStack[scope].CA;
        while (ca) {
            if (!strcmp(name, ca->name)) {
                if (idx >= 0 && idx < ca->size) return ca->data[idx];
                return '\0';
            }
            ca = ca->next;
        }
    }
    return '\0';
}
int getBA(struct bA *BA, char *name, int idx) {
    for (int scope = envTop; scope >= 0; scope--) {
        struct bA *ba = envStack[scope].BA;
        while (ba) {
            if (!strcmp(name, ba->name)) {
                if (idx >= 0 && idx < ba->size) return ba->data[idx];
                return 0;
            }
            ba = ba->next;
        }
    }
    return 0;
}
struct Value getL(struct lV *LV, char *name, int idx) {
    struct Value result; memset(&result, 0, sizeof(result)); result.type = VAL_ERROR;
    while (LV) { if (!strcmp(name, LV->name)) { struct lV *target = LV; while (target->ref) target = target->ref; if (idx >= 0 && idx < target->size) return target->data[idx]; } LV = LV->next; } return result;
}

void setIA(struct iA *IA, char *name, int idx, int val) {
    for (int scope = envTop; scope >= 0; scope--) {
        struct iA *ia = envStack[scope].IA;
        while (ia) {
            if (!strcmp(name, ia->name)) {
                if (idx >= 0 && idx < ia->size) ia->data[idx] = val;
                return;
            }
            ia = ia->next;
        }
    }
}
void setFA(struct fA *FA, char *name, int idx, float val) {
    for (int scope = envTop; scope >= 0; scope--) {
        struct fA *fa = envStack[scope].FA;
        while (fa) {
            if (!strcmp(name, fa->name)) {
                if (idx >= 0 && idx < fa->size) fa->data[idx] = val;
                return;
            }
            fa = fa->next;
        }
    }
}
void setCA(struct cA *CA, char *name, int idx, char val) {
    for (int scope = envTop; scope >= 0; scope--) {
        struct cA *ca = envStack[scope].CA;
        while (ca) {
            if (!strcmp(name, ca->name)) {
                if (idx >= 0 && idx < ca->size) ca->data[idx] = val;
                return;
            }
            ca = ca->next;
        }
    }
}
void setBA(struct bA *BA, char *name, int idx, int val) {
    for (int scope = envTop; scope >= 0; scope--) {
        struct bA *ba = envStack[scope].BA;
        while (ba) {
            if (!strcmp(name, ba->name)) {
                if (idx >= 0 && idx < ba->size) ba->data[idx] = val;
                return;
            }
            ba = ba->next;
        }
    }
}

void appendFA(struct fA **q, char *name, int size) {
    struct fA *nnode = (struct fA*)malloc(sizeof(struct fA));
    strcpy(nnode->name, name); nnode->size = size; nnode->data = (float*)calloc(size, sizeof(float)); nnode->next = *q; *q = nnode;
}
void appendCA(struct cA **q, char *name, int size) {
    struct cA *nnode = (struct cA*)malloc(sizeof(struct cA));
    strcpy(nnode->name, name); nnode->size = size; nnode->data = (char*)calloc(size, sizeof(char)); nnode->next = *q; *q = nnode;
}
void appendBA(struct bA **q, char *name, int size) {
    struct bA *nnode = (struct bA*)malloc(sizeof(struct bA));
    strcpy(nnode->name, name); nnode->size = size; nnode->data = (int*)calloc(size, sizeof(int)); nnode->next = *q; *q = nnode;
}

void appendIA_copy(struct iA **q, char *name, struct iA *ref) {
    struct iA *nnode = (struct iA*)malloc(sizeof(struct iA));
    strcpy(nnode->name, name); nnode->size = ref->size; nnode->data = (int*)malloc(ref->size * sizeof(int));
    memcpy(nnode->data, ref->data, ref->size * sizeof(int));
    nnode->next = *q; *q = nnode;
}
void appendFA_copy(struct fA **q, char *name, struct fA *ref) {
    struct fA *nnode = (struct fA*)malloc(sizeof(struct fA));
    strcpy(nnode->name, name); nnode->size = ref->size; nnode->data = (float*)malloc(ref->size * sizeof(float));
    memcpy(nnode->data, ref->data, ref->size * sizeof(float));
    nnode->next = *q; *q = nnode;
}
void appendCA_copy(struct cA **q, char *name, struct cA *ref) {
    struct cA *nnode = (struct cA*)malloc(sizeof(struct cA));
    strcpy(nnode->name, name); nnode->size = ref->size; nnode->data = (char*)malloc(ref->size * sizeof(char));
    memcpy(nnode->data, ref->data, ref->size * sizeof(char));
    nnode->next = *q; *q = nnode;
}
void appendBA_copy(struct bA **q, char *name, struct bA *ref) {
    struct bA *nnode = (struct bA*)malloc(sizeof(struct bA));
    strcpy(nnode->name, name); nnode->size = ref->size; nnode->data = (int*)malloc(ref->size * sizeof(int));
    memcpy(nnode->data, ref->data, ref->size * sizeof(int));
    nnode->next = *q; *q = nnode;
}

void appendL(struct lV **q, char *name, int capacity) {
    struct lV *nnode = (struct lV *)malloc(sizeof(struct lV));
    strcpy(nnode->name, name); nnode->size = 0; nnode->capacity = capacity;
    nnode->data = (struct Value *)malloc(sizeof(struct Value) * capacity); nnode->ref = NULL;
    nnode->next = *q; *q = nnode;
}
void appendL_ref(struct lV **q, char *name, struct lV *ref) {
    struct lV *nnode = (struct lV*)malloc(sizeof(struct lV));
    while (ref->ref) ref = ref->ref;
    strcpy(nnode->name, name); nnode->size = 0; nnode->capacity = 0;
    nnode->data = NULL; nnode->ref = ref;
    nnode->next = *q; *q = nnode;
}

void appendM(struct mV **q, char *name) {
    struct mV *nnode = (struct mV *)malloc(sizeof(struct mV));
    strcpy(nnode->name, name);
    nnode->count = 0;
    nnode->capacity = 4;
    nnode->entries = (struct mapEntry*)malloc(sizeof(struct mapEntry) * nnode->capacity);
    nnode->next = *q;
    *q = nnode;
}

void appendM_copy(struct mV **q, char *name, struct mV *ref) {
    struct mV *nnode = (struct mV*)malloc(sizeof(struct mV));
    strcpy(nnode->name, name);
    nnode->count = ref ? ref->count : 0;
    nnode->capacity = (ref && ref->capacity > 0) ? ref->capacity : 4;
    if (nnode->capacity < nnode->count) nnode->capacity = nnode->count;
    nnode->entries = (struct mapEntry*)malloc(sizeof(struct mapEntry) * nnode->capacity);
    if (ref && ref->entries) {
        for (int i = 0; i < ref->count; i++) {
            strcpy(nnode->entries[i].key, ref->entries[i].key);
            nnode->entries[i].value = ref->entries[i].value;
        }
    }
    nnode->next = *q;
    *q = nnode;
}

void setM_Val(struct mV *MV, char *name, char *key, struct Value val) {
    for (int idx = envTop; idx >= 0; idx--) {
        struct mV *mv = envStack[idx].MV;
        while (mv) {
            if (!strcmp(mv->name, name)) {
                for (int i = 0; i < mv->count; i++) {
                    if (!strcmp(mv->entries[i].key, key)) {
                        mv->entries[i].value = val;
                        return;
                    }
                }
                if (mv->count >= mv->capacity) {
                    int newCap = (mv->capacity == 0) ? 4 : mv->capacity * 2;
                    mv->entries = (struct mapEntry*)realloc(mv->entries, sizeof(struct mapEntry) * newCap);
                    mv->capacity = newCap;
                }
                strcpy(mv->entries[mv->count].key, key);
                mv->entries[mv->count].value = val;
                mv->count++;
                return;
            }
            mv = mv->next;
        }
    }
}

void setM_F(struct mV *MV, char *name, char *key, float val) {
    struct Value v;
    memset(&v, 0, sizeof(v));
    v.type = VAL_FLOAT;
    v.f = val;
    setM_Val(MV, name, key, v);
}

struct Value getM_Val(struct mV *MV, char *name, char *key) {
    struct Value v;
    memset(&v, 0, sizeof(v));
    v.type = VAL_FLOAT;
    v.f = 0.0;
    for (int idx = envTop; idx >= 0; idx--) {
        struct mV *mv = envStack[idx].MV;
        while (mv) {
            if (!strcmp(mv->name, name)) {
                for (int i = 0; i < mv->count; i++) {
                    if (!strcmp(mv->entries[i].key, key)) {
                        return mv->entries[i].value;
                    }
                }
                return v;
            }
            mv = mv->next;
        }
    }
    return v;
}

float getM_F(struct mV *MV, char *name, char *key) {
    struct Value v = getM_Val(MV, name, key);
    if (v.type == VAL_STRING) {
        return (float)atof(v.s);
    }
    return v.f;
}

char* getM_S(struct mV *MV, char *name, char *key) {
    for (int idx = envTop; idx >= 0; idx--) {
        struct mV *mv = envStack[idx].MV;
        while (mv) {
            if (!strcmp(mv->name, name)) {
                for (int i = 0; i < mv->count; i++) {
                    if (!strcmp(mv->entries[i].key, key)) {
                        if (mv->entries[i].value.type == VAL_STRING) return mv->entries[i].value.s;
                        return "";
                    }
                }
                return "";
            }
            mv = mv->next;
        }
    }
    return "";
}

int appendFunc(struct funcDef **head, char *name, int paramCount, struct funcParam params[], long bodyPos, char *filepath) {
    struct funcDef *temp = *head;
    while (temp) { if (!strcmp(temp->name, name)) { return 1; } temp = temp->next; }
    struct funcDef *nnode = (struct funcDef*)malloc(sizeof(struct funcDef));
    strcpy(nnode->name, name);
    nnode->paramCount = paramCount;
    for (int i = 0; i < paramCount; i++) { nnode->params[i] = params[i]; }
    nnode->bodyPos = bodyPos;
    if (filepath) strcpy(nnode->filepath, filepath);
    else strcpy(nnode->filepath, "");
    nnode->next = *head; *head = nnode;
    return 0;
}
struct funcDef* findFunc(struct funcDef *list, char *name) {
    while(list) { if(!strcmp(list->name, name)) return list; list = list->next; }
    return NULL;
}

struct classDef* findClass(struct classDef *list, const char *name) {
    while (list) {
        if (!strcmp(list->name, name)) return list;
        list = list->next;
    }
    return NULL;
}

static inline struct classMethod* findClassMethod(struct classDef *cls, const char *name) {
    struct classDef *curr = cls;
    while (curr) {
        for (int i = 0; i < curr->methodCount; i++) {
            if (!strcmp(curr->methods[i].name, name)) {
                return &curr->methods[i];
            }
        }
        curr = curr->parent;
    }
    return NULL;
}

static inline struct classMethod* findConstructor(struct classDef *cls) {
    if (!cls) return NULL;
    for (int i = 0; i < cls->methodCount; i++) {
        if (!strcmp(cls->methods[i].name, "construct")) {
            return &cls->methods[i];
        }
    }
    return NULL;
}

static inline int isSubclassOf(struct classDef *child, struct classDef *parent) {
    if (!child || !parent) return 0;
    struct classDef *curr = child;
    while (curr) {
        if (curr == parent || !strcmp(curr->name, parent->name)) {
            return 1;
        }
        curr = curr->parent;
    }
    return 0;
}

void freeClassRegistry(struct classDef **head) {
    struct classDef *cur = *head;
    while (cur) {
        struct classDef *next = cur->next;
        for (int m = 0; m < cur->methodCount; m++) {
            if (cur->methods[m].func) {
                free(cur->methods[m].func);
                cur->methods[m].func = NULL;
            }
        }
        free(cur);
        cur = next;
    }
    *head = NULL;
}


void scopeSetSV(char *name, char *val) {
    appendS(&envStack[envTop].SV, name, val);
}

char* scopeGetSV(char *name) {
    for (int i = envTop; i >= 0; i--) {
        struct sV *temp = envStack[i].SV;
        while (temp != NULL) {
            if (!strcmp(temp->name, name)) return temp->data;
            temp = temp->next;
        }
    }
    return NULL;
}

void setL(struct lV *LV, char *name, int idx, struct Value val) {
    for (int scope = envTop; scope >= 0; scope--) {
        LV = envStack[scope].LV;
        while (LV) {
            if (!strcmp(LV->name, name)) {
                struct lV *target = LV;
                while (target->ref) target = target->ref;
                if (idx >= 0 && idx < target->size) target->data[idx] = val;
                return;
            }
            LV = LV->next;
        }
    }
}
void scopePushL(char *name, struct Value val) {
    for (int idx = envTop; idx >= 0; idx--) {
        struct lV *lv = envStack[idx].LV;
        while (lv) {
            if (!strcmp(lv->name, name)) {
                struct lV *target = lv;
                while (target->ref) target = target->ref;
                if (target->size >= target->capacity) {
                    target->capacity = (target->capacity == 0) ? 4 : target->capacity * 2;
                    target->data = (struct Value*)realloc(target->data, target->capacity * sizeof(struct Value));
                }
                target->data[target->size++] = val;
                return;
            }
            lv = lv->next;
        }
    }
}
struct Value scopePopL(char *name) {
    struct Value result; memset(&result, 0, sizeof(result)); result.type = VAL_ERROR;
    for (int idx = envTop; idx >= 0; idx--) {
        struct lV *lv = envStack[idx].LV;
        while (lv) {
            if (!strcmp(lv->name, name)) {
                struct lV *target = lv;
                while (target->ref) target = target->ref;
                if (target->size > 0) return target->data[--target->size];
                return result;
            }
            lv = lv->next;
        }
    }
    return result;
}
float scopeGetLen(char *name) {
    for (int i = envTop; i >= 0; i--) {
        struct lV *lv = envStack[i].LV; while (lv) { if (!strcmp(lv->name, name)) { struct lV *target = lv; while (target->ref) target = target->ref; return (float)target->size; } lv = lv->next; }
        struct mV *mv = envStack[i].MV; while (mv) { if (!strcmp(mv->name, name)) return (float)mv->count; mv = mv->next; }
        struct sV *sv = envStack[i].SV; while (sv) { if (!strcmp(sv->name, name)) return (float)strlen(sv->data); sv = sv->next; }
        struct iA *ia = envStack[i].IA; while (ia) { if (!strcmp(ia->name, name)) return (float)ia->size; ia = ia->next; }
        struct fA *fa = envStack[i].FA; while (fa) { if (!strcmp(fa->name, name)) return (float)fa->size; fa = fa->next; }
        struct cA *ca = envStack[i].CA; while (ca) { if (!strcmp(ca->name, name)) return (float)ca->size; ca = ca->next; }
        struct bA *ba = envStack[i].BA; while (ba) { if (!strcmp(ba->name, name)) return (float)ba->size; ba = ba->next; }
    }
    return 0.0;
}
float scopeMapExists(char *name, char *key) {
    for (int idx = envTop; idx >= 0; idx--) {
        struct mV *mv = envStack[idx].MV;
        while (mv) {
            if (!strcmp(mv->name, name)) {
                for (int i=0; i<mv->count; i++) {
                    if (!strcmp(mv->entries[i].key, key)) return 1.0;
                }
                return 0.0;
            }
            mv = mv->next;
        }
    }
    return 0.0;
}
void scopeMapDelete(char *name, char *key) {
    for (int idx = envTop; idx >= 0; idx--) {
        struct mV *mv = envStack[idx].MV;
        while (mv) {
            if (!strcmp(mv->name, name)) {
                for (int i=0; i<mv->count; i++) {
                    if (!strcmp(mv->entries[i].key, key)) {
                        for (int j=i; j<mv->count-1; j++) {
                            mv->entries[j] = mv->entries[j+1];
                        }
                        mv->count--;
                        return;
                    }
                }
                return;
            }
            mv = mv->next;
        }
    }
}

int scopeValueExists(char *name) {
    return typeFetcher(name) != 0;
}

void scopeDeleteValue(char *name) {
    for (int scope = envTop; scope >= 0; scope--) {
        struct iV **iv = &envStack[scope].IV; while (*iv) { if (!strcmp((*iv)->name, name)) { struct iV *node = *iv; *iv = node->next; free(node); return; } iv = &(*iv)->next; }
        struct fV **fv = &envStack[scope].FV; while (*fv) { if (!strcmp((*fv)->name, name)) { struct fV *node = *fv; *fv = node->next; free(node); return; } fv = &(*fv)->next; }
        struct cV **cv = &envStack[scope].CV; while (*cv) { if (!strcmp((*cv)->name, name)) { struct cV *node = *cv; *cv = node->next; free(node); return; } cv = &(*cv)->next; }
        struct bV **bv = &envStack[scope].BV; while (*bv) { if (!strcmp((*bv)->name, name)) { struct bV *node = *bv; *bv = node->next; free(node); return; } bv = &(*bv)->next; }
        struct sV **sv = &envStack[scope].SV; while (*sv) { if (!strcmp((*sv)->name, name)) { struct sV *node = *sv; *sv = node->next; free(node); return; } sv = &(*sv)->next; }
        struct iA **ia = &envStack[scope].IA; while (*ia) { if (!strcmp((*ia)->name, name)) { struct iA *node = *ia; *ia = node->next; free(node->data); free(node); return; } ia = &(*ia)->next; }
        struct fA **fa = &envStack[scope].FA; while (*fa) { if (!strcmp((*fa)->name, name)) { struct fA *node = *fa; *fa = node->next; free(node->data); free(node); return; } fa = &(*fa)->next; }
        struct cA **ca = &envStack[scope].CA; while (*ca) { if (!strcmp((*ca)->name, name)) { struct cA *node = *ca; *ca = node->next; free(node->data); free(node); return; } ca = &(*ca)->next; }
        struct bA **ba = &envStack[scope].BA; while (*ba) { if (!strcmp((*ba)->name, name)) { struct bA *node = *ba; *ba = node->next; free(node->data); free(node); return; } ba = &(*ba)->next; }
        struct lV **lv = &envStack[scope].LV; while (*lv) { if (!strcmp((*lv)->name, name)) { struct lV *node = *lv; *lv = node->next; if (!node->ref) free(node->data); free(node); return; } lv = &(*lv)->next; }
        struct mV **mv = &envStack[scope].MV; while (*mv) { if (!strcmp((*mv)->name, name)) { struct mV *node = *mv; *mv = node->next; if (node->entries) free(node->entries); free(node); return; } mv = &(*mv)->next; }
        struct oV **ov = &envStack[scope].OV; while (*ov) { if (!strcmp((*ov)->name, name)) { struct oV *node = *ov; *ov = node->next; free(node); return; } ov = &(*ov)->next; }
    }
}
