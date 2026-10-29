
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

struct savedVar { char name[100]; int type; int intVal; float floatVal; char charVal; char strVal[1000]; };

struct lV { char name[100]; int size; int capacity; struct Value *data; struct lV *ref; struct lV *next; };

struct callFrame {
    struct iV *IV; struct fV *FV; struct cV *CV; struct bV *BV; struct sV *SV;
    struct iA *IA; struct fA *FA; struct cA *CA; struct bA *BA;
    struct lV *LV; struct mV *MV;
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

int typeFetcher(char *name) {
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
    }
    return 0;
}

struct Value valFetcherGlobal(char *name) {
    struct Value v;
    v.type = VAL_ERROR;
    v.f = 0.0;
    v.ptr = NULL;

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
    }
}
