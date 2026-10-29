#include <stdio.h>
extern FILE *current_fp;
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "Keywords.h"
#include "VarLL.h"
#include "Builtins.h"

struct Value run_interpreter_loop(FILE *fp, const char *current_filepath);

#include "ExprEvalStack.h"

struct node *Stack;
struct node2 *PFEval;

struct Value val(char *);
int priority( char);

static inline void format_string_token(char *dest, size_t dest_sz, const char *src) {
    size_t idx = 0;
    if (idx < dest_sz - 1) dest[idx++] = '"';
    for (int i = 0; src[i] && idx < dest_sz - 3; i++) {
        if (src[i] == '"') {
            dest[idx++] = '\\';
            dest[idx++] = '"';
        } else if (src[i] == '\\') {
            dest[idx++] = '\\';
            dest[idx++] = '\\';
        } else {
            dest[idx++] = src[i];
        }
    }
    if (idx < dest_sz - 1) dest[idx++] = '"';
    dest[idx] = '\0';
}

static inline void parseCallArguments(const char *argExpr, struct Value *argVals, int *argCount) {
    *argCount = 0;
    if (!argExpr) return;
    char trimmed[1000];
    strncpy(trimmed, argExpr, sizeof(trimmed) - 1);
    trimmed[sizeof(trimmed) - 1] = '\0';
    trim_spaces(trimmed);
    if (strlen(trimmed) == 0) return;

    char singleArg[1000] = "";
    int pDepth = 0;
    int in_q = 0;
    for (int k = 0; trimmed[k]; k++) {
        char ch = trimmed[k];
        if (ch == '"') in_q = !in_q;
        if (!in_q) {
            if (ch == '(') pDepth++;
            else if (ch == ')') pDepth--;
        }
        if (ch == ',' && pDepth == 0 && !in_q) {
            trim_spaces(singleArg);
            if (strlen(singleArg) > 0) {
                strcat(singleArg, " ;");
                argVals[(*argCount)++] = val(singleArg);
            }
            singleArg[0] = '\0';
        } else {
            int sl = strlen(singleArg);
            if (sl < 995) {
                singleArg[sl] = ch;
                singleArg[sl+1] = '\0';
            }
        }
    }
    trim_spaces(singleArg);
    if (strlen(singleArg) > 0) {
        strcat(singleArg, " ;");
        argVals[(*argCount)++] = val(singleArg);
    }
}

static inline void parseCallArguments(const char *argExpr, struct Value *argVals, int *argCount);
static inline struct Value executeMethod(struct objInstance *inst, struct classMethod *method, struct Value *argVals, int argCount);
static inline int runImplicitParentConstructors(struct objInstance *inst, struct classDef *parent);

static inline struct Value executeMethod(struct objInstance *inst, struct classMethod *method, struct Value *argVals, int argCount) {
    struct Value retVal;
    memset(&retVal, 0, sizeof(retVal));
    retVal.type = VAL_FLOAT;
    retVal.f = 0.0f;

    if (!inst) {
        printf("Error: Null reference: cannot call method on null receiver\n");
        retVal.type = VAL_ERROR;
        return retVal;
    }
    if (!method || !method->func) {
        printf("Error: Invalid method\n");
        retVal.type = VAL_ERROR;
        return retVal;
    }
    if (method->isPrivate) {
        struct classDef *callerContext = (callStackTop >= 0) ? callStack[callStackTop].executingClass : NULL;
        struct classDef *declaringClass = method->declaringClass ? method->declaringClass : inst->cls;
        if (callerContext != declaringClass) {
            printf("Error: Cannot access private member '%s' of class '%s' from context '%s'\n",
                   method->name,
                   declaringClass ? declaringClass->name : "<unknown>",
                   callerContext ? callerContext->name : "<global>");
            retVal.type = VAL_ERROR;
            return retVal;
        }
    }
    struct funcDef *func = method->func;
    if (argCount != func->paramCount) {
        printf("Error: Argument count mismatch for method '%s' in class '%s': expected %d, got %d\n",
               method->name, inst->cls ? inst->cls->name : "<unknown>", func->paramCount, argCount);
        retVal.type = VAL_ERROR;
        return retVal;
    }
    if (callStackTop >= 999) {
        printf("Error: Stack Overflow\n");
        retVal.type = VAL_ERROR;
        return retVal;
    }

    callStackTop++;
    struct callFrame *frame = &callStack[callStackTop];
    frame->saved_envTop = envTop;
    frame->hasReturn = 0;
    frame->returnValue.type = VAL_FLOAT;
    frame->returnValue.f = 0.0f;
    frame->returnValue.ptr = NULL;
    frame->returnValue.s[0] = '\0';
    frame->thisObj = inst;
    frame->executingClass = method->declaringClass;
    frame->isConstructor = !strcmp(method->name, "construct");
    frame->superCalled = 0;
    frame->statementCount = 0;
    frame->ctorFailed = 0;

    envTop++;
    initEnvironmentFrame(&envStack[envTop]);

    appendO(&envStack[envTop].OV, "this", inst->cls ? inst->cls->name : "", inst);

    int paramBindingError = 0;
    for (int p = 0; p < argCount && p < func->paramCount; p++) {
        if (func->params[p].type == 1) {
            appendI(&envStack[envTop].IV, func->params[p].name, (int)argVals[p].f);
        } else if (func->params[p].type == 2) {
            appendF(&envStack[envTop].FV, func->params[p].name, argVals[p].f);
        } else if (func->params[p].type == 3) {
            appendC(&envStack[envTop].CV, func->params[p].name, (char)argVals[p].f);
        } else if (func->params[p].type == 4) {
            appendB(&envStack[envTop].BV, func->params[p].name, (int)argVals[p].f);
        } else if (func->params[p].type == 5) {
            appendS(&envStack[envTop].SV, func->params[p].name, argVals[p].s);
        } else if (func->params[p].type == 6) {
            appendIA_copy(&envStack[envTop].IA, func->params[p].name, (struct iA*)argVals[p].ptr);
        } else if (func->params[p].type == 7) {
            appendFA_copy(&envStack[envTop].FA, func->params[p].name, (struct fA*)argVals[p].ptr);
        } else if (func->params[p].type == 8) {
            appendCA_copy(&envStack[envTop].CA, func->params[p].name, (struct cA*)argVals[p].ptr);
        } else if (func->params[p].type == 9) {
            appendBA_copy(&envStack[envTop].BA, func->params[p].name, (struct bA*)argVals[p].ptr);
        } else if (func->params[p].type == 10) {
            appendL_ref(&envStack[envTop].LV, func->params[p].name, (struct lV*)argVals[p].ptr);
        } else if (func->params[p].type == 11) {
            appendM_copy(&envStack[envTop].MV, func->params[p].name, (struct mV*)argVals[p].ptr);
        } else if (func->params[p].type == 12) {
            if (argVals[p].type != VAL_OBJECT) {
                printf("Error: Type mismatch: cannot pass non-object value to parameter '%s' of class '%s'\n",
                       func->params[p].name, func->params[p].className[0] ? func->params[p].className : "object");
                paramBindingError = 1;
            } else {
                struct objInstance *oInst = (struct objInstance*)argVals[p].ptr;
                if (func->params[p].className[0] != '\0' && strcmp(func->params[p].className, "object") != 0) {
                    struct classDef *targetCls = findClass(CLASS_REGISTRY, func->params[p].className);
                    if (targetCls != NULL && oInst != NULL && oInst->cls != NULL && !isSubclassOf(oInst->cls, targetCls)) {
                        printf("Error: Type mismatch: cannot pass instance of class '%s' to parameter '%s' of class '%s'\n",
                               oInst->cls->name, func->params[p].name, targetCls->name);
                        paramBindingError = 1;
                    }
                }
                appendO(&envStack[envTop].OV, func->params[p].name, func->params[p].className, oInst);
            }
        }
    }

    if (paramBindingError) {
        frame->ctorFailed = 1;
        retVal.type = VAL_ERROR;
        if (frame->saved_envTop >= 0) {
            for (int e = envTop; e > frame->saved_envTop; e--) cleanupFrame(&envStack[e]);
            envTop = frame->saved_envTop;
        } else {
            cleanupFrame(&envStack[envTop]);
            envTop--;
        }
        callStackTop--;
        return retVal;
    }

    FILE *old_fp = current_fp;
    FILE *methodFp = current_fp;
    int closeFp = 0;
    if (strlen(func->filepath) > 0) {
        methodFp = fopen(func->filepath, "rb");
        if (methodFp) {
            closeFp = 1;
            current_fp = methodFp;
        } else {
            methodFp = current_fp;
        }
    }
    long savedPos = 0;
    if (!closeFp && methodFp) { savedPos = ftell(methodFp); }
    if (methodFp) fseek(methodFp, func->bodyPos, SEEK_SET);

    struct classDef *curClass = method->declaringClass ? method->declaringClass : inst->cls;
    if (frame->isConstructor && methodFp && curClass && curClass->parent) {
        long curP = ftell(methodFp);
        char peekTok[100] = "";
        read_token_skip_comments(methodFp, peekTok);
        int firstTokenIsSuper = (!strcmp(peekTok, "super") || !strncmp(peekTok, "super.", 6));
        fseek(methodFp, curP, SEEK_SET);

        if (!firstTokenIsSuper) {
            if (parentRequiresArguments(curClass->parent) > 0) {
                struct classDef *reqP = getParentRequiringArguments(curClass->parent);
                printf("Error: Class '%s' constructor cannot initialize superclass '%s': superclass requires %d argument(s)\n",
                       curClass->name, reqP ? reqP->name : "<unknown>", parentRequiresArguments(curClass->parent));
                frame->ctorFailed = 1;
                retVal.type = VAL_ERROR;
                if (closeFp) { fclose(methodFp); current_fp = old_fp; }
                else if (methodFp) { fseek(methodFp, savedPos, SEEK_SET); }
                if (frame->saved_envTop >= 0) {
                    for (int e = envTop; e > frame->saved_envTop; e--) cleanupFrame(&envStack[e]);
                    envTop = frame->saved_envTop;
                } else {
                    cleanupFrame(&envStack[envTop]);
                    envTop--;
                }
                callStackTop--;
                return retVal;
            } else {
                int parentInitOk = runImplicitParentConstructors(inst, curClass->parent);
                if (!parentInitOk) {
                    frame->ctorFailed = 1;
                    retVal.type = VAL_ERROR;
                    if (closeFp) { fclose(methodFp); current_fp = old_fp; }
                    else if (methodFp) { fseek(methodFp, savedPos, SEEK_SET); }
                    if (frame->saved_envTop >= 0) {
                        for (int e = envTop; e > frame->saved_envTop; e--) cleanupFrame(&envStack[e]);
                        envTop = frame->saved_envTop;
                    } else {
                        cleanupFrame(&envStack[envTop]);
                        envTop--;
                    }
                    callStackTop--;
                    return retVal;
                }
            }
        }
    }

    const char *passPath = (strlen(func->filepath) > 0) ? func->filepath : NULL;
    run_interpreter_loop(methodFp, passPath);

    if (frame->hasReturn) {
        retVal = frame->returnValue;
    }
    if (frame->ctorFailed) {
        retVal.type = VAL_ERROR;
    }

    if (closeFp) {
        fclose(methodFp);
        current_fp = old_fp;
    } else if (methodFp) {
        fseek(methodFp, savedPos, SEEK_SET);
    }

    if (frame->saved_envTop >= 0) {
        for (int e = envTop; e > frame->saved_envTop; e--) {
            cleanupFrame(&envStack[e]);
        }
        envTop = frame->saved_envTop;
    } else {
        cleanupFrame(&envStack[envTop]);
        envTop--;
    }
    callStackTop--;

    return retVal;
}

