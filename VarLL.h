
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct iV { char name[100]; int data; struct iV *next; };
struct fV { char name[100]; float data; struct fV *next; };
struct cV { char name[100]; char data; struct cV *next; };
struct bV { char name[100]; int data; struct bV *next; };
struct sV { char name[100]; char data[1000]; struct sV *next; };

struct iA { char name[100]; int size; int *data; struct iA *next; };
struct fA { char name[100]; int size; float *data; struct fA *next; };
struct cA { char name[100]; int size; char *data; struct cA *next; };
struct bA { char name[100]; int size; int *data; struct bA *next; };

struct lV { char name[100]; int size; int capacity; float *data; struct lV *next; };
struct mV { char name[100]; char keys[100][100]; float values[100]; int count; struct mV *next; };

struct funcParam { int type; char name[100]; };
struct funcDef { char name[100]; int paramCount; struct funcParam params[10]; long bodyPos; char filepath[1000]; struct funcDef *next; };

struct savedVar { char name[100]; int type; int intVal; float floatVal; char charVal; char strVal[1000]; };

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
    struct lV *lv = frame->LV; while(lv) { struct lV *n = lv->next; free(lv->data); free(lv); lv = n; } frame->LV = NULL;
    struct mV *mv = frame->MV; while(mv) { struct mV *n = mv->next; free(mv); mv = n; } frame->MV = NULL;
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
    while (IA) { if (!strcmp(name, IA->name)) { if (idx >= 0 && idx < IA->size) return IA->data[idx]; return 0; } IA = IA->next; } return 0;
}
float getFA(struct fA *FA, char *name, int idx) {
    while (FA) { if (!strcmp(name, FA->name)) { if (idx >= 0 && idx < FA->size) return FA->data[idx]; return 0.0; } FA = FA->next; } return 0.0;
}
char getCA(struct cA *CA, char *name, int idx) {
    while (CA) { if (!strcmp(name, CA->name)) { if (idx >= 0 && idx < CA->size) return CA->data[idx]; return '\0'; } CA = CA->next; } return '\0';
}
int getBA(struct bA *BA, char *name, int idx) {
    while (BA) { if (!strcmp(name, BA->name)) { if (idx >= 0 && idx < BA->size) return BA->data[idx]; return 0; } BA = BA->next; } return 0;
}
float getL(struct lV *LV, char *name, int idx) {
    while (LV) { if (!strcmp(name, LV->name)) { if (idx >= 0 && idx < LV->size) return LV->data[idx]; } LV = LV->next; } return 0.0;
}

void setIA(struct iA *IA, char *name, int idx, int val) {
    while (IA) { if (!strcmp(name, IA->name)) { if (idx >= 0 && idx < IA->size) IA->data[idx] = val; return; } IA = IA->next; }
}
void setFA(struct fA *FA, char *name, int idx, float val) {
    while (FA) { if (!strcmp(name, FA->name)) { if (idx >= 0 && idx < FA->size) FA->data[idx] = val; return; } FA = FA->next; }
}
void setCA(struct cA *CA, char *name, int idx, char val) {
    while (CA) { if (!strcmp(name, CA->name)) { if (idx >= 0 && idx < CA->size) CA->data[idx] = val; return; } CA = CA->next; }
}
void setBA(struct bA *BA, char *name, int idx, int val) {
    while (BA) { if (!strcmp(name, BA->name)) { if (idx >= 0 && idx < BA->size) BA->data[idx] = val; return; } BA = BA->next; }
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
    nnode->data = (float *)malloc(sizeof(float) * capacity);
    nnode->next = *q; *q = nnode;
}
void appendL_copy(struct lV **q, char *name, struct lV *ref) {
    struct lV *nnode = (struct lV*)malloc(sizeof(struct lV));
    strcpy(nnode->name, name); nnode->size = ref->size; nnode->capacity = ref->capacity;
    nnode->data = (float*)malloc(ref->capacity * sizeof(float));
    memcpy(nnode->data, ref->data, ref->capacity * sizeof(float));
    nnode->next = *q; *q = nnode;
}



