#include <stdio.h>
extern FILE *current_fp;
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "Keywords.h"
#include "VarLL.h"
#include "Builtins.h"

float run_interpreter_loop(FILE *fp);

#include "ExprEvalStack.h"

struct node *Stack;
struct node2 *PFEval;

float val(char *);
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
    
    // Process tokens
    for (int i = 0; i < tokenCount; i++) {
        // Function evaluation
        if (i < tokenCount - 1 && strcmp(tokens[i+1], "(") == 0) {
            struct funcDef *func = findFunc(FN, tokens[i]);
            struct BuiltinFunc *bfunc = findBuiltin(tokens[i]);
            if (func != NULL || bfunc != NULL) {
                // Find closing )
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
                    
                    float res = bfunc->handler(rawArgs, rArgCount);
                    sprintf(tokens[i], "%f", res);
                    int shift = j - (i + 1);
                    for (int k = i + 1; k < tokenCount - shift; k++) {
                        strcpy(tokens[k], tokens[k + shift]);
                    }
                    tokenCount -= shift;
                } else if (func != NULL) {
                    // Evaluate arguments
                    float argVals[10];
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
                    
                    // Set up call frame
                    if (callStackTop >= 999) {
                        printf("Error: Stack Overflow\n");
                        return;
                    }
                    callStackTop++;
                    struct callFrame *frame = &callStack[callStackTop];
                    frame->saved_envTop = envTop;
                    frame->hasReturn = 0;
                    frame->returnValue = 0.0;
                    
                    // Setup new environment
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
                    
                    // Assign arguments to params
                    for (int p = 0; p < argCount && p < func->paramCount; p++) {
                        if (func->params[p].type == 1) {
                            appendI(&envStack[envTop].IV, func->params[p].name, (int)argVals[p]);
                        } else if (func->params[p].type == 2) {
                            appendF(&envStack[envTop].FV, func->params[p].name, argVals[p]);
                        }
                    }
                    
                    
    // Execute function body
                    long savedPos = ftell(current_fp);
                    fseek(current_fp, func->bodyPos, SEEK_SET);
                    
                    float retVal = run_interpreter_loop(current_fp);
    
                    
                    // Restore state
                    fseek(current_fp, savedPos, SEEK_SET);
                    while (envTop > frame->saved_envTop) {
                        envTop--;
                    }
                    callStackTop--;
                    
                    // Replace tokens with result
                    char resStr[100];
                    sprintf(resStr, "%f", retVal);
                    strcpy(tokens[i], resStr);
                    
                    // Shift tokens
                    int shift = j - (i + 1);
                    for (int k = i + 1; k < tokenCount - shift; k++) {
                        strcpy(tokens[k], tokens[k + shift]);
                    }
                    tokenCount -= shift;
                }
        }
    }
}
    
    // Array and Map resolution
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
                    res = (float)getIA(envStack[envTop].IA, tokens[i], (int)val(argExpr));
                } else if (type == 7) { 
                    res = getFA(envStack[envTop].FA, tokens[i], (int)val(argExpr));
                } else if (type == 11) { 
                    char key[100] = "";
                    sscanf(argExpr, "%s", key);
                    if (key[0] == '"') {
                        key[strlen(key)-1] = '\0';
                        memmove(key, key+1, strlen(key));
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
        // Rebuild expr
        expr[0] = '\0';
        for (int i = 0; i < tokenCount; i++) {
            if (i > 0) strcat(expr, " ");
            strcat(expr, tokens[i]);
        }
    }
}

float val( char *buff)
{
    Stack = NULL;
    PFEval = NULL;
    resolve_complex_types(buff);

    int c;
    float tot;
    
    char postfix[1000]="", *pf, opr;
    pf= &postfix[0];

    char *f, func[100];
    f= &func[0];

    char *t, temp[100];
    t= &temp[0];

    float x, n1, n2;

    // removed buff skip

    while( *buff != ';' && *buff != '\n' && *buff != '\0')
    {
        if(*buff == '(')
        {
            push( &Stack, *buff);
            buff++;
        }
        // removed = skip
        else if( *buff == ' ' || *buff == '\t')
        {
            if (pf > &postfix[0] && *(pf-1) != ' ')
            {
                *pf= ' ';
                pf++;
            }
            buff++;
            continue;
        }
        else if (*buff == '=' && *(buff+1) == '=') { *buff = 'E'; strcpy(buff+1, buff+2); continue; }
        else if (*buff == '!' && *(buff+1) == '=') { *buff = 'N'; strcpy(buff+1, buff+2); continue; }
        else if (*buff == '<' && *(buff+1) == '=') { *buff = 'L'; strcpy(buff+1, buff+2); continue; }
        else if (*buff == '>' && *(buff+1) == '=') { *buff = 'G'; strcpy(buff+1, buff+2); continue; }
        else if( *buff == '$' || *buff == '*' || *buff == '/' || *buff == '%' || *buff == '+' || *buff == '-' || *buff == '<' || *buff == '>' || *buff == '&' || *buff == '|' || *buff == '~' || *buff == '!' || *buff == 'E' || *buff == 'N' || *buff == 'L' || *buff == 'G')
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
            if( *buff == ' ' || *buff == '\t')
                buff++;
        }
        else if( *buff == ')')
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
    

    while(*pf != '\0')
    {
        if(*pf == ' ')
        {
            *t= '\0';
            
            if (strlen(temp) > 0) {
                if (!strcmp(temp, "true")) { pushF(1.0, &PFEval); }
                else if (!strcmp(temp, "false")) { pushF(0.0, &PFEval); }
                else {
                    c= typeFetcher(temp);
                    
                    if (c == 1) { x= valFetcherGlobal(temp); pushF( x, &PFEval); }
                    else if (c == 2) { x= valFetcherGlobal(temp); pushF( x, &PFEval); }
                    else if(c == 3) { x= valFetcherGlobal(temp); pushF( x, &PFEval); }
                    else if(isdigit(temp[0])) { x= atof(temp);  pushF( x, &PFEval); }
                }
            }
            t= &temp[0];
            *t = '\0';
        }
        else if (priority(*pf))
        {
            n1= popF(&PFEval);
            n2= popF(&PFEval);
            switch(*pf)
            {
                case '$': tot= pow(n2,n1); break;
                case '%': tot= remainder(n2,n1); break;
                case '/': tot= n2/n1; break;
                case '*': tot= n2*n1; break;
                case '+': tot= n2+n1; break;
                case '-': tot= n2-n1; break;
                case '<': tot= n2<n1; break;
                case '>': tot= n2>n1; break;
                case '&': tot= n2&&n1; break;
                case '|': tot= n2||n1; break;
                case '~': tot= n2==n1; break;
                case '!': tot= n2!=n1; break;
                case 'L': tot= n2<=n1; break;
                case 'G': tot= n2>=n1; break;
                case 'E': tot= n2==n1; break;
                case 'N': tot= n2!=n1; break;
            }
            pushF( tot, &PFEval);
        }
        else
        {
            *t= *pf;
            t++;
        }
        pf++;
    }

    float final_val = 0.0;
    if (PFEval != NULL) final_val = popF(&PFEval);
    
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
    else if (Op == '>' || Op == '<' || Op == 'G' || Op == 'L')
    {
        return 4;
    }
    else if (Op == '~' || Op == '!' || Op == 'E' || Op == 'N')
    {
        return 3;
    }
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