static inline int runImplicitParentConstructors(struct objInstance *inst, struct classDef *parent) {
    if (!inst || !parent) return 1;
    struct classDef *cur = parent;
    while (cur && findConstructor(cur) == NULL) {
        cur = cur->parent;
    }
    if (!cur) return 1;

    struct classMethod *pCtor = findConstructor(cur);
    if (!pCtor || !pCtor->func) return 1;

    if (pCtor->func->paramCount > 0) {
        printf("Error: Superclass '%s' constructor requires %d argument(s)\n", cur->name, pCtor->func->paramCount);
        return 0;
    }
    if (pCtor->isPrivate) {
        printf("Error: Cannot access private constructor of class '%s'\n", cur->name);
        return 0;
    }
    struct Value ret = executeMethod(inst, pCtor, NULL, 0);
    if (ret.type == VAL_ERROR) return 0;
    return 1;
}

void resolve_complex_types(char *expr) {
    char temp[1000];
    strcpy(temp, expr);
    
    char tokens[200][100];
    int tokenCount = 0;
    
    char *p = temp;
    while (*p) {
        while (*p && (*p == ' ' || *p == '\t' || *p == '\n')) p++;
        if (!*p) break;
        if (*p == '"') {
            char tok[100];
            int idx = 0;
            tok[idx++] = *p++;
            while (*p && idx < 95) {
                if (*p == '\\' && *(p+1) == '"') {
                    tok[idx++] = *p++;
                    tok[idx++] = *p++;
                    continue;
                }
                if (*p == '"') {
                    tok[idx++] = *p++;
                    break;
                }
                tok[idx++] = *p++;
            }
            tok[idx] = '\0';
            strcpy(tokens[tokenCount++], tok);
        } else if (*p == '[' || *p == ']') {
            char tok[2];
            tok[0] = *p++;
            tok[1] = '\0';
            strcpy(tokens[tokenCount++], tok);
        } else if (*p == '(' || *p == ')' || *p == ',' || *p == ';') {
            char tok[2];
            tok[0] = *p++;
            tok[1] = '\0';
            strcpy(tokens[tokenCount++], tok);
        } else {
            char tok[100];
            int idx = 0;
            while (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '"' && *p != '[' && *p != ']' && *p != '(' && *p != ')' && *p != ',' && *p != ';' && idx < 95) tok[idx++] = *p++;
            tok[idx] = '\0';
            strcpy(tokens[tokenCount++], tok);
        }
    }
    
     
    int resolvedNew = 1;
    while (resolvedNew) {
        resolvedNew = 0;
        for (int i = 0; i < tokenCount; i++) {
            if (!strcmp(tokens[i], "new") && i < tokenCount - 1) {
                char className[100];
                strncpy(className, tokens[i+1], sizeof(className) - 1);
                className[sizeof(className) - 1] = '\0';
                
                int j = i + 2;
                if (j < tokenCount && !strcmp(tokens[j], "(")) {
                    int brackets = 1;
                    j++;
                    char argExpr[1000] = "";
                    while (j < tokenCount && brackets > 0) {
                        if (!strcmp(tokens[j], "(")) brackets++;
                        else if (!strcmp(tokens[j], ")")) brackets--;
                        if (brackets > 0) {
                            strcat(argExpr, tokens[j]);
                            strcat(argExpr, " ");
                        }
                        j++;
                    }
                    
                    struct classDef *cls = findClass(CLASS_REGISTRY, className);
                    if (cls == NULL) {
                        printf("Error: Unknown class '%s'\n", className);
                        strcpy(tokens[i], "null");
                    } else {
                        struct Value argVals[10];
                        int argCount = 0;
                        parseCallArguments(argExpr, argVals, &argCount);

                        struct classMethod *ctor = findConstructor(cls);
                        if (ctor == NULL) {
                            if (argCount == 0) {
                                int req = parentRequiresArguments(cls->parent);
                                if (req > 0) {
                                    struct classDef *pReq = getParentRequiringArguments(cls->parent);
                                    printf("Error: Class '%s' cannot be default-constructed: superclass '%s' requires %d argument(s)\n",
                                           className, pReq ? pReq->name : "parent", req);
                                    strcpy(tokens[i], "null");
                                } else {
                                    struct objInstance *inst = allocateObject(cls);
                                    if (!inst) {
                                        printf("Error: Out of memory\n");
                                        strcpy(tokens[i], "null");
                                    } else {
                                        int parentInitOk = runImplicitParentConstructors(inst, cls->parent);
                                        if (!parentInitOk) {
                                            freeFailedObject(inst);
                                            strcpy(tokens[i], "null");
                                        } else {
                                            char tempName[64];
                                            snprintf(tempName, sizeof(tempName), "__obj_%d", inst->id);
                                            appendO(&envStack[envTop].OV, tempName, cls->name, inst);
                                            strcpy(tokens[i], tempName);
                                        }
                                    }
                                }
                            } else {
                                printf("Error: Class '%s' has no constructor matching %d argument(s)\n", className, argCount);
                                strcpy(tokens[i], "null");
                            }
                        } else {
                            struct funcDef *cFunc = ctor->func;
                            if (cFunc && argCount != cFunc->paramCount) {
                                printf("Error: Argument count mismatch for constructor in class '%s': expected %d, got %d\n",
                                       className, cFunc->paramCount, argCount);
                                strcpy(tokens[i], "null");
                            } else {
                                struct objInstance *inst = allocateObject(cls);
                                if (!inst) {
                                    printf("Error: Out of memory\n");
                                    strcpy(tokens[i], "null");
                                } else {
                                    struct Value ctorRet = executeMethod(inst, ctor, argVals, argCount);
                                    if (ctorRet.type == VAL_ERROR) {
                                        freeFailedObject(inst);
                                        strcpy(tokens[i], "null");
                                    } else {
                                        char tempName[64];
                                        snprintf(tempName, sizeof(tempName), "__obj_%d", inst->id);
                                        appendO(&envStack[envTop].OV, tempName, cls->name, inst);
                                        strcpy(tokens[i], tempName);
                                    }
                                }
                            }
                        }
                    }
                    int shift = j - (i + 1);
                    for (int k = i + 1; k < tokenCount - shift; k++) {
                        strcpy(tokens[k], tokens[k + shift]);
                    }
                    tokenCount -= shift;
                    resolvedNew = 1;
                    break;
                }
            }
        }
    }

    for (int i = 0; i < tokenCount; i++) {
         
        if (i < tokenCount - 1 && strcmp(tokens[i+1], "(") == 0) {
            struct funcDef *func = findFunc(FN, tokens[i]);
            struct BuiltinFunc *bfunc = findBuiltin(tokens[i]);
            if (func != NULL || bfunc != NULL) {
                 
                int brackets = 1;
                int j = i + 2;
                char argExpr[1000] = "";
                while (j < tokenCount && brackets > 0) {
                    if (strcmp(tokens[j], "(") == 0) brackets++;
                    if (strcmp(tokens[j], ")") == 0) brackets--;
                    
                    if (brackets > 0) {
                        strcat(argExpr, tokens[j]);
                        strcat(argExpr, " ");
                    }
                    j++;
                }
                
                if (bfunc != NULL) {
                    char rawArgs[10][1000];
                    int rArgCount = 0;
                    char single[1000] = "";
                    int pDepth2 = 0;
                    int in_q2 = 0;
                    for (int k = 0; argExpr[k]; k++) {
                        char ch = argExpr[k];
                        if (ch == '"') in_q2 = !in_q2;
                        if (!in_q2) {
                            if (ch == '(') pDepth2++;
                            else if (ch == ')') pDepth2--;
                        }
                        if (ch == ',' && pDepth2 == 0 && !in_q2) {
                            trim_spaces(single);
                            strcpy(rawArgs[rArgCount++], single);
                            single[0] = '\0';
                        } else {
                            int sl = strlen(single);
                            if (sl < 995) {
                                single[sl] = ch;
                                single[sl+1] = '\0';
                            }
                        }
                    }
                    trim_spaces(single);
                    if (strlen(single) > 0) strcpy(rawArgs[rArgCount++], single);
                    
                    strcpy(builtin_string_return, "");
                    struct Value res = bfunc->handler(rawArgs, rArgCount);
                    if (res.type == VAL_STRING) {
                        format_string_token(tokens[i], sizeof(tokens[i]), res.s);
                    } else if (res.type == VAL_OBJECT) {
                        struct objInstance *rInst = (struct objInstance*)res.ptr;
                        if (rInst) {
                            snprintf(tokens[i], sizeof(tokens[i]), "__obj_%d", rInst->id);
                            appendO(&envStack[envTop].OV, tokens[i], rInst->cls ? rInst->cls->name : "", rInst);
                        } else {
                            strcpy(tokens[i], "null");
                        }
                    } else if (strlen(builtin_string_return) > 0) {
                        format_string_token(tokens[i], sizeof(tokens[i]), builtin_string_return);
                    } else {
                        sprintf(tokens[i], "%f", res.f);
                    }
                    int shift = j - (i + 1);
                    for (int k = i + 1; k < tokenCount - shift; k++) {
                        strcpy(tokens[k], tokens[k + shift]);
                    }
                    tokenCount -= shift;
                } else if (func != NULL) {
                     
                    struct Value argVals[10];
                    int argCount = 0;
                    
                    if (strlen(argExpr) > 0) {
                        char singleArg[1000] = "";
                        int pDepth = 0;
                        int in_q3 = 0;
                        for (int k = 0; argExpr[k]; k++) {
                            char ch = argExpr[k];
                            if (ch == '"') in_q3 = !in_q3;
                            if (!in_q3) {
                                if (ch == '(') pDepth++;
                                else if (ch == ')') pDepth--;
                            }
                            if (ch == ',' && pDepth == 0 && !in_q3) {
                                trim_spaces(singleArg);
                                strcat(singleArg, ";");
                                argVals[argCount++] = val(singleArg);
                                singleArg[0] = '\0';
                            } else {
                                int sl = strlen(singleArg);
                                if (sl < 995) {
                                    singleArg[sl] = ch;
                                    singleArg[sl+1] = '\0';
                                }
                            }
                        }
                        trim_spaces(singleArg);
                        if (strlen(singleArg) > 0) {
                            strcat(singleArg, ";");
                            argVals[argCount++] = val(singleArg);
                        }
                    }
                    
                     
                    if (callStackTop >= 999) {
                        printf("Error: Stack Overflow\n");
                        return;
                    }
                    callStackTop++;
                    struct callFrame *frame = &callStack[callStackTop];
                    frame->saved_envTop = envTop;
                    frame->hasReturn = 0;
                    frame->returnValue.type = VAL_FLOAT;
                    frame->returnValue.f = 0.0;
                    frame->thisObj = NULL;
                    frame->executingClass = NULL;
                    
                     
                    envTop++;
                    initEnvironmentFrame(&envStack[envTop]);
                    
                     
                    for (int p = 0; p < argCount && p < func->paramCount; p++) {
                         
                        if (func->params[p].type == 1) {
                            appendI(&envStack[envTop].IV, func->params[p].name, (int)argVals[p].f);
                        } else if (func->params[p].type == 2) {
                            appendF(&envStack[envTop].FV, func->params[p].name, argVals[p].f);
                        } else if (func->params[p].type == 3) {
                            appendC(&envStack[envTop].CV, func->params[p].name, (char)argVals[p].f);
                        } else if (func->params[p].type == 4) {
                            appendB(&envStack[envTop].BV, func->params[p].name, (int)argVals[p].f);
                        } else if (func->params[p].type == 5) {
                            appendS(&envStack[envTop].SV, func->params[p].name, argVals[p].s);
                        } else if (func->params[p].type == 6) {
                            appendIA_copy(&envStack[envTop].IA, func->params[p].name, (struct iA*)argVals[p].ptr);
                        } else if (func->params[p].type == 7) {
                            appendFA_copy(&envStack[envTop].FA, func->params[p].name, (struct fA*)argVals[p].ptr);
                        } else if (func->params[p].type == 8) {
                            appendCA_copy(&envStack[envTop].CA, func->params[p].name, (struct cA*)argVals[p].ptr);
                        } else if (func->params[p].type == 9) {
                            appendBA_copy(&envStack[envTop].BA, func->params[p].name, (struct bA*)argVals[p].ptr);
                        } else if (func->params[p].type == 10) {
                            appendL_ref(&envStack[envTop].LV, func->params[p].name, (struct lV*)argVals[p].ptr);
                        } else if (func->params[p].type == 11) {
                            appendM_copy(&envStack[envTop].MV, func->params[p].name, (struct mV*)argVals[p].ptr);
                        } else if (func->params[p].type == 12) {
                            if (argVals[p].type != VAL_OBJECT) {
                                printf("Error: Type mismatch: cannot pass non-object value to parameter '%s' of class '%s'\n",
                                       func->params[p].name, func->params[p].className[0] ? func->params[p].className : "object");
                            } else {
                                struct objInstance *inst = (struct objInstance*)argVals[p].ptr;
                                if (func->params[p].className[0] != '\0' && strcmp(func->params[p].className, "object") != 0) {
                                    struct classDef *targetCls = findClass(CLASS_REGISTRY, func->params[p].className);
                                    if (targetCls != NULL && inst != NULL && inst->cls != NULL && !isSubclassOf(inst->cls, targetCls)) {
                                        printf("Error: Type mismatch: cannot pass instance of class '%s' to parameter '%s' of class '%s'\n",
                                               inst->cls->name, func->params[p].name, targetCls->name);
                                    }
                                }
                                appendO(&envStack[envTop].OV, func->params[p].name, func->params[p].className, inst);
                            }
                        }
                    }
                    
                    
     
                    FILE *funcFp = current_fp;
                    int closeFp = 0;
                    if (strlen(func->filepath) > 0) {
                        funcFp = fopen(func->filepath, "rb");
                        if (funcFp) { closeFp = 1; }
                        else { funcFp = current_fp; }
                    }
                    
                    long savedPos = 0;
                    if (!closeFp) { savedPos = ftell(funcFp); }
                    
                    fseek(funcFp, func->bodyPos, SEEK_SET);
                    
                     
                    struct Value retVal = run_interpreter_loop(funcFp, NULL);
                    
                     
                    if (closeFp) { fclose(funcFp); }
                    else { fseek(funcFp, savedPos, SEEK_SET); }
                    if (frame->saved_envTop >= 0) {
                        for (int e = envTop; e > frame->saved_envTop; e--) {
                            cleanupFrame(&envStack[e]);
                        }
                        envTop = frame->saved_envTop;
                    } else {
                        cleanupFrame(&envStack[envTop]);
                        envTop--;
                    }
                    callStackTop--;
                    
                     
                    char resStr[1000];
                    if (retVal.type == VAL_STRING) {
                        format_string_token(resStr, sizeof(resStr), retVal.s);
                    } else if (retVal.type == VAL_OBJECT) {
                        struct objInstance *inst = (struct objInstance*)retVal.ptr;
                        if (inst) {
                            snprintf(resStr, sizeof(resStr), "__obj_%d", inst->id);
                            appendO(&envStack[envTop].OV, resStr, inst->cls ? inst->cls->name : "", inst);
                        } else {
                            strcpy(resStr, "null");
                        }
                    } else {
                        sprintf(resStr, "%f", retVal.f);
                    }
                    strcpy(tokens[i], resStr);
                    
                     
                    int shift = j - (i + 1);
                    for (int k = i + 1; k < tokenCount - shift; k++) {
                        strcpy(tokens[k], tokens[k + shift]);
                    }
                    tokenCount -= shift;
                }
        }
    }

    int resolvedCompound = 1;
    while (resolvedCompound) {
        resolvedCompound = 0;

        for (int i = 0; i < tokenCount; i++) {
            if (i + 3 < tokenCount && strcmp(tokens[i+1], ".") == 0 && strcmp(tokens[i+3], "(") == 0) {
                int isReceiver = 0;
                struct objInstance *inst = NULL;
                if (!strcmp(tokens[i], "this")) {
                    isReceiver = 1;
                    inst = (callStackTop >= 0) ? callStack[callStackTop].thisObj : NULL;
                    if (!inst) printf("Error: 'this' used outside of an instance method or on null receiver\n");
                } else if (!strncmp(tokens[i], "__obj_", 6)) {
                    isReceiver = 1;
                    struct Value rVal = valFetcherGlobal(tokens[i]);
                    if (rVal.type == VAL_OBJECT) inst = (struct objInstance*)rVal.ptr;
                } else if (typeFetcher(tokens[i]) == 12) {
                    isReceiver = 1;
                    struct Value rVal = valFetcherGlobal(tokens[i]);
                    if (rVal.type == VAL_OBJECT) inst = (struct objInstance*)rVal.ptr;
                }

                if (isReceiver) {
                    char methodName[100];
                    snprintf(methodName, sizeof(methodName), "%s", tokens[i+2]);

                    int brackets = 1;
                    int j = i + 4;
                    char argExpr[1000] = "";
                    while (j < tokenCount && brackets > 0) {
                        if (strcmp(tokens[j], "(") == 0) brackets++;
                        if (strcmp(tokens[j], ")") == 0) brackets--;
                        if (brackets > 0) {
                            strcat(argExpr, tokens[j]);
                            strcat(argExpr, " ");
                        }
                        j++;
                    }

                    if (!inst) {
                        printf("Error: Null reference: cannot call method '%s' on null receiver '%s'\n", methodName, tokens[i]);
                        strcpy(tokens[i], "0");
                    } else {
                        struct classMethod *method = findClassMethod(inst->cls, methodName);
                        if (!method) {
                            printf("Error: Method '%s' not found in class '%s'\n", methodName, inst->cls ? inst->cls->name : "<unknown>");
                            strcpy(tokens[i], "0");
                        } else {
                            struct Value argVals[10];
                            int argCount = 0;
                            parseCallArguments(argExpr, argVals, &argCount);

                            if (argCount != method->func->paramCount) {
                                printf("Error: Argument count mismatch for method '%s' in class '%s': expected %d, got %d\n",
                                       methodName, inst->cls ? inst->cls->name : "<unknown>", method->func->paramCount, argCount);
                                strcpy(tokens[i], "0");
                            } else {
                                struct Value retVal = executeMethod(inst, method, argVals, argCount);
                                if (retVal.type == VAL_ERROR) {
                                    strcpy(tokens[i], "0");
                                } else if (retVal.type == VAL_STRING) {
                                    format_string_token(tokens[i], sizeof(tokens[i]), retVal.s);
                                } else if (retVal.type == VAL_OBJECT) {
                                    struct objInstance *rInst = (struct objInstance*)retVal.ptr;
                                    if (rInst) {
                                        snprintf(tokens[i], sizeof(tokens[i]), "__obj_%d", rInst->id);
                                        appendO(&envStack[envTop].OV, tokens[i], rInst->cls ? rInst->cls->name : "", rInst);
                                    } else {
                                        strcpy(tokens[i], "null");
                                    }
                                } else {
                                    sprintf(tokens[i], "%f", retVal.f);
                                }
                            }
                        }
                    }

                    int shift = j - (i + 1);
                    for (int k = i + 1; k < tokenCount - shift; k++) {
                        strcpy(tokens[k], tokens[k + shift]);
                    }
                    tokenCount -= shift;
                    resolvedCompound = 1;
                    break;
                }
            }
            else if (i + 1 < tokenCount && !strcmp(tokens[i+1], "(") && tokens[i][0] != '"' && !isdigit(tokens[i][0]) && !(tokens[i][0] == '-' && isdigit(tokens[i][1]))) {
                char *lastDot = strrchr(tokens[i], '.');
                if (lastDot != NULL) {
                    int rLen = lastDot - tokens[i];
                    char recPath[100], methodName[100];
                    if (rLen > 0 && rLen < 95) {
                        strncpy(recPath, tokens[i], rLen);
                        recPath[rLen] = '\0';
                        strncpy(methodName, lastDot + 1, sizeof(methodName) - 1);
                        methodName[sizeof(methodName) - 1] = '\0';

                        int isRec = 0;
                        struct objInstance *inst = NULL;
                        char *curDot = strchr(recPath, '.');
                        if (curDot == NULL) {
                            if (!strcmp(recPath, "this")) {
                                isRec = 1;
                                inst = (callStackTop >= 0) ? callStack[callStackTop].thisObj : NULL;
                                if (!inst) printf("Error: 'this' used outside of an instance method or on null receiver\n");
                            } else if (!strncmp(recPath, "__obj_", 6)) {
                                isRec = 1;
                                struct Value rVal = valFetcherGlobal(recPath);
                                if (rVal.type == VAL_OBJECT) inst = (struct objInstance*)rVal.ptr;
                            } else if (typeFetcher(recPath) == 12) {
                                isRec = 1;
                                struct Value rVal = valFetcherGlobal(recPath);
                                if (rVal.type == VAL_OBJECT) inst = (struct objInstance*)rVal.ptr;
                            }
                        } else {
                            char rootName[100];
                            int rootLen = curDot - recPath;
                            strncpy(rootName, recPath, rootLen);
                            rootName[rootLen] = '\0';
                            if (!strcmp(rootName, "this")) {
                                isRec = 1;
                                inst = (callStackTop >= 0) ? callStack[callStackTop].thisObj : NULL;
                                if (!inst) printf("Error: 'this' used outside of an instance method or on null receiver\n");
                            } else if (!strncmp(rootName, "__obj_", 6)) {
                                isRec = 1;
                                struct Value rVal = valFetcherGlobal(rootName);
                                if (rVal.type == VAL_OBJECT) inst = (struct objInstance*)rVal.ptr;
                            } else if (typeFetcher(rootName) == 12) {
                                isRec = 1;
                                struct Value rVal = valFetcherGlobal(rootName);
                                if (rVal.type == VAL_OBJECT) inst = (struct objInstance*)rVal.ptr;
                            }
                            if (isRec && inst) {
                                char *seg = curDot + 1;
                                while (seg && *seg) {
                                    char *nd = strchr(seg, '.');
                                    char mf[100];
                                    int slen = nd ? (int)(nd - seg) : (int)strlen(seg);
                                    if (slen >= (int)sizeof(mf)) slen = sizeof(mf) - 1;
                                    memcpy(mf, seg, slen);
                                    mf[slen] = '\0';
                                    int fFound = 0;
                                    struct Value fv = getFieldValue(inst, mf, &fFound);
                                    if (fFound <= 0 || fv.type != VAL_OBJECT) { inst = NULL; break; }
                                    inst = (struct objInstance*)fv.ptr;
                                    seg = nd ? (nd + 1) : NULL;
                                }
                            }
                        }

                        if (isRec && methodName[0] != '\0') {
                            int brackets = 1;
                            int j = i + 2;
                            char argExpr[1000] = "";
                            while (j < tokenCount && brackets > 0) {
                                if (strcmp(tokens[j], "(") == 0) brackets++;
                                if (strcmp(tokens[j], ")") == 0) brackets--;
                                if (brackets > 0) {
                                    strcat(argExpr, tokens[j]);
                                    strcat(argExpr, " ");
                                }
                                j++;
                            }

                            if (!inst) {
                                printf("Error: Null reference: cannot call method '%s' on null receiver '%s'\n", methodName, recPath);
                                strcpy(tokens[i], "0");
                            } else {
                                struct classMethod *method = findClassMethod(inst->cls, methodName);
                                if (!method) {
                                    printf("Error: Method '%s' not found in class '%s'\n", methodName, inst->cls ? inst->cls->name : "<unknown>");
                                    strcpy(tokens[i], "0");
                                } else {
                                    struct Value argVals[10];
                                    int argCount = 0;
                                    parseCallArguments(argExpr, argVals, &argCount);

                                    if (argCount != method->func->paramCount) {
                                        printf("Error: Argument count mismatch for method '%s' in class '%s': expected %d, got %d\n",
                                               methodName, inst->cls ? inst->cls->name : "<unknown>", method->func->paramCount, argCount);
                                        strcpy(tokens[i], "0");
                                    } else {
                                        struct Value retVal = executeMethod(inst, method, argVals, argCount);
                                        if (retVal.type == VAL_ERROR) {
                                            strcpy(tokens[i], "0");
                                        } else if (retVal.type == VAL_STRING) {
                                            format_string_token(tokens[i], sizeof(tokens[i]), retVal.s);
                                        } else if (retVal.type == VAL_OBJECT) {
                                            struct objInstance *rInst = (struct objInstance*)retVal.ptr;
                                            if (rInst) {
                                                snprintf(tokens[i], sizeof(tokens[i]), "__obj_%d", rInst->id);
                                                appendO(&envStack[envTop].OV, tokens[i], rInst->cls ? rInst->cls->name : "", rInst);
                                            } else {
                                                strcpy(tokens[i], "null");
                                            }
                                        } else {
                                            sprintf(tokens[i], "%f", retVal.f);
                                        }
                                    }
                                }
                            }

                            int shift = j - (i + 1);
                            for (int k = i + 1; k < tokenCount - shift; k++) {
                                strcpy(tokens[k], tokens[k + shift]);
                            }
                            tokenCount -= shift;
                            resolvedCompound = 1;
                            break;
                        }
                    }
                }
            }
        }

        if (resolvedCompound) continue;

        for (int i = 0; i < tokenCount; i++) {
            if (i + 2 < tokenCount && !strcmp(tokens[i+1], ".")) {
                if (i + 3 < tokenCount && !strcmp(tokens[i+3], "(")) {
                    continue;
                }
                
                int isReceiver = 0;
                struct objInstance *inst = NULL;
                if (!strcmp(tokens[i], "this")) {
                    isReceiver = 1;
                    inst = (callStackTop >= 0) ? callStack[callStackTop].thisObj : NULL;
                    if (!inst) {
                        printf("Error: 'this' used outside of an instance method or on null receiver\n");
                    }
                } else if (!strncmp(tokens[i], "__obj_", 6)) {
                    isReceiver = 1;
                    struct Value rVal = valFetcherGlobal(tokens[i]);
                    if (rVal.type == VAL_OBJECT) inst = (struct objInstance*)rVal.ptr;
                } else if (typeFetcher(tokens[i]) == 12) {
                    isReceiver = 1;
                    struct Value rVal = valFetcherGlobal(tokens[i]);
                    if (rVal.type == VAL_OBJECT) inst = (struct objInstance*)rVal.ptr;
                }
                
                if (isReceiver) {
                    char *fieldName = tokens[i+2];
                    if (!inst) {
                        printf("Error: Null reference: cannot read field '%s' on null receiver '%s'\n", fieldName, tokens[i]);
                        strcpy(tokens[i], "null");
                    } else {
                        int found = 0;
                        struct Value fVal = getFieldValue(inst, fieldName, &found);
                        if (found == 0) {
                            printf("Error: Field '%s' not found in class '%s'\n", fieldName, inst->cls ? inst->cls->name : "<unknown>");
                            strcpy(tokens[i], "0");
                        } else if (found < 0) {
                            strcpy(tokens[i], "null");
                        } else {
                            if (fVal.type == VAL_STRING) {
                                format_string_token(tokens[i], sizeof(tokens[i]), fVal.s);
                            } else if (fVal.type == VAL_OBJECT) {
                                struct objInstance *fInst = (struct objInstance*)fVal.ptr;
                                if (fInst) {
                                    snprintf(tokens[i], sizeof(tokens[i]), "__obj_%d", fInst->id);
                                    appendO(&envStack[envTop].OV, tokens[i], fInst->cls ? fInst->cls->name : "", fInst);
                                } else {
                                    strcpy(tokens[i], "null");
                                }
                            } else if (fVal.type == VAL_LIST) {
                                if (fVal.ptr != NULL) {
                                    char lName[64];
                                    snprintf(lName, sizeof(lName), "__lst_%d_%s", inst->id, fieldName);
                                    appendL_ref(&envStack[envTop].LV, lName, (struct lV*)fVal.ptr);
                                    strcpy(tokens[i], lName);
                                } else {
                                    strcpy(tokens[i], "null");
                                }
                            } else if (fVal.type == VAL_MAP) {
                                if (fVal.ptr != NULL) {
                                    char mName[64];
                                    snprintf(mName, sizeof(mName), "__map_%d_%s", inst->id, fieldName);
                                    appendM_copy(&envStack[envTop].MV, mName, (struct mV*)fVal.ptr);
                                    strcpy(tokens[i], mName);
                                } else {
                                    strcpy(tokens[i], "null");
                                }
                            } else if (fVal.type == VAL_FLOAT) {
                                snprintf(tokens[i], sizeof(tokens[i]), "%f", fVal.f);
                            } else {
                                char sBuf[1000] = "";
                                value_to_string(fVal, sBuf, sizeof(sBuf));
                                format_string_token(tokens[i], sizeof(tokens[i]), sBuf);
                            }
                        }
                    }
                    int shift = 2;
                    for (int k = i + 1; k < tokenCount - shift; k++) {
                        strcpy(tokens[k], tokens[k + shift]);
                    }
                    tokenCount -= shift;
                    resolvedCompound = 1;
                    break;
                }
            }
            else if (tokens[i][0] != '"' && !isdigit(tokens[i][0]) && !(tokens[i][0] == '-' && isdigit(tokens[i][1]))) {
                char *dot = strchr(tokens[i], '.');
                if (dot != NULL) {
                    if (i + 1 < tokenCount && !strcmp(tokens[i+1], "(")) {
                        continue;
                    }
                    int rLen = dot - tokens[i];
                    if (rLen > 0 && rLen < 95) {
                        char rec[100], fld[100];
                        strncpy(rec, tokens[i], rLen);
                        rec[rLen] = '\0';
                        char *secondDot = strchr(dot + 1, '.');
                        int fLen = secondDot ? (int)(secondDot - (dot + 1)) : (int)strlen(dot + 1);
                        if (fLen >= (int)sizeof(fld)) fLen = (int)sizeof(fld) - 1;
                        memcpy(fld, dot + 1, fLen);
                        fld[fLen] = '\0';
                        char *rest = secondDot ? secondDot : "";
                        
                        int isRec = 0;
                        struct objInstance *inst = NULL;
                        if (!strcmp(rec, "this")) {
                            isRec = 1;
                            inst = (callStackTop >= 0) ? callStack[callStackTop].thisObj : NULL;
                            if (!inst) printf("Error: 'this' used outside of an instance method or on null receiver\n");
                        } else if (!strncmp(rec, "__obj_", 6)) {
                            isRec = 1;
                            struct Value rVal = valFetcherGlobal(rec);
                            if (rVal.type == VAL_OBJECT) inst = (struct objInstance*)rVal.ptr;
                        } else if (typeFetcher(rec) == 12) {
                            isRec = 1;
                            struct Value rVal = valFetcherGlobal(rec);
                            if (rVal.type == VAL_OBJECT) inst = (struct objInstance*)rVal.ptr;
                        }
                        
                        if (isRec && fld[0] != '\0') {
                            if (!inst) {
                                printf("Error: Null reference: cannot read field '%s' on null receiver '%s'\n", fld, rec);
                                strcpy(tokens[i], "null");
                            } else {
                                int found = 0;
                                struct Value fVal = getFieldValue(inst, fld, &found);
                                if (found == 0) {
                                    printf("Error: Field '%s' not found in class '%s'\n", fld, inst->cls ? inst->cls->name : "<unknown>");
                                    strcpy(tokens[i], "0");
                                } else if (found < 0) {
                                    strcpy(tokens[i], "null");
                                } else if (rest[0] != '\0') {
                                    if (fVal.type == VAL_OBJECT) {
                                        struct objInstance *fInst = (struct objInstance*)fVal.ptr;
                                        if (fInst) {
                                            char oName[100];
                                            snprintf(oName, sizeof(oName), "__obj_%d", fInst->id);
                                            appendO(&envStack[envTop].OV, oName, fInst->cls ? fInst->cls->name : "", fInst);
                                            snprintf(tokens[i], sizeof(tokens[i]), "%s%s", oName, rest);
                                        } else {
                                            printf("Error: Null reference: cannot access field on null object\n");
                                            strcpy(tokens[i], "null");
                                        }
                                    } else {
                                        printf("Error: Cannot access member on non-object value\n");
                                        strcpy(tokens[i], "0");
                                    }
                                } else {
                                    if (fVal.type == VAL_STRING) {
                                        format_string_token(tokens[i], sizeof(tokens[i]), fVal.s);
                                    } else if (fVal.type == VAL_OBJECT) {
                                        struct objInstance *fInst = (struct objInstance*)fVal.ptr;
                                        if (fInst) {
                                            snprintf(tokens[i], sizeof(tokens[i]), "__obj_%d", fInst->id);
                                            appendO(&envStack[envTop].OV, tokens[i], fInst->cls ? fInst->cls->name : "", fInst);
                                        } else {
                                            strcpy(tokens[i], "null");
                                        }
                                    } else if (fVal.type == VAL_LIST) {
                                        if (fVal.ptr != NULL) {
                                            char lName[64];
                                            snprintf(lName, sizeof(lName), "__lst_%d_%s", inst->id, fld);
                                            appendL_ref(&envStack[envTop].LV, lName, (struct lV*)fVal.ptr);
                                            strcpy(tokens[i], lName);
                                        } else {
                                            strcpy(tokens[i], "null");
                                        }
                                    } else if (fVal.type == VAL_MAP) {
                                        if (fVal.ptr != NULL) {
                                            char mName[64];
                                            snprintf(mName, sizeof(mName), "__map_%d_%s", inst->id, fld);
                                            appendM_copy(&envStack[envTop].MV, mName, (struct mV*)fVal.ptr);
                                            strcpy(tokens[i], mName);
                                        } else {
                                            strcpy(tokens[i], "null");
                                        }
                                    } else if (fVal.type == VAL_FLOAT) {
                                        snprintf(tokens[i], sizeof(tokens[i]), "%f", fVal.f);
                                    } else {
                                        char sBuf[1000] = "";
                                        value_to_string(fVal, sBuf, sizeof(sBuf));
                                        format_string_token(tokens[i], sizeof(tokens[i]), sBuf);
                                    }
                                }
                            }
                            resolvedCompound = 1;
                            break;
                        }
                    }
                }
            }
        }

        if (resolvedCompound) continue;

        for (int i = 0; i < tokenCount; i++) {
            if (i < tokenCount - 3 && strcmp(tokens[i+1], "[") == 0) {
                int type = typeFetcher(tokens[i]);
                if (type >= 6 && type <= 11) {
                    struct Value curVal = valFetcherGlobal(tokens[i]);
                    int nextIdx = i + 1;
                    while (nextIdx < tokenCount && strcmp(tokens[nextIdx], "[") == 0) {
                        int bDepth = 1;
                        int j = nextIdx + 1;
                        char argExpr[1000] = "";
                        while (j < tokenCount && bDepth > 0) {
                            if (strcmp(tokens[j], "[") == 0) bDepth++;
                            else if (strcmp(tokens[j], "]") == 0) bDepth--;
                            if (bDepth > 0) {
                                strcat(argExpr, tokens[j]);
                                strcat(argExpr, " ");
                            }
                            j++;
                        }
                        strcat(argExpr, ";");
                        struct Value idxVal = val(argExpr);
                        
                        if (curVal.type == VAL_LIST) {
                            struct lV *lv = (struct lV*)curVal.ptr;
                            while (lv && lv->ref) lv = lv->ref;
                            int idx = (int)idxVal.f;
                            if (lv && idx >= 0 && idx < lv->size) {
                                curVal = lv->data[idx];
                            } else {
                                curVal.type = VAL_ERROR;
                                curVal.f = 0.0;
                            }
                        } else if (curVal.type == VAL_MAP) {
                            struct mV *mv = (struct mV*)curVal.ptr;
                            char key[100] = "";
                            if (idxVal.type == VAL_STRING) {
                                strcpy(key, idxVal.s);
                            } else if (idxVal.type == VAL_FLOAT) {
                                if (idxVal.f == (float)(int)idxVal.f) sprintf(key, "%d", (int)idxVal.f);
                                else sprintf(key, "%f", idxVal.f);
                            } else {
                                sscanf(argExpr, "%s", key);
                                if (key[0] == '"') {
                                    key[strlen(key)-1] = '\0';
                                    memmove(key, key+1, strlen(key));
                                }
                            }
                            int found = 0;
                            if (mv) {
                                for (int k = 0; k < mv->count; k++) {
                                    if (!strcmp(mv->entries[k].key, key)) {
                                        curVal = mv->entries[k].value;
                                        found = 1;
                                        break;
                                    }
                                }
                            }
                            if (!found) {
                                curVal.type = VAL_ERROR;
                                curVal.f = 0.0;
                            }
                        } else if (curVal.type == VAL_INT_ARRAY) {
                            struct iA *ia = (struct iA*)curVal.ptr;
                            int idx = (int)idxVal.f;
                            curVal.type = VAL_FLOAT;
                            curVal.f = (ia && idx >= 0 && idx < ia->size) ? (float)ia->data[idx] : 0.0f;
                        } else if (curVal.type == VAL_FLOAT_ARRAY) {
                            struct fA *fa = (struct fA*)curVal.ptr;
                            int idx = (int)idxVal.f;
                            curVal.type = VAL_FLOAT;
                            curVal.f = (fa && idx >= 0 && idx < fa->size) ? fa->data[idx] : 0.0f;
                        } else if (curVal.type == VAL_CHAR_ARRAY) {
                            struct cA *ca = (struct cA*)curVal.ptr;
                            int idx = (int)idxVal.f;
                            curVal.type = VAL_FLOAT;
                            curVal.f = (ca && idx >= 0 && idx < ca->size) ? (float)ca->data[idx] : 0.0f;
                        } else if (curVal.type == VAL_BOOL_ARRAY) {
                            struct bA *ba = (struct bA*)curVal.ptr;
                            int idx = (int)idxVal.f;
                            curVal.type = VAL_FLOAT;
                            curVal.f = (ba && idx >= 0 && idx < ba->size) ? (float)ba->data[idx] : 0.0f;
                        } else {
                            curVal.type = VAL_ERROR;
                            curVal.f = 0.0;
                        }
                        
                        nextIdx = j;
                    }
                    
                    if (curVal.type == VAL_STRING) {
                        format_string_token(tokens[i], sizeof(tokens[i]), curVal.s);
                    } else if (curVal.type == VAL_FLOAT) {
                        sprintf(tokens[i], "%f", curVal.f);
                    } else if (curVal.type == VAL_OBJECT) {
                        struct objInstance *rInst = (struct objInstance*)curVal.ptr;
                        if (rInst) {
                            snprintf(tokens[i], sizeof(tokens[i]), "__obj_%d", rInst->id);
                            appendO(&envStack[envTop].OV, tokens[i], rInst->cls ? rInst->cls->name : "", rInst);
                        } else {
                            strcpy(tokens[i], "null");
                        }
                    } else {
                        char strVal[1000] = "";
                        value_to_string(curVal, strVal, sizeof(strVal));
                        format_string_token(tokens[i], sizeof(tokens[i]), strVal);
                    }
                    int shift = nextIdx - (i + 1);
                    for (int k = i + 1; k < tokenCount - shift; k++) {
                        strcpy(tokens[k], tokens[k + shift]);
                    }
                    tokenCount -= shift;
                    resolvedCompound = 1;
                    break;
                }
            }
        }
    }

    if (1) {
         
        expr[0] = '\0';
        for (int i = 0; i < tokenCount; i++) {
            if (i > 0) strcat(expr, " ");
            strcat(expr, tokens[i]);
        }
    }
}

}

