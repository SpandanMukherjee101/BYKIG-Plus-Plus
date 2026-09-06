
#include <string.h>

char __datatypes[3][6]={
"int",
"float",
"char"
};

char __ifel[4][5]={
"if",
"elif",
"else",
"}"
};

char __loop[6]={"while"};

char __basicFunctions[2][4]={
"out",
"in"
};

int whichone(char *c)
{
    if (!(strcmp(c,__datatypes[0]))) return 1;
    else if (!(strcmp(c,__datatypes[1]))) return 2;
    else if (!(strcmp(c,__datatypes[2]))) return 3;
    else if (!(strcmp(c,__ifel[0]))) return 4;
    else if (!(strcmp(c,__ifel[1]))) return 5;
    else if (!(strcmp(c,__ifel[2]))) return 6;
    else if (!(strcmp(c,__ifel[3]))) return 7;
    else if (!(strcmp(c,__loop))) return 8;
    else if (!(strcmp(c,__basicFunctions[0]))) return 9;
    else if (!(strcmp(c,__basicFunctions[1]))) return 10;
    
     
    else if (!(strcmp(c, "bool"))) return 14;
    else if (!(strcmp(c, "intarr"))) return 15;
    else if (!(strcmp(c, "floatarr"))) return 16;
    else if (!(strcmp(c, "chararr"))) return 17;
    else if (!(strcmp(c, "boolarr"))) return 18;
    else if (!(strcmp(c, "string"))) return 19;
    else if (!(strcmp(c, "func"))) return 20;
    else if (!(strcmp(c, "return"))) return 21;
    else if (!(strcmp(c, "list"))) return 22;
    else if (!(strcmp(c, "map"))) return 23;
    else if (!(strcmp(c, "use"))) return 24;
    else if (!(strcmp(c, "break"))) return 25;
    else if (!(strcmp(c, "continue"))) return 26;
    
    else return 11;
}
