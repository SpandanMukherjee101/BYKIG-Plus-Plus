#include <stdio.h>

#include <stdlib.h>
void safe_fscanf(FILE* fp, const char* fmt, char* buf) {
    if (fscanf(fp, fmt, buf) <= 0) {
        printf("Error: Unexpected EOF or read error.\n");
        fflush(stdout);
        exit(1);
    }
}

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

static inline void auto_trim(char *s) {
    if (!s) return;
    int len = strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' || s[len-1] == '\r' || s[len-1] == '\n')) {
        s[--len] = '\0';
    }
    int start = 0;
    while (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n') start++;
    if (start > 0) memmove(s, s + start, len - start + 1);
}

void execute_for_stmt(char *raw_stmt) {
    if (!raw_stmt) return;
    char stmt[1024];
    strncpy(stmt, raw_stmt, sizeof(stmt) - 1);
    stmt[sizeof(stmt) - 1] = '\0';
    
    char *s = stmt;
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
    int len = strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' || s[len-1] == '\r' || s[len-1] == '\n' || s[len-1] == ';')) {
        s[--len] = '\0';
    }
    if (len == 0) return;
    
    int decl_type = 0;
    int kw_len = 0;
    if (strncmp(s, "int ", 4) == 0) { decl_type = 1; kw_len = 4; }
    else if (strncmp(s, "float ", 6) == 0) { decl_type = 2; kw_len = 6; }
    else if (strncmp(s, "char ", 5) == 0) { decl_type = 3; kw_len = 5; }
    else if (strncmp(s, "bool ", 5) == 0) { decl_type = 4; kw_len = 5; }
    else if (strncmp(s, "string ", 7) == 0) { decl_type = 5; kw_len = 7; }
    
    if (decl_type > 0) {
        char *p = s + kw_len;
        char part[512] = "";
        int part_len = 0;
        int pdepth = 0;
        int in_q = 0;
        for (int i = 0; ; i++) {
            char ch = p[i];
            if (ch == '"') in_q = !in_q;
            if (!in_q) {
                if (ch == '(') pdepth++;
                else if (ch == ')') pdepth--;
            }
            if ((ch == ',' && pdepth == 0 && !in_q) || ch == '\0') {
                part[part_len] = '\0';
                auto_trim(part);
                if (strlen(part) > 0) {
                    char *eq = strchr(part, '=');
                    char vname[100] = "";
                    if (eq) {
                        int vlen = eq - part;
                        while (vlen > 0 && (part[vlen-1] == ' ' || part[vlen-1] == '\t')) vlen--;
                        int vstart = 0;
                        while (vstart < vlen && (part[vstart] == ' ' || part[vstart] == '\t')) vstart++;
                        int nlen = vlen - vstart;
                        if (nlen >= 100) nlen = 99;
                        strncpy(vname, part + vstart, nlen);
                        vname[nlen] = '\0';
                        
                        char *expr = eq + 1;
                        while (*expr == ' ' || *expr == '\t') expr++;
                        char exprBuf[1024];
                        snprintf(exprBuf, sizeof(exprBuf), "%s ;", expr);
                        struct Value v = val(exprBuf);
                        
                        if (decl_type == 1) appendI(&envStack[envTop].IV, vname, (int)v.f);
                        else if (decl_type == 2) appendF(&envStack[envTop].FV, vname, v.f);
                        else if (decl_type == 3) appendC(&envStack[envTop].CV, vname, (char)v.f);
                        else if (decl_type == 4) appendB(&envStack[envTop].BV, vname, (int)v.f);
                        else if (decl_type == 5) {
                            if (v.type == VAL_STRING) appendS(&envStack[envTop].SV, vname, v.s);
                            else {
                                char *start = strchr(expr, '"');
                                if (start) {
                                    char *end = strrchr(start + 1, '"');
                                    if (end) *end = '\0';
                                    appendS(&envStack[envTop].SV, vname, start + 1);
                                } else {
                                    char *sval = scopeGetSV(expr);
                                    appendS(&envStack[envTop].SV, vname, sval ? sval : "");
                                }
                            }
                        }
                    } else {
                        sscanf(part, "%s", vname);
                        if (decl_type == 1) appendI(&envStack[envTop].IV, vname, 0);
                        else if (decl_type == 2) appendF(&envStack[envTop].FV, vname, 0.0f);
                        else if (decl_type == 3) appendC(&envStack[envTop].CV, vname, '\0');
                        else if (decl_type == 4) appendB(&envStack[envTop].BV, vname, 0);
                        else if (decl_type == 5) appendS(&envStack[envTop].SV, vname, "");
                    }
                }
                part_len = 0;
                if (ch == '\0') break;
            } else {
                if (part_len < sizeof(part) - 1) part[part_len++] = ch;
            }
        }
        return;
    }
    
    char part[512] = "";
    int part_len = 0;
    int pdepth = 0;
    int in_q = 0;
    for (int i = 0; ; i++) {
        char ch = s[i];
        if (ch == '"') in_q = !in_q;
        if (!in_q) {
            if (ch == '(') pdepth++;
            else if (ch == ')') pdepth--;
        }
        if ((ch == ',' && pdepth == 0 && !in_q) || ch == '\0') {
            part[part_len] = '\0';
            auto_trim(part);
            if (strlen(part) > 0) {
                char *eq = NULL;
                int pd = 0;
                int q = 0;
                for (int j = 0; part[j]; j++) {
                    if (part[j] == '"') q = !q;
                    if (!q) {
                        if (part[j] == '(') pd++;
                        else if (part[j] == ')') pd--;
                        else if (pd == 0 && part[j] == '=' && part[j+1] != '=' &&
                                 (j == 0 || (part[j-1] != '=' && part[j-1] != '!' && part[j-1] != '<' && part[j-1] != '>'))) {
                            eq = &part[j];
                            break;
                        }
                    }
                }
                if (eq) {
                    char lhs[256] = "";
                    int l_len = eq - part;
                    while (l_len > 0 && (part[l_len-1] == ' ' || part[l_len-1] == '\t')) l_len--;
                    int l_start = 0;
                    while (l_start < l_len && (part[l_start] == ' ' || part[l_start] == '\t')) l_start++;
                    int real_l = l_len - l_start;
                    if (real_l >= 256) real_l = 255;
                    strncpy(lhs, part + l_start, real_l);
                    lhs[real_l] = '\0';
                    
                    char *rhs = eq + 1;
                    while (*rhs == ' ' || *rhs == '\t') rhs++;
                    char rhsBuf[1024];
                    snprintf(rhsBuf, sizeof(rhsBuf), "%s ;", rhs);
                    struct Value rVal = val(rhsBuf);
                    
                    char *bracket = strchr(lhs, '[');
                    if (bracket) {
                        char name[100] = "";
                        int nlen = bracket - lhs;
                        while (nlen > 0 && (lhs[nlen-1] == ' ' || lhs[nlen-1] == '\t')) nlen--;
                        if (nlen >= 100) nlen = 99;
                        strncpy(name, lhs, nlen);
                        name[nlen] = '\0';
                        
                        char *close_b = strrchr(bracket + 1, ']');
                        char idx_str[256] = "";
                        if (close_b) {
                            int ilen = close_b - (bracket + 1);
                            while (ilen > 0 && (bracket[1 + ilen - 1] == ' ' || bracket[1 + ilen - 1] == '\t')) ilen--;
                            char *istart = bracket + 1;
                            while (*istart == ' ' || *istart == '\t') { istart++; ilen--; }
                            if (ilen >= 256) ilen = 255;
                            strncpy(idx_str, istart, ilen);
                            idx_str[ilen] = '\0';
                        }
                        int t = typeFetcher(name);
                        char idxBuf[512];
                        snprintf(idxBuf, sizeof(idxBuf), "%s ;", idx_str);
                        if (t == 6) setIA(envStack[envTop].IA, name, (int)val(idxBuf).f, (int)rVal.f);
                        else if (t == 7) setFA(envStack[envTop].FA, name, (int)val(idxBuf).f, rVal.f);
                        else if (t == 8) setCA(envStack[envTop].CA, name, (int)val(idxBuf).f, (char)rVal.f);
                        else if (t == 9) setBA(envStack[envTop].BA, name, (int)val(idxBuf).f, (int)rVal.f);
                        else if (t == 10) setL(envStack[envTop].LV, name, (int)val(idxBuf).f, rVal);
                        else if (t == 11) {
                            char keyStr[100] = "";
                            struct Value kV = val(idxBuf);
                            if (kV.type == VAL_STRING) strcpy(keyStr, kV.s);
                            else if (kV.type == VAL_FLOAT) {
                                if (kV.f == (float)(int)kV.f) sprintf(keyStr, "%d", (int)kV.f);
                                else sprintf(keyStr, "%f", kV.f);
                            } else {
                                strcpy(keyStr, idx_str);
                                if (keyStr[0] == '"') {
                                    keyStr[strlen(keyStr)-1] = '\0';
                                    memmove(keyStr, keyStr+1, strlen(keyStr));
                                }
                            }
                            setM_Val(envStack[envTop].MV, name, keyStr, rVal);
                        }
                    } else {
                        int t = typeFetcher(lhs);
                        if (t == 1) {
                            if (!setI_scope(lhs, (int)rVal.f)) appendI(&envStack[envTop].IV, lhs, (int)rVal.f);
                        } else if (t == 2) {
                            if (!setF_scope(lhs, rVal.f)) appendF(&envStack[envTop].FV, lhs, rVal.f);
                        } else if (t == 3) {
                            if (!setC_scope(lhs, (char)rVal.f)) appendC(&envStack[envTop].CV, lhs, (char)rVal.f);
                        } else if (t == 4) {
                            if (!setB_scope(lhs, (int)rVal.f)) appendB(&envStack[envTop].BV, lhs, (int)rVal.f);
                        } else if (t == 5) {
                            if (rVal.type == VAL_STRING) {
                                if (!setS_scope(lhs, rVal.s)) appendS(&envStack[envTop].SV, lhs, rVal.s);
                            } else {
                                char *v = scopeGetSV(rhs);
                                if (v) {
                                    if (!setS_scope(lhs, v)) appendS(&envStack[envTop].SV, lhs, v);
                                } else {
                                    if (!setS_scope(lhs, rhs)) appendS(&envStack[envTop].SV, lhs, rhs);
                                }
                            }
                        } else if (t == 0) {
                            appendI(&envStack[envTop].IV, lhs, (int)rVal.f);
                        }
                    }
                } else {
                    char callBuf[1024];
                    snprintf(callBuf, sizeof(callBuf), "%s ;", part);
                    val(callBuf);
                }
            }
            part_len = 0;
            if (ch == '\0') break;
        } else {
            if (part_len < sizeof(part) - 1) part[part_len++] = ch;
        }
    }
}