struct Value val(char *buff)
{
    if (buff == NULL) { struct Value ev; ev.type=VAL_FLOAT; ev.f=0.0; ev.s[0]='\0'; return ev; }
    char *chk = buff;
    while (*chk == ' ' || *chk == '\t' || *chk == '\r' || *chk == '\n') chk++;
    if (*chk == ';' || *chk == '\0') {
        struct Value ev; ev.type=VAL_FLOAT; ev.f=0.0; ev.s[0]='\0'; return ev;
    }
    Stack = NULL;
    PFEval = NULL;
    struct Value tot;
    float x; struct Value n1, n2;
    resolve_complex_types(buff);

    int c;
    
    char postfix[1000]="", *pf, opr;
    pf= &postfix[0];

    char *f, func[100];
    f= &func[0];

    char *t, temp[100];
    t= &temp[0];



     

    int in_quotes = 0;
    int prev_was_op = 1;
    while( (*buff != ';' || in_quotes) && *buff != '\n' && *buff != '\0')
    {
        if (*buff == '\\' && *(buff+1) == '"') {
            if (in_quotes) {
                *pf = *buff; pf++; buff++;
                *pf = *buff; pf++; buff++;
                continue;
            }
        }
        if (*buff == '"') {
            in_quotes = !in_quotes;
            prev_was_op = 0;
        }
        
        if(*buff == '(' && !in_quotes)
        {
            push( &Stack, *buff);
            buff++;
            prev_was_op = 1;
        }
        else if (*buff == '=' && *(buff+1) == '=' && !in_quotes) { *buff = '#'; strcpy(buff+1, buff+2); continue; }
        else if (*buff == '&' && *(buff+1) == '&' && !in_quotes) { *buff = '&'; strcpy(buff+1, buff+2); continue; }
        else if (*buff == '|' && *(buff+1) == '|' && !in_quotes) { *buff = '|'; strcpy(buff+1, buff+2); continue; }
        else if( (*buff == ' ' || *buff == '\t' || *buff == '\r' || *buff == '=') && !in_quotes)
        {
            strcpy(buff, buff+1);
            continue;
        }
        else if (*buff == '!' && *(buff+1) == '=' && !in_quotes) { *buff = '@'; strcpy(buff+1, buff+2); continue; }
        else if (*buff == '<' && *(buff+1) == '=' && !in_quotes) { *buff = '^'; strcpy(buff+1, buff+2); continue; }
        else if (*buff == '>' && *(buff+1) == '=' && !in_quotes) { *buff = '?'; strcpy(buff+1, buff+2); continue; }
        else if( (*buff == '$' || *buff == '*' || *buff == '/' || *buff == '%' || *buff == '+' || (*buff == '-' && !(prev_was_op && (isdigit(*(buff+1)) || *(buff+1) == '.'))) || *buff == '<' || *buff == '>' || *buff == '&' || *buff == '|' || *buff == '~' || *buff == '!' || *buff == '#' || *buff == '@' || *buff == '^' || *buff == '?') && !in_quotes)
        {
            *f='\0';
            f= &func[0];
            
            if (Stack != NULL)
            {
                opr= pop( &Stack);
                if(priority (opr) >= priority (*buff ))
                {
                    *pf= ' ';
                    pf++;
                    while ( priority (opr) >= priority (*buff ) )
				    {
		    			*pf = opr;
			    		pf++;

                        opr= pop( &Stack);

                        *pf= ' ';
                        pf++;
				    }
                }
                else
                {
                *pf= ' ';
                pf++;
                }
                
				push ( &Stack, opr );
				push ( &Stack, *buff);
            }
			else
            {
                *pf= ' ';
                pf++;
				push( &Stack, *buff );
            }

			buff++ ;
            if( *buff == ' ' || *buff == '\t' || *buff == '\r')
                buff++;
            prev_was_op = 1;
        }
        else if( *buff == ')' && !in_quotes)
        {
            opr = pop ( &Stack) ;
			while ( ( opr ) != '(' && ( opr ) != ' ' )
			{
                *pf= ' ';
                pf++;
				*pf = opr ;
				pf++ ;
				opr =  pop ( &Stack) ;
			}
			buff++;
            prev_was_op = 0;
        }
        else
        {
            *pf= *buff;
            pf++;

            *f= *buff;
            *f++;

            buff++;
            prev_was_op = 0;
        }
    }

