#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main()
{
    char cmd[1100]={"start Interpreter.exe "}, fn[1000];

    printf("Welcome to BYKIG!!!\a\a\nEnter the filename to be interpreted:\t");
    gets( fn);
    strcat(cmd, fn);
    strcat(cmd, ".bykig");

    FILE *fp= fopen("temp.bat","w");
    fputs( cmd, fp);
    fclose( fp);

    system("temp.bat");

    remove( "temp.bat");

    return 0;
}
