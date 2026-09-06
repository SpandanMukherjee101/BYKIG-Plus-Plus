#include <stdio.h>

#include <stdlib.h>
void safe_fscanf(FILE* fp, const char* fmt, char* buf) {
    if (fscanf(fp, fmt, buf) <= 0) {
        printf("Error: Unexpected EOF or read error.\n");
        fflush(stdout);
        exit(1);
    }
}

#include <stdlib.h>
#include <string.h>

#include "ArithmaticExprEval.h"



struct callFrame callStack[10000];
int callStackTop = -1;
struct callFrame envStack[10000];
int envTop = 0;
struct funcDef *FN = NULL;
FILE *current_fp = NULL;
struct Value global_return_val;
FILE *global_fp = NULL;

struct Value run_interpreter_loop(FILE *fp, const char *current_filepath) {

    char buffer[1000], tempS[1000], name[100], tD[100], dataC, ch;
    int mode, dataI, i, j, type, c[1000] = {0}, k, p= -1, b1, b2, w[1000] = {0}, l= 0;
    float dataF;

    int break_loop = 0;
    while (1)
    {
        if (break_loop) break;
        
        
        int ret = fscanf(fp, "%s", buffer); if (ret == EOF) break;
        
        if (buffer[0] == '/' && buffer[1] == '/') {
            fgets(buffer, 1000, fp);
            continue;
        }


        if (!(strcmp(buffer, ".")))
        {
            
            break;
        }

        mode= 0;

        mode= whichone(buffer);
        switch (mode)
        {
        
        case 1:
        
        startI:
            safe_fscanf(fp, "%s", buffer); strcpy(name, buffer);
            dataI=0;

            safe_fscanf(fp, "%s", buffer); if (!(strcmp(buffer,",")))
            {
                appendI( &envStack[envTop].IV, name, dataI);
                goto startI;
            }
            else if (!(strcmp(buffer,"=")))
            {
                char exprBuf[1000] = "";
                int pDepth = 0;
                while (1) {
                    safe_fscanf(fp, "%s", tD);
                    if (!strcmp(tD, ".")) { goto ERROR; }
                      if (!strcmp(tD, "(")) pDepth++;
                    else if (!strcmp(tD, ")")) pDepth--;
                    
                    if (pDepth == 0 && (!strcmp(tD, ",") || !strcmp(tD, ";"))) {
                        strcpy(buffer, tD);
                        break;
                    }
                    strcat(exprBuf, tD);
                    strcat(exprBuf, " ");
                }
                strcat(exprBuf, ";");
                dataI = (int)val(exprBuf).f;
                
                // Do NOT call safe_fscanf(fp, "%s", buffer) here because the while loop
                // already put the trailing ',' or ';' into buffer!
                
                if (!(strcmp(buffer,",")))
                {
                    appendI( &envStack[envTop].IV, name, dataI);
                    goto startI;
                }
                else if (!(strcmp(buffer,";")))
                {
                    appendI( &envStack[envTop].IV, name, dataI);
                }
            }
            else if (!(strcmp(buffer,";")))
            {
                appendI( &envStack[envTop].IV, name, dataI);
            }

            break;

        case 2:
        
        startF:
            safe_fscanf(fp, "%s", buffer); strcpy(name, buffer);
            dataF=0;

            safe_fscanf(fp, "%s", buffer); if (!(strcmp(buffer,",")))
            {
                appendF( &envStack[envTop].FV, name, dataF);
                goto startF;
            }
            else if (!(strcmp(buffer,"=")))
            {
                char exprBuf[1000] = "";
                int pDepth = 0;
                while (1) {
                    safe_fscanf(fp, "%s", tD);
                    if (!strcmp(tD, ".")) { goto ERROR; }
                      if (!strcmp(tD, "(")) pDepth++;
                    else if (!strcmp(tD, ")")) pDepth--;
                    
                    if (pDepth == 0 && (!strcmp(tD, ",") || !strcmp(tD, ";"))) {
                        strcpy(buffer, tD);
                        break;
                    }
                    strcat(exprBuf, tD);
                    strcat(exprBuf, " ");
                }
                strcat(exprBuf, ";");
                dataF = (float)val(exprBuf).f;
                
                // Do NOT call safe_fscanf(fp, "%s", buffer) here because the while loop
                // already put the trailing ',' or ';' into buffer!
                
                if (!(strcmp(buffer,",")))
                {
                    appendF( &envStack[envTop].FV, name, dataF);
                    goto startF;
                }
                else if (!(strcmp(buffer,";")))
                {
                    appendF( &envStack[envTop].FV, name, dataF);
                }
            }
            else if (!(strcmp(buffer,";")))
            {
                appendF( &envStack[envTop].FV, name, dataF);
            }

            break;
        
        case 3:
        
        startC:
            safe_fscanf(fp, "%s", buffer); strcpy(name, buffer);
            dataC= '\0';

            safe_fscanf(fp, "%s", buffer); if (!(strcmp(buffer,",")))
            {
                appendC( &envStack[envTop].CV, name, dataC);
                goto startC;
            }
            else if (!(strcmp(buffer,"=")))
            {
                safe_fscanf(fp, "%s", tD);
                dataC = tD[1];

                safe_fscanf(fp, "%s", buffer); if (!(strcmp(buffer,",")))
                {
                    appendC( &envStack[envTop].CV, name, dataC);
                    goto startC;
                }
                else if (!(strcmp(buffer,";")))
                {
                    appendC( &envStack[envTop].CV, name, dataC);
                }
            }
            else if (!(strcmp(buffer,";")))
            {
                appendC( &envStack[envTop].CV, name, dataC);
            }

            break;

        case 4:

            fgets(buffer, 1000, fp);
            
            p++;
            k= p-1;
           
            dataF = val(buffer).f;
            if (dataF)
            {
                fgets(buffer, 1000, fp);
                c[p]= 1;
            }
            else
            {
                fgets(buffer, 1000, fp);
                while (k!=p)
                {
                    fgets(buffer, 1000, fp);

                    b1= 0;
                    b2= 0;
                    for(int i= 0; i<strlen(buffer); i++)
                    {
                        if(buffer[i]=='{')
                        {
                            b1= 1;
                            break;
                        }
                    }
                    for(int i= 0; i<strlen(buffer); i++)
                    {
                        if(buffer[i]=='}')
                        {
                            b2= 1;
                            break;
                        }
                    }

                    if (b1)
                        p++;
                    else if (b2)
                        p--;
                }

                safe_fscanf(fp, "%s", buffer); if(strcmp(buffer, "elif") && strcmp(buffer, "else"))
                {
                    c[++p]=0;
                    --p;
                    fseek( fp, -strlen(buffer), SEEK_CUR);
                }
                else
                {
                    fseek( fp, -strlen(buffer), SEEK_CUR);
                }
            }

            break;

        case 5:

            fgets(buffer, 1000, fp);
           
            p++;
            k= p-1;
            
            dataF = val(buffer).f;
            if (dataF && c[p]!=1)
            {
                fgets(buffer, 1000, fp);
                c[p]= 1;
            }
            else
            {
                fgets(buffer, 1000, fp);
                while (k!=p)
                {
                
                    fgets(buffer, 1000, fp);

                    b1= 0;
                    b2= 0;
                    for(int i= 0; i<strlen(buffer); i++)
                    {
                        if(buffer[i]=='{')
                        {
                            b1= 1;
                            break;
                        }
                    }
                    for(int i= 0; i<strlen(buffer); i++)
                    {
                        if(buffer[i]=='}')
                        {
                            b2= 1;
                            break;
                        }
                    }

                    if (b1)
                        p++;
                    else if (b2)
                        p--;
                }

                safe_fscanf(fp, "%s", buffer); if(strcmp(buffer, "elif") && strcmp(buffer, "else"))
                {
                    c[++p]=0;
                    --p;
                    fseek( fp, -strlen(buffer), SEEK_CUR);
                }
                else
                {
                    fseek( fp, -strlen(buffer), SEEK_CUR);
                }
            }

            break;

        case 6:
            
            fgets(buffer, 1000, fp);
            
            p++;
            k= p-1;
            
            if (c[p]!= 1)
            {
                fgets(buffer, 1000, fp);
                c[p]= 1;
            }
            else
            {
                fgets(buffer, 1000, fp);
                while (k!=p)
                {
                
                    fgets(buffer, 1000, fp);

                    b1= 0;
                    b2= 0;
                    for(int i= 0; i<strlen(buffer); i++)
                    {
                        if(buffer[i]=='{')
                        {
                            b1= 1;
                            break;
                        }
                    }
                    for(int i= 0; i<strlen(buffer); i++)
                    {
                        if(buffer[i]=='}')
                        {
                            b2= 1;
                            break;
                        }
                    }

                    if (b1)
                        p++;
                    else if (b2)
                        p--;
                }

                safe_fscanf(fp, "%s", buffer); if(strcmp(buffer, "elif") && strcmp(buffer, "else"))
                {
                    c[++p]=0;
                    --p;
                    fseek( fp, -strlen(buffer), SEEK_CUR);
                }
                else
                {
                    fseek( fp, -strlen(buffer), SEEK_CUR);
                }
            }

            break;

        case 7:
            if (p < 0) {
                break_loop = 1;
                break;
            }
        
            safe_fscanf(fp, "%s", buffer); if(strcmp(buffer, "elif") && strcmp(buffer, "else") && c[p]!= 2)
            {
                c[p]=0;
                k= --p;
                fseek( fp, -strlen(buffer), SEEK_CUR);
            }
            else
            {
                if (c[p]==2)
                {
                    fseek( fp, w[p], SEEK_SET);
                    w[p]= 0;
                    --p;
                    k= p;
                }
                else
                {
                    k= --p;
                    fseek( fp, -strlen(buffer), SEEK_CUR);
                }
            }

            break;

        case 8:

            p++;
            k= p-1;
            w[p]=ftell( fp)-5;

            fgets(buffer, 1000, fp);
            dataF = val(buffer).f;
            fgets(buffer, 1000, fp);
            
            if (dataF)
            {
                c[p]= 2;
            }
            else
            {
                while (k!=p)
                {
                    fgets(buffer, 1000, fp);

                    b1= 0;
                    b2= 0;

                    for(int i= 0; i<strlen(buffer); i++)
                    {
                        if(buffer[i]=='{')
                        {
                            b1= 1;
                            break;
                        }
                    }
                    
                    for(int i= 0; i<strlen(buffer); i++)
                    {
                        if(buffer[i]=='}')
                        {
                            b2= 1;
                            break;
                        }
                    }

                    if (b1)
                        p++;
                    else if (b2)
                        p--;
                }

                c[++p]=0;
                --p;
            }
            
            break;

        case 9:
        
            fgets( buffer, 1000000, fp);

            j=1;

            while (1)
            {           
                if(buffer[j]=='\"')
                {
                    j++;
                    for (i = 0; !(buffer[j]=='\"' && (buffer[j+1]!='%' && (buffer[j+2]==';' ||  buffer[j+2]=='+'))); i++)
                    {
                        if (buffer[j]=='%' && (buffer[j+1]=='+' || buffer[j+1]==';'))
                        {
                            j++;
                            i--;
                            continue;
                        }
                        else if(buffer[j]=='%' && buffer[j+1]=='n')
                        {
                            tempS[i]='\n';
                            j+=2;
                            continue;
                        }
                        
                        
                        tempS[i]= buffer[j];
                        j++;
                    }
                    
                    tempS[i]='\0';
                    j+=2;
                    printf("%s", tempS);
                    fflush(stdout);
                    
                    for(int t = 0; t < i; t++)
                        tempS[t]= '\0';
                }
                else if (buffer[j]=='+')
                {
                    j+=2;
                }
                else if(buffer[j]!=';')
                {
                    for( i=0; !(buffer[j+1]==';' ||  buffer[j+1]=='+'); i++)
                    {
                        tempS[i]= buffer[j];
                        j++;
                    }
                    
                    tempS[i]= '\0';
                    j++;
                    
                    type= typeFetcher(tempS);

                    if (type==1)
                    {
                        dataI= (int)valFetcherGlobal(tempS).f;
                        printf("%d", dataI);
                    }
                    else if (type==2)
                    {
                        dataF= valFetcherGlobal(tempS).f;
                        printf("%f", dataF);
                    }
                    else if (type==3)
                    {
                        dataC= (char)valFetcherGlobal(tempS).f;
                        printf("%c", dataC);
                    }
                    else if (type==4)
                    {
                        dataI= (int)valFetcherGlobal(tempS).f;
                        printf("%s", dataI ? "true" : "false");
                    }
                    else if (type==5)
                    {
                        char *strData = scopeGetSV(tempS);
                        if (strData) printf("%s", strData);
                        else printf("\nVariable not found!!!\n");
                    }
                    else
                    {
                        // Expression evaluation: handles array/list/map indexing, function calls, etc.
                        char evalBuf[1000];
                        int tLen = strlen(tempS);
                        while (tLen > 0 && (tempS[tLen-1] == ' ' || tempS[tLen-1] == '\t' || tempS[tLen-1] == '\r')) {
                            tempS[--tLen] = '\0';
                        }
                        sprintf(evalBuf, "%s ;", tempS);
                        struct Value result = val(evalBuf);
                        if (result.type == VAL_STRING) {
                            printf("%s", result.s);
                        } else {
                            printf("%f", result.f);
                        }
                    }

                    for(int t = 0; t < i; t++)
                        tempS[t]= '\0';
                }
                if (buffer[j]==';')
                {
                    break;
                }
            }

            break;
        
        case 10:

        IN:

            safe_fscanf(fp, "%s", buffer); type= typeFetcher(buffer);
            
            if (type == 0)
            {
                goto ERROR;
            }

            if (type == 1)
            {
                scanf("%d",&dataI);
                appendI( &envStack[envTop].IV, buffer, dataI);
            }
            else if (type == 2)
            {
                scanf("%f",&dataF);
                appendF( &envStack[envTop].FV, buffer, dataF);
            }
            else if (type == 3)
            {
                scanf("%c",&dataC);
                appendC( &envStack[envTop].CV, buffer, dataC);
            }
            else if (type == 5)
            {
                char tempS[1000];
                if (scanf("%s", tempS) <= 0) exit(1);
                appendS(&envStack[envTop].SV, buffer, tempS);
            }
            else if (type >= 6 && type <= 11)
            {
                char nameBuf[100]; strcpy(nameBuf, buffer);
                safe_fscanf(fp, "%s", buffer);
                safe_fscanf(fp, "%s", buffer);
                
                char keyStr[1000] = "";
                int idx = 0;
                
                if (type == 11) {
                    struct Value kV = val(buffer);
                    if (kV.type == 1) { // VAL_STRING
                        strcpy(keyStr, kV.s);
                    } else {
                        strcpy(keyStr, buffer);
                        if (keyStr[0] == '"') {
                            keyStr[strlen(keyStr)-1] = '\0';
                            memmove(keyStr, keyStr+1, strlen(keyStr));
                        }
                    }
                } else {
                    idx = (int)val(buffer).f;
                }
                
                safe_fscanf(fp, "%s", buffer);
                
                if (type == 6) {
                    int valI; if (scanf("%d", &valI) <= 0) exit(1);
                    setIA(envStack[envTop].IA, nameBuf, idx, valI);
                } else if (type == 7) {
                    float valF; if (scanf("%f", &valF) <= 0) exit(1);
                    setFA(envStack[envTop].FA, nameBuf, idx, valF);
                } else if (type == 8) {
                    char valC; if (scanf(" %c", &valC) <= 0) exit(1);
                    setCA(envStack[envTop].CA, nameBuf, idx, valC);
                } else if (type == 9) {
                    int valI; if (scanf("%d", &valI) <= 0) exit(1);
                    setBA(envStack[envTop].BA, nameBuf, idx, valI);
                } else if (type == 10) {
                    float valF; if (scanf("%f", &valF) <= 0) exit(1);
                    setL(envStack[envTop].LV, nameBuf, idx, valF);
                } else if (type == 11) {
                    float valF; if (scanf("%f", &valF) <= 0) exit(1);
                    setM_F(envStack[envTop].MV, nameBuf, keyStr, valF);
                }
            }

            safe_fscanf(fp, "%s", buffer); if(!(strcmp( buffer, "+")))
                goto IN;
            break;
        
        case 11:
            
            type= typeFetcher(buffer);
            
            if (type == 0)
            {
                if (findFunc(FN, buffer) != NULL || (!strcmp(buffer, "push")) || (!strcmp(buffer, "pop")) || (!strcmp(buffer, "delete")) || (!strcmp(buffer, "exists")) || (!strcmp(buffer, "len")) || (!strcmp(buffer, "file_write")) || (!strcmp(buffer, "file_append")) || (!strcmp(buffer, "file_read")) || (!strcmp(buffer, "file_exists")) || (!strcmp(buffer, "file_scan_open")) || (!strcmp(buffer, "file_scan_next")) || (!strcmp(buffer, "file_scan_close")) || (!strcmp(buffer, "file_scan_get_pos")) || (!strcmp(buffer, "file_scan_set_pos"))) {
                    char exprBuf[1000] = "";
                    strcpy(exprBuf, buffer);
                    strcat(exprBuf, " ");
                    int pDepth = 0;
                    char tD[100];
                    while (1) {
                        safe_fscanf(fp, "%s", tD);
                        if (!strcmp(tD, ".")) { goto ERROR; }
                      if (!strcmp(tD, ".")) { goto ERROR; }
                      strcat(exprBuf, tD);
                        strcat(exprBuf, " ");
                        if (!strcmp(tD, "(")) pDepth++;
                        else if (!strcmp(tD, ")")) pDepth--;
                        if (pDepth == 0 && (!strcmp(tD, ";") || strchr(tD, ';'))) break;
                    }
                    val(exprBuf);
                    break;
                }
                goto ERROR;
            }            
            DO_ASSIGNMENT:
            strcpy(name, buffer);

            if (type >= 6 && type <= 11) { // Arrays, Lists, Maps
                safe_fscanf(fp, "%s", buffer); safe_fscanf(fp, "%s", buffer); int idx = 0;
                char keyStr[1000] = "";
                if (type == 11) {
                    struct Value kV = val(buffer);
                    if (kV.type == 1) { // VAL_STRING
                        strcpy(keyStr, kV.s);
                    } else {
                        strcpy(keyStr, buffer);
                        if (keyStr[0] == '"') {
                            keyStr[strlen(keyStr)-1] = '\0';
                            memmove(keyStr, keyStr+1, strlen(keyStr));
                        }
                    }
                } else {
                    idx = (int)val(buffer).f; // read index
                }
                
                safe_fscanf(fp, "%s", buffer); safe_fscanf(fp, "%s", buffer); char exprBuf[1000] = "";
                int pDepth = 0;
                while (1) {
                    safe_fscanf(fp, "%s", tD);
                    if (!strcmp(tD, ".")) { goto ERROR; }
                      if (!strcmp(tD, "(")) pDepth++;
                    else if (!strcmp(tD, ")")) pDepth--;
                    if (pDepth == 0 && (!strcmp(tD, ";"))) {
                        break;
                    }
                    strcat(exprBuf, tD);
                    strcat(exprBuf, " ");
                }
                strcat(exprBuf, ";");
                
                if (type == 6) setIA(envStack[envTop].IA, name, idx, (int)val(exprBuf).f);
                else if (type == 7) setFA(envStack[envTop].FA, name, idx, val(exprBuf).f);
                else if (type == 8) setCA(envStack[envTop].CA, name, idx, (char)val(exprBuf).f);
                else if (type == 9) setBA(envStack[envTop].BA, name, idx, (int)val(exprBuf).f);
                else if (type == 10) setL(envStack[envTop].LV, name, idx, val(exprBuf).f);
                else if (type == 11) setM_F(envStack[envTop].MV, name, keyStr, val(exprBuf).f);
            } else {
                fgets(buffer, 1000, fp);

                if (type == 1)
                {
                    dataI= (int)val(buffer).f;
                    appendI( &envStack[envTop].IV, name, dataI);
                }
                else if (type == 2)
                {
                    dataF = val(buffer).f;
                    appendF( &envStack[envTop].FV, name, dataF);
                }
                else if (type == 3) { appendC(&envStack[envTop].CV, name, (char)val(buffer).f); }
                else if (type == 4) { appendB(&envStack[envTop].BV, name, (int)val(buffer).f); }
                else if (type == 5) {
                    char *eq = strchr(buffer, '=');
                    if (eq) {
                        strcpy(builtin_string_return, "");
                        val(eq + 1);
                        if (strlen(builtin_string_return) > 0) {
                            appendS(&envStack[envTop].SV, name, builtin_string_return);
                        } else {
                            char *start = strchr(eq + 1, '"');
                            if (start) {
                                char *end = strrchr(start + 1, '"');
                                if (end) *end = '\0';
                                appendS(&envStack[envTop].SV, name, start + 1);
                            } else {
                                char *src = strtok(eq + 1, " \t\r\n;");
                                char *v = scopeGetSV(src);
                                if (v) appendS(&envStack[envTop].SV, name, v);
                            }
                        }
                    }
                }
            }
            break;

case 14:
        
        startB:
            safe_fscanf(fp, "%s", buffer); strcpy(name, buffer);
            dataI= 0;

            safe_fscanf(fp, "%s", buffer); if (!(strcmp(buffer,",")))
            {
                appendB( &envStack[envTop].BV, name, dataI);
                goto startB;
            }
            else if (!(strcmp(buffer,"=")))
            {
                safe_fscanf(fp, "%s", tD);
                if (!strcmp(tD, "true")) dataI = 1;
                else if (!strcmp(tD, "false")) dataI = 0;
                else dataI = atoi(tD);

                safe_fscanf(fp, "%s", buffer); if (!(strcmp(buffer,",")))
                {
                    appendB( &envStack[envTop].BV, name, dataI);
                    goto startB;
                }
                else if (!(strcmp(buffer,";")))
                {
                    appendB( &envStack[envTop].BV, name, dataI);
                }
            }
            else if (!(strcmp(buffer,";")))
            {
                appendB( &envStack[envTop].BV, name, dataI);
            }
        break;

case 15: // intarr
            safe_fscanf(fp, "%s", name);
            safe_fscanf(fp, "%s", buffer); if (!strcmp(buffer, "[")) {
                safe_fscanf(fp, "%s", buffer); int size = (int)val(buffer).f;
                safe_fscanf(fp, "%s", buffer); safe_fscanf(fp, "%s", buffer); appendIA(&envStack[envTop].IA, name, size);
                if (!strcmp(buffer, "=")) {
                    safe_fscanf(fp, "%s", buffer); int i = 0;
                    safe_fscanf(fp, "%s", buffer); while (strcmp(buffer, "}")) {
                        if (strcmp(buffer, ",")) {
                            setIA(envStack[envTop].IA, name, i++, (int)val(buffer).f);
                        }
                        safe_fscanf(fp, "%s", buffer); }
                    safe_fscanf(fp, "%s", buffer); }
            } else if (!strcmp(buffer, "=")) {
                safe_fscanf(fp, "%s", buffer); appendIA(&envStack[envTop].IA, name, 100); // default large capacity
                int i = 0;
                safe_fscanf(fp, "%s", buffer); while (strcmp(buffer, "}")) {
                    if (strcmp(buffer, ",")) {
                        setIA(envStack[envTop].IA, name, i++, (int)val(buffer).f);
                    }
                    safe_fscanf(fp, "%s", buffer); }
                safe_fscanf(fp, "%s", buffer); struct iA *arr = envStack[envTop].IA;
                while (arr) { if (!strcmp(arr->name, name)) { arr->size = i; break; } arr = arr->next; }
            }
            break;

        case 16: // floatarr
            safe_fscanf(fp, "%s", name);
            safe_fscanf(fp, "%s", buffer); if (!strcmp(buffer, "[")) {
                safe_fscanf(fp, "%s", buffer); int size = (int)val(buffer).f;
                safe_fscanf(fp, "%s", buffer); safe_fscanf(fp, "%s", buffer); appendFA(&envStack[envTop].FA, name, size);
                if (!strcmp(buffer, "=")) {
                    safe_fscanf(fp, "%s", buffer); int i = 0;
                    safe_fscanf(fp, "%s", buffer); while (strcmp(buffer, "}")) {
                        if (strcmp(buffer, ",")) {
                            setFA(envStack[envTop].FA, name, i++, val(buffer).f);
                        }
                        safe_fscanf(fp, "%s", buffer); }
                    safe_fscanf(fp, "%s", buffer); }
            } else if (!strcmp(buffer, "=")) {
                safe_fscanf(fp, "%s", buffer); appendFA(&envStack[envTop].FA, name, 100);
                int i = 0;
                safe_fscanf(fp, "%s", buffer); while (strcmp(buffer, "}")) {
                    if (strcmp(buffer, ",")) {
                        setFA(envStack[envTop].FA, name, i++, val(buffer).f);
                    }
                    safe_fscanf(fp, "%s", buffer); }
                safe_fscanf(fp, "%s", buffer); struct fA *arr = envStack[envTop].FA;
                while (arr) { if (!strcmp(arr->name, name)) { arr->size = i; break; } arr = arr->next; }
            }
            break;

        case 17: // chararr
            safe_fscanf(fp, "%s", name);
            safe_fscanf(fp, "%s", buffer); if (!strcmp(buffer, "[")) {
                safe_fscanf(fp, "%s", buffer); int size = (int)val(buffer).f;
                safe_fscanf(fp, "%s", buffer); safe_fscanf(fp, "%s", buffer); appendCA(&envStack[envTop].CA, name, size);
                if (!strcmp(buffer, "=")) {
                    safe_fscanf(fp, "%s", buffer); int i = 0;
                    safe_fscanf(fp, "%s", buffer); while (strcmp(buffer, "}")) {
                        if (strcmp(buffer, ",")) {
                            setCA(envStack[envTop].CA, name, i++, buffer[1] == '\'' ? buffer[1] : buffer[0]);
                        }
                        safe_fscanf(fp, "%s", buffer); }
                    safe_fscanf(fp, "%s", buffer); }
            } else if (!strcmp(buffer, "=")) {
                safe_fscanf(fp, "%s", buffer); appendCA(&envStack[envTop].CA, name, 100);
                int i = 0;
                safe_fscanf(fp, "%s", buffer); while (strcmp(buffer, "}")) {
                    if (strcmp(buffer, ",")) {
                        setCA(envStack[envTop].CA, name, i++, buffer[1] == '\'' ? buffer[1] : buffer[0]);
                    }
                    safe_fscanf(fp, "%s", buffer); }
                safe_fscanf(fp, "%s", buffer); struct cA *arr = envStack[envTop].CA;
                while (arr) { if (!strcmp(arr->name, name)) { arr->size = i; break; } arr = arr->next; }
            }
            break;

        case 18: // boolarr
            safe_fscanf(fp, "%s", name);
            safe_fscanf(fp, "%s", buffer); if (!strcmp(buffer, "[")) {
                safe_fscanf(fp, "%s", buffer); int size = (int)val(buffer).f;
                safe_fscanf(fp, "%s", buffer); safe_fscanf(fp, "%s", buffer); appendBA(&envStack[envTop].BA, name, size);
                if (!strcmp(buffer, "=")) {
                    safe_fscanf(fp, "%s", buffer); int i = 0;
                    safe_fscanf(fp, "%s", buffer); while (strcmp(buffer, "}")) {
                        if (strcmp(buffer, ",")) {
                            setBA(envStack[envTop].BA, name, i++, (int)val(buffer).f);
                        }
                        safe_fscanf(fp, "%s", buffer); }
                    safe_fscanf(fp, "%s", buffer); }
            } else if (!strcmp(buffer, "=")) {
                safe_fscanf(fp, "%s", buffer); appendBA(&envStack[envTop].BA, name, 100);
                int i = 0;
                safe_fscanf(fp, "%s", buffer); while (strcmp(buffer, "}")) {
                    if (strcmp(buffer, ",")) {
                        setBA(envStack[envTop].BA, name, i++, (int)val(buffer).f);
                    }
                    safe_fscanf(fp, "%s", buffer); }
                safe_fscanf(fp, "%s", buffer); struct bA *arr = envStack[envTop].BA;
                while (arr) { if (!strcmp(arr->name, name)) { arr->size = i; break; } arr = arr->next; }
            }
            break;

        case 19: // string
            safe_fscanf(fp, "%s", name);
            fgets(buffer, 1000, fp);
            char *start = strchr(buffer, '"');
            if (start) {
                char *end = strrchr(start + 1, '"');
                if (end) *end = '\0';
                scopeSetSV(name, start + 1);
            } else {
                char *eq = strchr(buffer, '=');
                if (eq) {
                    char *src = strtok(eq + 1, " \t\r\n;");
                    if (src) {
                        char *v = scopeGetSV(src);
                        if (v) scopeSetSV(name, v);
                    }
                } else {
                    scopeSetSV(name, "");
                }
            }
            break;

        case 20: // func
            {
                char funcName[100];
                safe_fscanf(fp, "%s", funcName);
                
                int isDuplicate = 0;
                if (findFunc(FN, funcName) != NULL) {
                    printf("Error: Duplicate function '%s'\\n", funcName);
                    isDuplicate = 1;
                }
                
                safe_fscanf(fp, "%s", buffer); int paramCount = 0;
                struct funcParam params[10];
                
                safe_fscanf(fp, "%s", buffer); while (strcmp(buffer, ")")) {
                    if (strcmp(buffer, ",")) {
                        int pType = 0;
                        if (!strcmp(buffer, "int")) pType = 1;
                        else if (!strcmp(buffer, "float")) pType = 2;
                        else if (!strcmp(buffer, "char")) pType = 3;
                        else if (!strcmp(buffer, "bool")) pType = 4;
                        else if (!strcmp(buffer, "string")) pType = 5;
                        else if (!strcmp(buffer, "intarr")) pType = 6;
                        else if (!strcmp(buffer, "floatarr")) pType = 7;
                        else if (!strcmp(buffer, "chararr")) pType = 8;
                        else if (!strcmp(buffer, "boolarr")) pType = 9;
                        else if (!strcmp(buffer, "list")) pType = 10;
                        else if (!strcmp(buffer, "map")) pType = 11;
                        
                        params[paramCount].type = pType;
                        safe_fscanf(fp, "%s", params[paramCount].name); // read param name
                        paramCount++;
                    }
                    safe_fscanf(fp, "%s", buffer); }
                
                safe_fscanf(fp, "%s", buffer); // read '{'
                long bodyPos = ftell(fp);
                if (!isDuplicate) {
                    char tempPath[1000] = "";
                    if (current_filepath != NULL) strcpy(tempPath, current_filepath);
                    if (appendFunc(&FN, funcName, paramCount, params, bodyPos, tempPath) != 0) {
                        printf("Error: Failed to register function '%s'\n", funcName);
                    }
                }
                
                // Skip the body
                b1 = 1; // we just read the first '{'
                b2 = 0;
                while (b1 != b2) {
                    safe_fscanf(fp, "%s", buffer); if (buffer[0] == '/' && buffer[1] == '/') {
                        fgets(buffer, 1000, fp);
                        continue;
                    }
                    if (!strcmp(buffer, "{")) b1++;
                    if (!strcmp(buffer, "}")) b2++;
                }
            }
            break;

        case 21: // return
            {
                struct Value retVal; retVal.type=VAL_FLOAT; retVal.f=0.0;
                char exprBuf[1000] = "";
                int pDepth = 0;
                char tD[100];
                while (1) {
                    safe_fscanf(fp, "%s", tD);
                    if (!strcmp(tD, ".")) { goto ERROR; }
                      if (!strcmp(tD, "(")) pDepth++;
                    else if (!strcmp(tD, ")")) pDepth--;
                    
                    if (pDepth == 0 && (!strcmp(tD, ";"))) {
                        strcpy(buffer, tD);
                        break;
                    }
                    strcat(exprBuf, tD);
                    strcat(exprBuf, " ");
                }
                if (strlen(exprBuf) > 0) {
                    strcat(exprBuf, ";");
                    retVal = val(exprBuf);
                }
                if (callStackTop >= 0) {
                    callStack[callStackTop].hasReturn = 1;
                    callStack[callStackTop].returnValue = retVal;
                }
                global_return_val = retVal;
                break_loop = 1;
                break;
            }
            break;

        case 22: // list
            {
                safe_fscanf(fp, "%s", name); // read list name
                appendL(&envStack[envTop].LV, name, 4); // initialize with capacity 4

                safe_fscanf(fp, "%s", buffer); if (!(strcmp(buffer, "="))) {
                    safe_fscanf(fp, "%s", buffer); while (strcmp(buffer, "}")) {
                        safe_fscanf(fp, "%s", buffer); if (!strcmp(buffer, "}")) break;
                        float d = val(buffer).f;
                        scopePushL(name, d);
                        safe_fscanf(fp, "%s", buffer); if (!strcmp(buffer, "}")) break;
                    }
                    safe_fscanf(fp, "%s", buffer); }
            }
            break;

        

        case 25: // break
            {
                int target_p = p;
                while (target_p >= 0 && c[target_p] != 2) {
                    target_p--;
                }
                if (target_p < 0) {
                    printf("Error: break outside of loop\n");
                    break;
                }
                
                int k = target_p - 1;
                while (k != p) {
                    fgets(buffer, 1000, fp);
                    b1 = 0; b2 = 0;
                    for(int i= 0; i<strlen(buffer); i++) {
                        if(buffer[i]=='{') { b1= 1; break; }
                    }
                    for(int i= 0; i<strlen(buffer); i++) {
                        if(buffer[i]=='}') { b2= 1; break; }
                    }
                    if (b1) p++;
                    else if (b2) p--;
                }
                c[++p] = 0;
                --p;
            }
            break;

        case 24: // use
            safe_fscanf(fp, "%s", buffer); {
                char *start = strchr(buffer, '"');
                if (start) {
                    char *end = strrchr(start + 1, '"');
                    if (end) *end = '\0';
                    char *filename = start + 1;
                    
                    FILE *mod_fp = fopen(filename, "rb");
                    char resolved_path[1024];
                    strcpy(resolved_path, filename);
                    
                    if (!mod_fp && current_filepath) {
                        char base_path[1024];
                        strcpy(base_path, current_filepath);
                        char *last_slash = strrchr(base_path, '/');
                        char *last_bslash = strrchr(base_path, '\\');
                        char *slash = (last_slash > last_bslash) ? last_slash : last_bslash;
                        if (slash) {
                            *(slash + 1) = '\0';
                            strcat(base_path, filename);
                            mod_fp = fopen(base_path, "rb");
                            if (mod_fp) strcpy(resolved_path, base_path);
                        }
                    }
                    
                    if (mod_fp) {
                        FILE *old_fp = current_fp;
                        current_fp = mod_fp;
                        run_interpreter_loop(mod_fp, resolved_path);
                        fclose(mod_fp);
                        current_fp = old_fp;
                    } else {
                        printf("Error: Could not open module '%s'\n", filename);
                    }
                }
            }
            safe_fscanf(fp, "%s", buffer); break;

        case 23: // map
            {
                safe_fscanf(fp, "%s", name); // read map name
                appendM(&envStack[envTop].MV, name); // initialize map
                safe_fscanf(fp, "%s", buffer); if (!(strcmp(buffer, "="))) {
                    safe_fscanf(fp, "%s", buffer); while (strcmp(buffer, "}")) {
                        safe_fscanf(fp, "%s", buffer); if (!strcmp(buffer, "}")) break;
                        char key[100]; strcpy(key, buffer);
                        if (key[0] == '"') {
                            key[strlen(key)-1] = '\0';
                            memmove(key, key+1, strlen(key));
                        }
                        safe_fscanf(fp, "%s", buffer); safe_fscanf(fp, "%s", buffer); float d = val(buffer).f;
                        setM_F(envStack[envTop].MV, name, key, d);
                        safe_fscanf(fp, "%s", buffer); if (!strcmp(buffer, "}")) break;
                    }
                    safe_fscanf(fp, "%s", buffer); }
            }
            break;

        default:
        
            ERROR:

                    printf("Neither variable nor keyword nor function: \"%s\" (eof=%d, err=%d, ftell=%ld)!!!\n", buffer, feof(fp), ferror(fp), ftell(fp));
                    fflush(stdout);
                    fgets( buffer, sizeof(buffer), fp);
                    break;
          }
      }
      return global_return_val;
  }
  
  int main(int argc, char const *argv[])
{
    char buffer[1000], tempS[1000], name[100], tD[100], dataC, ch;
    int mode, dataI, i, j, type, c[1000] = {0}, k, p= -1, b1, b2, w[1000] = {0}, l= 0;
    float dataF;

    FILE* fp;

    if(argc==1)
    {
        printf("Enter the file name to run: ");
        scanf("%s", buffer);
        fp= fopen( buffer, "rb");
        setvbuf(fp, NULL, _IOFBF, 1048576);
    } else {
        fp= fopen( argv[1], "rb");
        setvbuf(fp, NULL, _IOFBF, 1048576);
    }

    if (fp == NULL) {
        printf("Error: Could not open file.\n");
        exit(1);
    }
    global_fp = fp;
    current_fp = fp;
    
    run_interpreter_loop(fp, argc==1 ? buffer : argv[1]);
return 0;
}