    while ( Stack != NULL)
	{
        *pf= ' ';
        pf++ ;
		opr= pop( &Stack);
        if (opr != ' ') {
            *pf= opr;
            pf++;
        }
	}

    *pf = ' '; pf++; *pf = '\0';

    pf= &postfix[0];
    int eval_in_quotes = 0;
    while(*pf != '\0')
    {
        if (*pf == '\\' && *(pf+1) == '"') {
            if (eval_in_quotes) {
                *t = *pf; t++; pf++;
                *t = *pf; t++; pf++;
                continue;
            }
        }
        if (*pf == '"') {
            eval_in_quotes = !eval_in_quotes;
        }
        if(*pf == ' ' && !eval_in_quotes)
        {
            *t= '\0';
            
            if (strlen(temp) > 0) {
                if (!strcmp(temp, "true")) { { struct Value v; v.type=VAL_FLOAT; v.f=1.0; pushV(v, &PFEval); } }
                else if (!strcmp(temp, "false")) { { struct Value v; v.type=VAL_FLOAT; v.f=0.0; pushV(v, &PFEval); } }
                else if (!strcmp(temp, "null")) { { struct Value v; v.type=VAL_OBJECT; v.f=0.0; v.ptr=NULL; v.s[0]='\0'; pushV(v, &PFEval); } }
                else {
                    c= typeFetcher(temp);
                    if (c >= 1 && c <= 12) { pushV(valFetcherGlobal(temp), &PFEval); }
                    else if (temp[0] == '"' || (temp[0] == '\\' && temp[1] == '"')) {
                        struct Value v; v.type = VAL_STRING;
                        char *start = (temp[0] == '\\' && temp[1] == '"') ? temp + 2 : temp + 1;
                        int vl = strlen(start);
                        if (vl >= 2 && start[vl-2] == '\\' && start[vl-1] == '"') start[vl-2] = '\0';
                        else if (vl >= 1 && start[vl-1] == '"') start[vl-1] = '\0';
                        int d = 0;
                        for (int s = 0; start[s] && d < 995; s++) {
                            if (start[s] == '\\' && start[s+1] == '"') {
                                v.s[d++] = '"';
                                s++;
                            } else if (start[s] == '\\' && start[s+1] == '\\') {
                                v.s[d++] = '\\';
                                s++;
                            } else {
                                v.s[d++] = start[s];
                            }
                        }
                        v.s[d] = '\0';
                        pushV(v, &PFEval);
                    }
                    else if (temp[0] == '\'') {
                        char chVal = (temp[1] == '\\') ? ((temp[2] == 'n') ? '\n' : ((temp[2] == 't') ? '\t' : temp[2])) : temp[1];
                        struct Value v; v.type = VAL_FLOAT; v.f = (float)chVal; pushV(v, &PFEval);
                    }
                    else if(isdigit(temp[0]) || (temp[0] == '-' && isdigit(temp[1]))) { x= atof(temp);  { struct Value v; v.type=VAL_FLOAT; v.f=x; pushV(v, &PFEval); } }
                }
            }
            t= &temp[0];
            *t = '\0';
        }
        else if (priority(*pf) && !(*pf == '-' && (isdigit(*(pf+1)) || *(pf+1) == '.')) && !eval_in_quotes)
        {
            n1= popV(&PFEval);
            n2= popV(&PFEval);
            tot.type = VAL_FLOAT;
            tot.f = 0.0;
             
            if (n1.type == VAL_OBJECT || n2.type == VAL_OBJECT) {
                if (*pf == '+') {
                    tot.type = VAL_STRING;
                    char s1[1000] = "", s2[1000] = "";
                    value_to_string(n2, s2, sizeof(s2));
                    value_to_string(n1, s1, sizeof(s1));
                    snprintf(tot.s, sizeof(tot.s), "%s%s", s2, s1);
                    pushV(tot, &PFEval);
                    pf++;
                    continue;
                } else if (*pf == '~' || *pf == '#') {
                    tot.type = VAL_FLOAT;
                    if (n1.type == VAL_OBJECT && n2.type == VAL_OBJECT) {
                        tot.f = (n1.ptr == n2.ptr) ? 1.0 : 0.0;
                    } else {
                        tot.f = 0.0;
                    }
                    pushV(tot, &PFEval);
                    pf++;
                    continue;
                } else if (*pf == '!' || *pf == '@') {
                    tot.type = VAL_FLOAT;
                    if (n1.type == VAL_OBJECT && n2.type == VAL_OBJECT) {
                        tot.f = (n1.ptr != n2.ptr) ? 1.0 : 0.0;
                    } else {
                        tot.f = 1.0;
                    }
                    pushV(tot, &PFEval);
                    pf++;
                    continue;
                } else {
                    tot.type = VAL_ERROR;
                    tot.f = 0.0;
                    pushV(tot, &PFEval);
                    pf++;
                    continue;
                }
            }

            if (n1.type >= VAL_INT_ARRAY || n2.type >= VAL_INT_ARRAY) {
                if (*pf == '+') {
                    tot.type = VAL_STRING;
                    char s1[1000] = "", s2[1000] = "";
                    value_to_string(n2, s2, sizeof(s2));
                    value_to_string(n1, s1, sizeof(s1));
                    snprintf(tot.s, sizeof(tot.s), "%s%s", s2, s1);
                    pushV(tot, &PFEval);
                    pf++;
                    continue;
                } else if (*pf == '~' || *pf == '#') {
                    char s1[1000] = "", s2[1000] = "";
                    value_to_string(n2, s2, sizeof(s2));
                    value_to_string(n1, s1, sizeof(s1));
                    tot.type = VAL_FLOAT;
                    tot.f = (strcmp(s1, s2) == 0) ? 1.0 : 0.0;
                    pushV(tot, &PFEval);
                    pf++;
                    continue;
                } else if (*pf == '!' || *pf == '@') {
                    char s1[1000] = "", s2[1000] = "";
                    value_to_string(n2, s2, sizeof(s2));
                    value_to_string(n1, s1, sizeof(s1));
                    tot.type = VAL_FLOAT;
                    tot.f = (strcmp(s1, s2) != 0) ? 1.0 : 0.0;
                    pushV(tot, &PFEval);
                    pf++;
                    continue;
                } else {
                    tot.type = VAL_ERROR;
                    tot.f = 0.0;
                    pushV(tot, &PFEval);
                    pf++;
                    continue;
                }
            }
            switch(*pf)
            {
                case '$': tot.f= pow(n2.f,n1.f); break;
                case '%': tot.f= remainder(n2.f,n1.f); break;
                case '/': tot.f= n2.f/n1.f; break;
                case '*': tot.f= n2.f*n1.f; break;
                case '+':
                    if (n1.type == VAL_STRING || n2.type == VAL_STRING) {
                        tot.type = VAL_STRING;
                        char s2[1000] = "", s1[1000] = "";
                        if (n2.type == VAL_STRING) strcpy(s2, n2.s);
                        else if (n2.type == VAL_FLOAT) { if (n2.f == (float)(int)n2.f) sprintf(s2, "%d", (int)n2.f); else sprintf(s2, "%f", n2.f); }
                        if (n1.type == VAL_STRING) strcpy(s1, n1.s);
                        else if (n1.type == VAL_FLOAT) { if (n1.f == (float)(int)n1.f) sprintf(s1, "%d", (int)n1.f); else sprintf(s1, "%f", n1.f); }
                        snprintf(tot.s, sizeof(tot.s), "%s%s", s2, s1);
                    } else {
                        tot.f= n2.f+n1.f;
                    }
                    break;
                case '-': tot.f= n2.f-n1.f; break;
                case '<': tot.f= n2.f<n1.f; break;
                case '>': tot.f= n2.f>n1.f; break;
                case '&': tot.f= n2.f&&n1.f; break;
                case '|': tot.f= n2.f||n1.f; break;
                case '~':
                case '#':
                    if (n1.type == VAL_STRING && n2.type == VAL_STRING) {
                        tot.f = (strcmp(n1.s, n2.s) == 0) ? 1.0 : 0.0;
                    } else {
                        tot.f= n2.f==n1.f;
                    }
                    break;
                case '!':
                case '@':
                    if (n1.type == VAL_STRING && n2.type == VAL_STRING) {
                        tot.f = (strcmp(n1.s, n2.s) != 0) ? 1.0 : 0.0;
                    } else {
                        tot.f= n2.f!=n1.f;
                    }
                    break;
                case '^': tot.f= n2.f<=n1.f; break;
                case '?': tot.f= n2.f>=n1.f; break;
            }
            pushV(tot, &PFEval);
        }
        else
        {
            *t= *pf;
            t++;
        }
        pf++;
    }

    struct Value final_val; final_val.type = VAL_FLOAT; final_val.f = 0.0;
    if (PFEval != NULL) final_val = popV(&PFEval);
    
    return final_val;
}


int priority(char Op)
{
    if (Op == '$')
    {
        return 7;
    }
    else if (Op == '*' || Op == '/' || Op == '%')
    {
        return 6;
    }
    else if (Op == '+' || Op == '-')
    {
        return 5;
    }
    else if (Op == '>' || Op == '<' || Op == '?' || Op == '^')
        return 4;
    else if (Op == '~' || Op == '!' || Op == '#' || Op == '@')
        return 4;
    else if (Op == '&')
    {
        return 2;
    }
    else if (Op == '|')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
