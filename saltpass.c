#include <string.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <math.h>

void read_string(char* password)
{
    static struct termios old_terminal;
    static struct termios new_terminal;

    // get settings of the actual terminal
    tcgetattr(0, &old_terminal);

    // do not echo the characters
    new_terminal = old_terminal;
    new_terminal.c_lflag &= ~(ECHO);

    // set this as the new terminal options
    tcsetattr(0, TCSANOW, &new_terminal);

    // get the password
    // the user can add chars and delete if he puts it wrong
    // the input process is done when he hits the enter
    fgets(password, BUFSIZ, stdin);

    // go back to the old settings
    tcsetattr(0, TCSANOW, &old_terminal);
}

void memclear_string(char *ch, int l){
    for(int i = 0; i < l; i++)
        ch[i] = '\0'; 
    l = 0;
}

int main(int argc, char* argv[])
{
    //read salt without displaying in terminal
    puts("Insert salt (could be website name, file name, etc..), backspace works:");
    char salt[BUFSIZ]; memclear_string(salt, sizeof(salt));
    read_string(salt);
    salt[strlen(salt) - 1] = '\0';//replace newline with 0

    //get up to date salt value if file exists and it exists in file and file name was passed as argument
    if(argc > 1){
        FILE *f;
        if ((f = fopen(argv[1],"r")) == NULL){
            fclose(f);
        }
        else{
            fseek(f, 0, SEEK_END);
            long fsize = ftell(f);
            fseek(f, 0, SEEK_SET);  /* same as rewind(f); */
            char slt_f[fsize + 1];
            slt_f[fsize] = '\0';
            // read full file
            fread(&slt_f, 1, fsize, f);
            fclose(f);
            char *ind = strstr(slt_f, salt); //see if newer salt exists
            if(ind != NULL)
            {
                printf("Found and loaded up to date salt from file\n");
                memclear_string(salt, sizeof(salt));
                for(int i = 0; i < BUFSIZ; i++){
                    if(ind[i] != '\n')
                        salt[i] = ind[i];
                    else
                        break;
                }
            }
        }
    }

    //read password without displaying in terminal
    char pass[BUFSIZ]; memclear_string(pass, sizeof(pass));
    puts("Insert password, backspace works:");
    read_string(pass);
    pass[strlen(pass) - 1] = '\0';

    //create string that will be hashed to produce unique password
    char data[BUFSIZ*2]; memclear_string(data, sizeof(data));

    int stop_ind = 0;
    for(int i = 0; i < BUFSIZ; i++){
        if(pass[i] != '\0' && pass[i] != '\000' && pass[i] != '\n')
            data[i] = pass[i];
        else{
            stop_ind = i;
            break;
        }
    }
    //append salt
    for(int i = 0; i <= BUFSIZ; i++){
        if(salt[i] != '\0' && salt[i] != '\000' && salt[i] != '\n')
            data[stop_ind + i] = salt[i];
        else{
            stop_ind += i;
            break;
        }
    } 
    
    //clear memory
    stop_ind = 0; 
    memclear_string(pass, sizeof(pass));
    memclear_string(salt, sizeof(salt));

    char hash[64 + 1];
    hash[64] = '\0';
    // pass to sha512 hash function
    SHA512(data, strlen(data), hash);
    memclear_string(data, sizeof(data));

    const int PASS_LENGTH = 16;
    char out[PASS_LENGTH + 1];
    out[PASS_LENGTH] = '\0';

    //convert to output password by remapping to appropriate ascii chars range[33,126]
    for(int i = 0; i < PASS_LENGTH; i++){
        out[i] = (float)(unsigned char)hash[i]/255*(126-33) + 33;
    }
    printf("Output password given salt is\n");
    puts(out);
    printf("Preferably, do not copy the password to clipboard\n");
    memclear_string(out, sizeof(out));
    memclear_string(hash, sizeof(hash));

    return 0;
}
