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

void resolve_complex_types(char *expr) {
    char temp[1000];
    strcpy(temp, expr);
    
    char tokens[100][100];
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
        } else {
            char tok[100];
            int idx = 0;
            while (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '"' && *p != '[' && *p != ']' && idx < 95) tok[idx++] = *p++;
            tok[idx] = '\0';
            strcpy(tokens[tokenCount++], tok);
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
                    
                     
                    envTop++;
                    envStack[envTop].IV = NULL;
                    envStack[envTop].FV = NULL;
                    envStack[envTop].CV = NULL;
                    envStack[envTop].BV = NULL;
                    envStack[envTop].IA = NULL;
                    envStack[envTop].FA = NULL;
                    envStack[envTop].CA = NULL;
                    envStack[envTop].BA = NULL;
                    envStack[envTop].LV = NULL;
                    
                     
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
			while ( ( opr ) != '(' )
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
                else {
                    c= typeFetcher(temp);
                    if (c >= 1 && c <= 11) { pushV(valFetcherGlobal(temp), &PFEval); }
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
