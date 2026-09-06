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


void resolve_complex_types(char *expr) {
    char temp[1000];
    strcpy(temp, expr);
    
    char tokens[100][100];
    int tokenCount = 0;
    
    char *token = strtok(temp, " \t\n");
    while (token != NULL) {
        strcpy(tokens[tokenCount++], token);
        token = strtok(NULL, " \t\n");
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
                    char *ptr = argExpr;
                    char t[100];
                    char single[1000] = "";
                    int pDepth2 = 0;
                    while (sscanf(ptr, "%s", t) == 1) {
                        ptr = strstr(ptr, t) + strlen(t);
                        if (!strcmp(t, "(")) pDepth2++;
                        else if (!strcmp(t, ")")) pDepth2--;
                        if (pDepth2 == 0 && !strcmp(t, ",")) {
                            strcpy(rawArgs[rArgCount++], single);
                            single[0] = '\0';
                        } else {
                            strcat(single, t);
                            strcat(single, " ");
                        }
                    }
                    if (strlen(single) > 0) strcpy(rawArgs[rArgCount++], single);
                    
                    struct Value res = bfunc->handler(rawArgs, rArgCount);
                    sprintf(tokens[i], "%f", res.f);
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
                        
                        char *ptr = argExpr;
                        char t[100];
                        while (sscanf(ptr, "%s", t) == 1) {
                            ptr = strstr(ptr, t) + strlen(t);
                            
                            if (!strcmp(t, "(")) pDepth++;
                            else if (!strcmp(t, ")")) pDepth--;
                            
                            if (pDepth == 0 && !strcmp(t, ",")) {
                                strcat(singleArg, ";");
                                argVals[argCount++] = val(singleArg);
                                singleArg[0] = '\0';
                            } else {
                                strcat(singleArg, t);
                                strcat(singleArg, " ");
                            }
                        }
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
                            appendL_copy(&envStack[envTop].LV, func->params[p].name, (struct lV*)argVals[p].ptr);
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
                    
                     
                    char resStr[100];
                    sprintf(resStr, "%f", retVal.f);
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
                int brackets = 1;
                int j = i + 2;
                char argExpr[1000] = "";
                while (j < tokenCount && brackets > 0) {
                    if (strcmp(tokens[j], "[") == 0) brackets++;
                    if (strcmp(tokens[j], "]") == 0) brackets--;
                    if (brackets > 0) {
                        strcat(argExpr, tokens[j]);
                        strcat(argExpr, " ");
                    }
                    j++;
                }
                strcat(argExpr, ";");
                float res = 0.0;
                if (type == 6) { 
                    res = (float)getIA(envStack[envTop].IA, tokens[i], (int)val(argExpr).f);
                } else if (type == 7) { 
                    res = getFA(envStack[envTop].FA, tokens[i], (int)val(argExpr).f);
                } else if (type == 10) { 
                    res = getL(envStack[envTop].LV, tokens[i], (int)val(argExpr).f);
                } else if (type == 8) { 
                    res = (float)getCA(envStack[envTop].CA, tokens[i], (int)val(argExpr).f);
                } else if (type == 9) { 
                    res = (float)getBA(envStack[envTop].BA, tokens[i], (int)val(argExpr).f);
                } else if (type == 11) { 
                    char key[100] = "";
                    struct Value kV = val(argExpr);
                    if (kV.type == 1) {  
                        strcpy(key, kV.s);
                    } else {
                        sscanf(argExpr, "%s", key);
                        if (key[0] == '"') {
                            key[strlen(key)-1] = '\0';
                            memmove(key, key+1, strlen(key));
                        }
                    }
                    res = getM_F(envStack[envTop].MV, tokens[i], key);
                }
                char resStr[100];
                sprintf(resStr, "%f", res);
                strcpy(tokens[i], resStr);
                int shift = j - (i + 1);
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
    while( (*buff != ';' || in_quotes) && *buff != '\n' && *buff != '\0')
    {
        if (*buff == '"') { in_quotes = !in_quotes; }
        
        if(*buff == '(' && !in_quotes)
        {
            push( &Stack, *buff);
            buff++;
        }
        else if (*buff == '=' && *(buff+1) == '=' && !in_quotes) { *buff = '#'; strcpy(buff+1, buff+2); continue; }
        else if( (*buff == ' ' || *buff == '\t' || *buff == '\r' || *buff == '=') && !in_quotes)
        {
            strcpy(buff, buff+1);
            continue;
        }
        else if (*buff == '!' && *(buff+1) == '=' && !in_quotes) { *buff = '@'; strcpy(buff+1, buff+2); continue; }
        else if (*buff == '<' && *(buff+1) == '=' && !in_quotes) { *buff = '^'; strcpy(buff+1, buff+2); continue; }
        else if (*buff == '>' && *(buff+1) == '=' && !in_quotes) { *buff = '?'; strcpy(buff+1, buff+2); continue; }
        else if( (*buff == '$' || *buff == '*' || *buff == '/' || *buff == '%' || *buff == '+' || *buff == '-' || *buff == '<' || *buff == '>' || *buff == '&' || *buff == '|' || *buff == '~' || *buff == '!' || *buff == '#' || *buff == '@' || *buff == '^' || *buff == '?') && !in_quotes)
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
        }
        else
        {
            *pf= *buff;
            pf++;

            *f= *buff;
            *f++;

            buff++;
        }
    }

    while ( Stack != NULL)
	{
        *pf= ' ';
        pf++ ;
		opr= pop( &Stack);
		*pf= opr;
        pf++;
	}

    *pf = ' '; pf++; *pf = '\0';

    pf= &postfix[0];
    int eval_in_quotes = 0;
    while(*pf != '\0')
    {
        if (*pf == '"') { eval_in_quotes = !eval_in_quotes; }
        if(*pf == ' ' && !eval_in_quotes)
        {
            *t= '\0';
            
            if (strlen(temp) > 0) {
                if (!strcmp(temp, "true")) { { struct Value v; v.type=VAL_FLOAT; v.f=1.0; pushV(v, &PFEval); } }
                else if (!strcmp(temp, "false")) { { struct Value v; v.type=VAL_FLOAT; v.f=0.0; pushV(v, &PFEval); } }
                else {
                    c= typeFetcher(temp);
                    if (c >= 1 && c <= 11) { pushV(valFetcherGlobal(temp), &PFEval); }
                    else if (temp[0] == '"') {
                        struct Value v; v.type = VAL_STRING;
                        strcpy(v.s, temp + 1);
                        if (strlen(v.s) > 0 && v.s[strlen(v.s)-1] == '"') v.s[strlen(v.s)-1] = '\0';
                        pushV(v, &PFEval);
                    }
                    else if(isdigit(temp[0]) || (temp[0] == '-' && isdigit(temp[1]))) { x= atof(temp);  { struct Value v; v.type=VAL_FLOAT; v.f=x; pushV(v, &PFEval); } }
                }
            }
            t= &temp[0];
            *t = '\0';
        }
        else if (priority(*pf) && !eval_in_quotes)
        {
            n1= popV(&PFEval);
            n2= popV(&PFEval);
            tot.type = VAL_FLOAT;
            tot.f = 0.0;
             
            if (n1.type >= VAL_INT_ARRAY || n2.type >= VAL_INT_ARRAY) {
                printf("Error: Syntax error on complex type arithmetic (n1.type=%d, n2.type=%d, op=%c)\n", n1.type, n2.type, *pf);
                tot.type = VAL_ERROR;
                tot.f = 0.0;
                pushV(tot, &PFEval);
                pf++;
                continue;
            }
            switch(*pf)
            {
                case '$': tot.f= pow(n2.f,n1.f); break;
                case '%': tot.f= remainder(n2.f,n1.f); break;
                case '/': tot.f= n2.f/n1.f; break;
                case '*': tot.f= n2.f*n1.f; break;
                case '+': tot.f= n2.f+n1.f; break;
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
        return 5;
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