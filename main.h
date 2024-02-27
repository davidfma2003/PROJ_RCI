#ifndef MAIN_H
#define MAIN_H

typedef struct IP_s{
    char reg_IP[30];
    int reg_UDP;
    int TCP;
    char*IP;
}conect_inf;

// Function declarations or prototypes

int main(int argc, char *argv[]);

#endif // MAIN_H
