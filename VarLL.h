
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
struct funcDef { char name[100]; int paramCount; struct funcParam params[10]; long bodyPos; struct funcDef *next; };

struct savedVar { char name[100]; int type; int intVal; float floatVal; char charVal; char strVal[1000]; };

struct callFrame {
    struct iV *IV; struct fV *FV; struct cV *CV; struct bV *BV; struct sV *SV;
    struct iA *IA; struct fA *FA; struct cA *CA; struct bA *BA;
    struct lV *LV; struct mV *MV;
    long returnPos; 
    int hasReturn; 
    float returnValue;
    int saved_p;
    int saved_k;
    int saved_envTop;
    int savedVarCount;
    int assignType;
    char assignTarget[100];
};


extern struct callFrame callStack[100];
extern int callStackTop;
extern struct callFrame envStack[100];
extern int envTop;
extern struct funcDef *FN;

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

// Scoped getters
int typeFetcher(char *name) {
    int scopes[] = {envTop, 0};
    int num = (envTop == 0) ? 1 : 2;
    for (int idx=0; idx<num; idx++) {
        int i = scopes[idx];
        struct iV *iv = envStack[i].IV; while (iv) { if (!strcmp(iv->name, name)) return 1; iv = iv->next; }
        struct fV *fv = envStack[i].FV; while (fv) { if (!strcmp(fv->name, name)) return 2; fv = fv->next; }
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

float valFetcherGlobal(char *name) {
    int scopes[] = {envTop, 0};
    int num = (envTop == 0) ? 1 : 2;
    for (int idx=0; idx<num; idx++) {
        int i = scopes[idx];
        struct iV *iv = envStack[i].IV; while (iv) { if (!strcmp(iv->name, name)) {  return (float)iv->data; } iv = iv->next; }
        struct fV *fv = envStack[i].FV; while (fv) { if (!strcmp(fv->name, name)) {  return fv->data; } fv = fv->next; }
        struct cV *cv = envStack[i].CV; while (cv) { if (!strcmp(cv->name, name)) return (float)cv->data; cv = cv->next; }
        struct bV *bv = envStack[i].BV; while (bv) { if (!strcmp(bv->name, name)) return (float)bv->data; bv = bv->next; }
    }
    
    return 0.0;
}


int getIA(struct iA *IA, char *name, int idx) {
    while (IA) { if (!strcmp(name, IA->name)) return IA->data[idx]; IA = IA->next; } return 0;
}
float getFA(struct fA *FA, char *name, int idx) {
    while (FA) { if (!strcmp(name, FA->name)) return FA->data[idx]; FA = FA->next; } return 0.0;
}
char getCA(struct cA *CA, char *name, int idx) {
    while (CA) { if (!strcmp(name, CA->name)) return CA->data[idx]; CA = CA->next; } return '\0';
}
int getBA(struct bA *BA, char *name, int idx) {
    while (BA) { if (!strcmp(name, BA->name)) return BA->data[idx]; BA = BA->next; } return 0;
}

void setIA(struct iA *IA, char *name, int idx, int val) {
    while (IA) { if (!strcmp(name, IA->name)) { IA->data[idx] = val; return; } IA = IA->next; }
}
void setFA(struct fA *FA, char *name, int idx, float val) {
    while (FA) { if (!strcmp(name, FA->name)) { FA->data[idx] = val; return; } FA = FA->next; }
}
void setCA(struct cA *CA, char *name, int idx, char val) {
    while (CA) { if (!strcmp(name, CA->name)) { CA->data[idx] = val; return; } CA = CA->next; }
}
void setBA(struct bA *BA, char *name, int idx, int val) {
    while (BA) { if (!strcmp(name, BA->name)) { BA->data[idx] = val; return; } BA = BA->next; }
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

void appendL(struct lV **q, char *name, int capacity) {
    struct lV *nnode = (struct lV *)malloc(sizeof(struct lV));
    strcpy(nnode->name, name); nnode->size = 0; nnode->capacity = capacity;
    nnode->data = (float *)malloc(sizeof(float) * capacity);
    nnode->next = *q; *q = nnode;
}



void appendM(struct mV **q, char *name) {
    struct mV *nnode = (struct mV *)malloc(sizeof(struct mV));
    strcpy(nnode->name, name); nnode->count = 0; nnode->next = *q; *q = nnode;
}
void setM_F(struct mV *MV, char *name, char *key, float val) {
    while (MV) {
        if (!strcmp(MV->name, name)) {
            for(int i=0; i<MV->count; i++) {
                if(!strcmp(MV->keys[i], key)) { MV->values[i] = val; return; }
            }
            if(MV->count < 100) { strcpy(MV->keys[MV->count], key); MV->values[MV->count] = val; MV->count++; }
            return;
        }
        MV = MV->next;
    }
}
float getM_F(struct mV *MV, char *name, char *key) {
    while(MV) {
        if(!strcmp(MV->name, name)) {
            for(int i=0; i<MV->count; i++) {
                if(!strcmp(MV->keys[i], key)) return MV->values[i];
            }
            return 0.0;
        }
        MV = MV->next;
    }
    
    return 0.0;
}
char* getM_S(struct mV *MV, char *name, char *key) {
    // Stub for strings if they ever get added
    return "";
}

// Function management
int appendFunc(struct funcDef **list, char *name, int paramCount, struct funcParam *params, long bodyPos, FILE *fp) {
    struct funcDef *nnode = (struct funcDef*)malloc(sizeof(struct funcDef));
    strcpy(nnode->name, name); nnode->paramCount = paramCount; nnode->bodyPos = bodyPos;
    for(int i=0; i<paramCount; i++) nnode->params[i] = params[i];
    nnode->next = *list; *list = nnode;
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


// List and Map helpers for builtin operations
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