void appendM(struct mV **q, char *name) {
    struct mV *nnode = (struct mV *)malloc(sizeof(struct mV));
    strcpy(nnode->name, name); nnode->count = 0; nnode->next = *q; *q = nnode;
}
void appendM_copy(struct mV **q, char *name, struct mV *ref) {
    struct mV *nnode = (struct mV*)malloc(sizeof(struct mV));
    memcpy(nnode, ref, sizeof(struct mV));
    strcpy(nnode->name, name);
    nnode->next = *q; *q = nnode;
}
void setM_F(struct mV *MV, char *name, char *key, float val) {
    int scopes[] = {envTop, 0};
    int num = (envTop == 0) ? 1 : 2;
    for (int idx=0; idx<num; idx++) {
        struct mV *mv = envStack[scopes[idx]].MV;
        while (mv) {
            if (!strcmp(mv->name, name)) {
                for(int i=0; i<mv->count; i++) {
                    if(!strcmp(mv->keys[i], key)) { mv->values[i] = val; return; }
                }
                if(mv->count < 100) { strcpy(mv->keys[mv->count], key); mv->values[mv->count] = val; mv->count++; }
                return;
            }
            mv = mv->next;
        }
    }
}
float getM_F(struct mV *MV, char *name, char *key) {
    int scopes[] = {envTop, 0};
    int num = (envTop == 0) ? 1 : 2;
    for (int idx=0; idx<num; idx++) {
        struct mV *mv = envStack[scopes[idx]].MV;
        while (mv) {
            if(!strcmp(mv->name, name)) {
                for(int i=0; i<mv->count; i++) {
                    if(!strcmp(mv->keys[i], key)) return mv->values[i];
                }
                return 0.0;
            }
            mv = mv->next;
        }
    }
    return 0.0;
}
char* getM_S(struct mV *MV, char *name, char *key) {
     
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


 
void setL(struct lV *LV, char *name, int idx, float val) {
    while (LV) {
        if (!strcmp(LV->name, name)) {
            if (idx >= 0 && idx < LV->size) LV->data[idx] = val;
            return;
        }
        LV = LV->next;
    }
}
void scopePushL(char *name, float val) {
    int scopes[] = {envTop, 0};
    int num = (envTop == 0) ? 1 : 2;
    for (int idx=0; idx<num; idx++) {
        struct lV *lv = envStack[scopes[idx]].LV;
        while (lv) {
            if (!strcmp(lv->name, name)) {
                if (lv->size >= lv->capacity) {
                    lv->capacity = (lv->capacity == 0) ? 4 : lv->capacity * 2;
                    lv->data = (float*)realloc(lv->data, lv->capacity * sizeof(float));
                }
                lv->data[lv->size++] = val;
                return;
            }
            lv = lv->next;
        }
    }
}
float scopePopL(char *name) {
    int scopes[] = {envTop, 0};
    int num = (envTop == 0) ? 1 : 2;
    for (int idx=0; idx<num; idx++) {
        struct lV *lv = envStack[scopes[idx]].LV;
        while (lv) {
            if (!strcmp(lv->name, name)) {
                if (lv->size > 0) return lv->data[--lv->size];
                return 0.0;
            }
            lv = lv->next;
        }
    }
    return 0.0;
}
float scopeGetLen(char *name) {
    int scopes[] = {envTop, 0};
    int num = (envTop == 0) ? 1 : 2;
    for (int idx=0; idx<num; idx++) {
        int i = scopes[idx];
        struct lV *lv = envStack[i].LV; while (lv) { if (!strcmp(lv->name, name)) return (float)lv->size; lv = lv->next; }
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
    int scopes[] = {envTop, 0};
    int num = (envTop == 0) ? 1 : 2;
    for (int idx=0; idx<num; idx++) {
        struct mV *mv = envStack[scopes[idx]].MV;
        while (mv) {
            if (!strcmp(mv->name, name)) {
                for (int i=0; i<mv->count; i++) {
                    if (!strcmp(mv->keys[i], key)) return 1.0;
                }
                return 0.0;
            }
            mv = mv->next;
        }
    }
    return 0.0;
}
void scopeMapDelete(char *name, char *key) {
    int scopes[] = {envTop, 0};
    int num = (envTop == 0) ? 1 : 2;
    for (int idx=0; idx<num; idx++) {
        struct mV *mv = envStack[scopes[idx]].MV;
        while (mv) {
            if (!strcmp(mv->name, name)) {
                for (int i=0; i<mv->count; i++) {
                    if (!strcmp(mv->keys[i], key)) {
                        for (int j=i; j<mv->count-1; j++) {
                            strcpy(mv->keys[j], mv->keys[j+1]);
                            mv->values[j] = mv->values[j+1];
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