struct Value run_interpreter_loop(FILE *fp, const char *current_filepath) {

    char buffer[1000], tempS[1000], name[100], tD[100], dataC, ch;
    int mode, dataI, i, j, type, c[1000] = {0}, k, p= -1, b1, b2, l= 0;
    long w[1000] = {0};
    int blockEnv[1000] = {0};
    struct forLoopState {
        char cond[256];
        char iter[256];
        long body_pos;
        int has_env;
    };
    struct forLoopState forLoops[64];
    memset(forLoops, 0, sizeof(forLoops));
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
                if (!strchr(buffer, '{')) fgets(buffer, 1000, fp);
                c[p]= 1;
                envTop++;
                initEnvironmentFrame(&envStack[envTop]);
                blockEnv[p] = 1;
            }
            else
            {
                if (!strchr(buffer, '{')) fgets(buffer, 1000, fp);
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
                if (!strchr(buffer, '{')) fgets(buffer, 1000, fp);
                c[p]= 1;
                envTop++;
                initEnvironmentFrame(&envStack[envTop]);
                blockEnv[p] = 1;
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
                if (!strchr(buffer, '{')) fgets(buffer, 1000, fp);
                c[p]= 1;
                envTop++;
                initEnvironmentFrame(&envStack[envTop]);
                blockEnv[p] = 1;
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

            if (c[p] == 3) {
                char after_buf[1000];
                int has_after = (fscanf(fp, "%s", after_buf) > 0);
                if (has_after) {
                    fseek(fp, -strlen(after_buf), SEEK_CUR);
                }

                if (strlen(forLoops[p].iter) > 0) {
                    execute_for_stmt(forLoops[p].iter);
                }
                int condVal = 1;
                if (strlen(forLoops[p].cond) > 0) {
                    char condBuf[512];
                    snprintf(condBuf, sizeof(condBuf), "%s ;", forLoops[p].cond);
                    condVal = (int)val(condBuf).f;
                }
                if (condVal) {
                    fseek(fp, forLoops[p].body_pos, SEEK_SET);
                    k = p - 1;
                } else {
                    if (forLoops[p].has_env) {
                        cleanupFrame(&envStack[envTop]);
                        envTop--;
                        forLoops[p].has_env = 0;
                    }
                    c[p] = 0;
                    k = --p;
                }
                break;
            }
        
            safe_fscanf(fp, "%s", buffer); if(strcmp(buffer, "elif") && strcmp(buffer, "else") && c[p]!= 2)
            {
                if (p >= 0 && blockEnv[p]) {
                    cleanupFrame(&envStack[envTop]);
                    envTop--;
                    blockEnv[p] = 0;
                }
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
                    if (p >= 0 && blockEnv[p]) {
                        cleanupFrame(&envStack[envTop]);
                        envTop--;
                        blockEnv[p] = 0;
                    }
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
            if (!strchr(buffer, '{')) fgets(buffer, 1000, fp);
            
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
                if (!setI_scope(buffer, dataI)) appendI( &envStack[envTop].IV, buffer, dataI);
            }
            else if (type == 2)
            {
                scanf("%f",&dataF);
                if (!setF_scope(buffer, dataF)) appendF( &envStack[envTop].FV, buffer, dataF);
            }
            else if (type == 3)
            {
                scanf("%c",&dataC);
                if (!setC_scope(buffer, dataC)) appendC( &envStack[envTop].CV, buffer, dataC);
            }
            else if (type == 5)
            {
                char tempS[1000];
                if (scanf("%s", tempS) <= 0) exit(1);
                if (!setS_scope(buffer, tempS)) appendS(&envStack[envTop].SV, buffer, tempS);
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
                    if (kV.type == VAL_STRING) {  
                        strcpy(keyStr, kV.s);
                    } else if (kV.type == VAL_FLOAT) {
                        if (kV.f == (float)(int)kV.f) sprintf(keyStr, "%d", (int)kV.f);
                        else sprintf(keyStr, "%f", kV.f);
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
                    struct Value vF; memset(&vF, 0, sizeof(vF)); vF.type = VAL_FLOAT; vF.f = valF;
                    setL(envStack[envTop].LV, nameBuf, idx, vF);
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
                if (findFunc(FN, buffer) != NULL || findBuiltin(buffer) != NULL) {
                    char exprBuf[1000] = "";
                    strcpy(exprBuf, buffer);
                    strcat(exprBuf, " ");
                    int pDepth = 0;
                    char tD[100];
                    while (1) {
                        safe_fscanf(fp, "%s", tD);
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

            if (type >= 6 && type <= 11) {  
                safe_fscanf(fp, "%s", buffer); safe_fscanf(fp, "%s", buffer); int idx = 0;
                char keyStr[1000] = "";
                if (type == 11) {
                    struct Value kV = val(buffer);
                    if (kV.type == 1) {  
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
                else if (type == 10) setL(envStack[envTop].LV, name, idx, val(exprBuf));
                else if (type == 11) setM_Val(envStack[envTop].MV, name, keyStr, val(exprBuf));
            } else {
                fgets(buffer, 1000, fp);

                if (type == 1)
                {
                    dataI= (int)val(buffer).f;
                    if (!setI_scope(name, dataI)) appendI( &envStack[envTop].IV, name, dataI);
                }
                else if (type == 2)
                {
                    dataF = val(buffer).f;
                    if (!setF_scope(name, dataF)) appendF( &envStack[envTop].FV, name, dataF);
                }
                else if (type == 3) {
                    char cVal = (char)val(buffer).f;
                    if (!setC_scope(name, cVal)) appendC(&envStack[envTop].CV, name, cVal);
                }
                else if (type == 4) {
                    int bVal = (int)val(buffer).f;
                    if (!setB_scope(name, bVal)) appendB(&envStack[envTop].BV, name, bVal);
                }
                else if (type == 5) {
                    char *eq = strchr(buffer, '=');
                    if (eq) {
                        strcpy(builtin_string_return, "");
                        val(eq + 1);
                        if (strlen(builtin_string_return) > 0) {
                            if (!setS_scope(name, builtin_string_return)) appendS(&envStack[envTop].SV, name, builtin_string_return);
                        } else {
                            char *start = strchr(eq + 1, '"');
                            if (start) {
                                char *end = strrchr(start + 1, '"');
                                if (end) *end = '\0';
                                if (!setS_scope(name, start + 1)) appendS(&envStack[envTop].SV, name, start + 1);
                            } else {
                                struct Value sRes = val(eq + 1);
                                if (sRes.type == VAL_STRING) {
                                    if (!setS_scope(name, sRes.s)) appendS(&envStack[envTop].SV, name, sRes.s);
                                } else {
                                    char *src = strtok(eq + 1, " \t\r\n;");
                                    char *v = scopeGetSV(src);
                                    if (v) {
                                        if (!setS_scope(name, v)) appendS(&envStack[envTop].SV, name, v);
                                    }
                                }
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

                if (!(strcmp(buffer,",")))
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

case 15:  
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
                safe_fscanf(fp, "%s", buffer); appendIA(&envStack[envTop].IA, name, 100);  
                int i = 0;
                safe_fscanf(fp, "%s", buffer); while (strcmp(buffer, "}")) {
                    if (strcmp(buffer, ",")) {
                        setIA(envStack[envTop].IA, name, i++, (int)val(buffer).f);
                    }
                    safe_fscanf(fp, "%s", buffer); }
                safe_fscanf(fp, "%s", buffer); struct iA *arr = envStack[envTop].IA;
                while (arr) { if (!strcmp(arr->name, name)) { arr->size = i; break; } arr = arr->next; }
            } else {
                int size = (int)val(buffer).f;
                safe_fscanf(fp, "%s", buffer);
                appendIA(&envStack[envTop].IA, name, size);
                if (!strcmp(buffer, "=")) {
                    safe_fscanf(fp, "%s", buffer); int i = 0;
                    safe_fscanf(fp, "%s", buffer); while (strcmp(buffer, "}")) {
                        if (strcmp(buffer, ",")) {
                            setIA(envStack[envTop].IA, name, i++, (int)val(buffer).f);
                        }
                        safe_fscanf(fp, "%s", buffer); }
                    safe_fscanf(fp, "%s", buffer); }
            }
            break;

        case 16:  
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
            } else {
                int size = (int)val(buffer).f;
                safe_fscanf(fp, "%s", buffer);
                appendFA(&envStack[envTop].FA, name, size);
                if (!strcmp(buffer, "=")) {
                    safe_fscanf(fp, "%s", buffer); int i = 0;
                    safe_fscanf(fp, "%s", buffer); while (strcmp(buffer, "}")) {
                        if (strcmp(buffer, ",")) {
                            setFA(envStack[envTop].FA, name, i++, val(buffer).f);
                        }
                        safe_fscanf(fp, "%s", buffer); }
                    safe_fscanf(fp, "%s", buffer); }
            }
            break;

        case 17:  
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
            } else {
                int size = (int)val(buffer).f;
                safe_fscanf(fp, "%s", buffer);
                appendCA(&envStack[envTop].CA, name, size);
                if (!strcmp(buffer, "=")) {
                    safe_fscanf(fp, "%s", buffer); int i = 0;
                    safe_fscanf(fp, "%s", buffer); while (strcmp(buffer, "}")) {
                        if (strcmp(buffer, ",")) {
                            setCA(envStack[envTop].CA, name, i++, buffer[1] == '\'' ? buffer[1] : buffer[0]);
                        }
                        safe_fscanf(fp, "%s", buffer); }
                    safe_fscanf(fp, "%s", buffer); }
            }
            break;

        case 18:  
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
            } else {
                int size = (int)val(buffer).f;
                safe_fscanf(fp, "%s", buffer);
                appendBA(&envStack[envTop].BA, name, size);
                if (!strcmp(buffer, "=")) {
                    safe_fscanf(fp, "%s", buffer); int i = 0;
                    safe_fscanf(fp, "%s", buffer); while (strcmp(buffer, "}")) {
                        if (strcmp(buffer, ",")) {
                            setBA(envStack[envTop].BA, name, i++, (int)val(buffer).f);
                        }
                        safe_fscanf(fp, "%s", buffer); }
                    safe_fscanf(fp, "%s", buffer); }
            }
            break;

        case 19:  
            safe_fscanf(fp, "%s", name);
            fgets(buffer, 1000, fp);
            {
                char *eq = strchr(buffer, '=');
                if (eq) {
                    char *rhs = eq + 1;
                    while (*rhs == ' ' || *rhs == '\t') rhs++;
                    if (*rhs == '"') {
                        char *end = strrchr(rhs + 1, '"');
                        if (end) *end = '\0';
                        scopeSetSV(name, rhs + 1);
                    } else {
                        strcpy(builtin_string_return, "");
                        struct Value sRes = val(rhs);
                        if (strlen(builtin_string_return) > 0) {
                            scopeSetSV(name, builtin_string_return);
                        } else if (sRes.type == VAL_STRING) {
                            scopeSetSV(name, sRes.s);
                        } else {
                            char *src = strtok(rhs, " \t\r\n;");
                            if (src) {
                                char *v = scopeGetSV(src);
                                if (v) scopeSetSV(name, v);
                            }
                        }
                    }
                } else {
                    scopeSetSV(name, "");
                }
            }
            break;

        case 20:  
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
                        safe_fscanf(fp, "%s", params[paramCount].name);  
                        paramCount++;
                    }
                    safe_fscanf(fp, "%s", buffer); }
                
                safe_fscanf(fp, "%s", buffer);  
                long bodyPos = ftell(fp);
                if (!isDuplicate) {
                    char tempPath[1000] = "";
                    if (current_filepath != NULL) strcpy(tempPath, current_filepath);
                    if (appendFunc(&FN, funcName, paramCount, params, bodyPos, tempPath) != 0) {
                        printf("Error: Failed to register function '%s'\n", funcName);
                    }
                }
                
                 
                b1 = 1;  
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

        case 21:  
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

        case 22:  
            {
                safe_fscanf(fp, "%s", name);  
                appendL(&envStack[envTop].LV, name, 4);  

                safe_fscanf(fp, "%s", buffer); if (!(strcmp(buffer, "="))) {
                    safe_fscanf(fp, "%s", buffer); while (strcmp(buffer, "}")) {
                        safe_fscanf(fp, "%s", buffer); if (!strcmp(buffer, "}")) break;
                        scopePushL(name, val(buffer));
                        safe_fscanf(fp, "%s", buffer); if (!strcmp(buffer, "}")) break;
                    }
                    safe_fscanf(fp, "%s", buffer); }
            }
            break;

        

        case 25:  
            {
                int target_p = p;
                while (target_p >= 0 && c[target_p] != 2 && c[target_p] != 3) {
                    target_p--;
                }
                if (target_p < 0) {
                    printf("Error: break outside of loop\n");
                    break;
                }
                for (int d = p; d >= target_p; d--) {
                    if (d >= 0 && d < 1000 && blockEnv[d]) {
                        cleanupFrame(&envStack[envTop]);
                        envTop--;
                        blockEnv[d] = 0;
                    }
                    if (d >= 0 && d < 64 && forLoops[d].has_env) {
                        cleanupFrame(&envStack[envTop]);
                        envTop--;
                        forLoops[d].has_env = 0;
                    }
                }
                
                int k = target_p - 1;
                while (k != p) {
                    if (!fgets(buffer, 1000, fp)) break;
                    for (int i = 0; buffer[i]; i++) {
                        if (buffer[i] == '"') {
                            i++;
                            while (buffer[i] && buffer[i] != '"') i++;
                            if (!buffer[i]) break;
                        } else if (buffer[i] == '/' && buffer[i+1] == '/') {
                            break;
                        } else if (buffer[i] == '{') {
                            p++;
                        } else if (buffer[i] == '}') {
                            p--;
                        }
                    }
                }
                c[++p] = 0;
                --p;
            }
            break;

        case 26:  
            {
                int target_p = p;
                while (target_p >= 0 && c[target_p] != 2 && c[target_p] != 3) {
                    target_p--;
                }
                if (target_p < 0) {
                    printf("Error: continue outside of loop\n");
                    break;
                }
                for (int d = p; d > target_p; d--) {
                    if (d >= 0 && d < 1000 && blockEnv[d]) {
                        cleanupFrame(&envStack[envTop]);
                        envTop--;
                        blockEnv[d] = 0;
                    }
                    if (d >= 0 && d < 64 && forLoops[d].has_env) {
                        cleanupFrame(&envStack[envTop]);
                        envTop--;
                        forLoops[d].has_env = 0;
                    }
                }
                if (c[target_p] == 2) {
                    fseek(fp, w[target_p], SEEK_SET);
                    w[target_p] = 0;
                    p = target_p - 1;
                    k = p;
                } else if (c[target_p] == 3) {
                    if (strlen(forLoops[target_p].iter) > 0) {
                        execute_for_stmt(forLoops[target_p].iter);
                    }
                    int condVal = 1;
                    if (strlen(forLoops[target_p].cond) > 0) {
                        char condBuf[512];
                        snprintf(condBuf, sizeof(condBuf), "%s ;", forLoops[target_p].cond);
                        condVal = (int)val(condBuf).f;
                    }
                    if (condVal) {
                        p = target_p;
                        k = p - 1;
                        fseek(fp, forLoops[target_p].body_pos, SEEK_SET);
                    } else {
                        fseek(fp, forLoops[target_p].body_pos, SEEK_SET);
                        int skip_depth = target_p;
                        int k_loop = target_p - 1;
                        while (k_loop != skip_depth) {
                            if (!fgets(buffer, 1000, fp)) break;
                            for (int i = 0; buffer[i]; i++) {
                                if (buffer[i] == '"') {
                                    i++;
                                    while (buffer[i] && buffer[i] != '"') i++;
                                    if (!buffer[i]) break;
                                } else if (buffer[i] == '/' && buffer[i+1] == '/') {
                                    break;
                                } else if (buffer[i] == '{') {
                                    skip_depth++;
                                } else if (buffer[i] == '}') {
                                    skip_depth--;
                                }
                            }
                        }
                        if (forLoops[target_p].has_env) {
                            cleanupFrame(&envStack[envTop]);
                            envTop--;
                            forLoops[target_p].has_env = 0;
                        }
                        c[target_p] = 0;
                        p = target_p - 1;
                        k = p;
                    }
                }
            }
            break;

        case 24:  
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

        case 23:  
            {
                safe_fscanf(fp, "%s", name);  
                appendM(&envStack[envTop].MV, name);  
                safe_fscanf(fp, "%s", buffer); if (!(strcmp(buffer, "="))) {
                    safe_fscanf(fp, "%s", buffer); while (strcmp(buffer, "}")) {
                        safe_fscanf(fp, "%s", buffer); if (!strcmp(buffer, "}")) break;
                        char key[100]; strcpy(key, buffer);
                        if (key[0] == '"') {
                            key[strlen(key)-1] = '\0';
                            memmove(key, key+1, strlen(key));
                        }
                        safe_fscanf(fp, "%s", buffer); safe_fscanf(fp, "%s", buffer);
                        struct Value d = val(buffer);
                        setM_Val(envStack[envTop].MV, name, key, d);
                        safe_fscanf(fp, "%s", buffer); if (!strcmp(buffer, "}")) break;
                    }
                    safe_fscanf(fp, "%s", buffer); }
            }
            break;

        case 27:
            {
                char header[1024] = "";
                int hlen = 0;
                int ch;
                while ((ch = fgetc(fp)) != EOF) {
                    if (ch == '{') break;
                    if (ch == '/') {
                        int next_ch = fgetc(fp);
                        if (next_ch == '/') {
                            while ((ch = fgetc(fp)) != EOF && ch != '\n');
                            continue;
                        } else {
                            ungetc(next_ch, fp);
                        }
                    }
                    if (hlen < sizeof(header) - 1) {
                        header[hlen++] = (char)ch;
                    }
                }
                header[hlen] = '\0';
                if (ch == EOF) {
                    printf("Error: Expected '{' after for loop header\n");
                    break;
                }
                long body_pos = ftell(fp);
                
                char *h = header;
                while (*h == ' ' || *h == '\t' || *h == '\r' || *h == '\n') h++;
                int h_end = strlen(h) - 1;
                while (h_end >= 0 && (h[h_end] == ' ' || h[h_end] == '\t' || h[h_end] == '\r' || h[h_end] == '\n')) {
                    h[h_end--] = '\0';
                }
                if (h[0] == '(') {
                    int pdepth = 0;
                    int match_idx = -1;
                    for (int i = 0; h[i] != '\0'; i++) {
                        if (h[i] == '(') pdepth++;
                        else if (h[i] == ')') {
                            pdepth--;
                            if (pdepth == 0) {
                                match_idx = i;
                                break;
                            }
                        }
                    }
                    if (match_idx == (int)strlen(h) - 1) {
                        h[match_idx] = '\0';
                        h++;
                    }
                }
                
                char init_str[256] = "";
                char cond_str[256] = "";
                char iter_str[256] = "";
                
                int semi1 = -1, semi2 = -1;
                int pdepth = 0;
                int in_q = 0;
                for (int i = 0; h[i] != '\0'; i++) {
                    if (h[i] == '"') in_q = !in_q;
                    if (!in_q) {
                        if (h[i] == '(') pdepth++;
                        else if (h[i] == ')') pdepth--;
                        else if (h[i] == ';' && pdepth == 0) {
                            if (semi1 == -1) semi1 = i;
                            else if (semi2 == -1) { semi2 = i; break; }
                        }
                    }
                }
                
                if (semi1 != -1) {
                    int len1 = semi1;
                    if (len1 >= 256) len1 = 255;
                    strncpy(init_str, h, len1);
                    init_str[len1] = '\0';
                    if (semi2 != -1) {
                        int len2 = semi2 - (semi1 + 1);
                        if (len2 >= 256) len2 = 255;
                        strncpy(cond_str, h + semi1 + 1, len2);
                        cond_str[len2] = '\0';
                        
                        strncpy(iter_str, h + semi2 + 1, sizeof(iter_str) - 1);
                        iter_str[sizeof(iter_str) - 1] = '\0';
                    } else {
                        strncpy(cond_str, h + semi1 + 1, sizeof(cond_str) - 1);
                        cond_str[sizeof(cond_str) - 1] = '\0';
                    }
                }
                
                auto_trim(init_str);
                auto_trim(cond_str);
                auto_trim(iter_str);
                
                p++;
                k = p - 1;
                envTop++;
                initEnvironmentFrame(&envStack[envTop]);
                forLoops[p].has_env = 1;
                strncpy(forLoops[p].cond, cond_str, sizeof(forLoops[p].cond) - 1);
                forLoops[p].cond[sizeof(forLoops[p].cond) - 1] = '\0';
                strncpy(forLoops[p].iter, iter_str, sizeof(forLoops[p].iter) - 1);
                forLoops[p].iter[sizeof(forLoops[p].iter) - 1] = '\0';
                forLoops[p].body_pos = body_pos;
                
                if (strlen(init_str) > 0) {
                    execute_for_stmt(init_str);
                }
                
                int condVal = 1;
                if (strlen(cond_str) > 0) {
                    char condBuf[512];
                    snprintf(condBuf, sizeof(condBuf), "%s ;", cond_str);
                    condVal = (int)val(condBuf).f;
                }
                
                if (condVal) {
                    c[p] = 3;
                } else {
                    int k_loop = p - 1;
                    while (k_loop != p) {
                        if (!fgets(buffer, sizeof(buffer), fp)) break;
                        for (int i = 0; buffer[i]; i++) {
                            if (buffer[i] == '"') {
                                i++;
                                while (buffer[i] && buffer[i] != '"') i++;
                                if (!buffer[i]) break;
                            } else if (buffer[i] == '/' && buffer[i+1] == '/') {
                                break;
                            } else if (buffer[i] == '{') {
                                p++;
                            } else if (buffer[i] == '}') {
                                p--;
                            }
                        }
                    }
                    if (forLoops[p + 1].has_env) {
                        cleanupFrame(&envStack[envTop]);
                        envTop--;
                        forLoops[p + 1].has_env = 0;
                    }
                    c[++p] = 0;
                    --p;
                    k = p;
                }
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
    } else {
        fp= fopen( argv[1], "rb");
    }

    if (fp == NULL) {
        printf("Error: Could not open file.\n");
        exit(1);
    }
    
    setvbuf(fp, NULL, _IOFBF, 1048576);
    global_fp = fp;
    current_fp = fp;
    
    run_interpreter_loop(fp, argc==1 ? buffer : argv[1]);
return 0;
}