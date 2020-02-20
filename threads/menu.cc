#include <stdio.h>

void ThreadTest();
void Garden();
void ProdCons();
void PortTest();

void Menu() {
    unsigned opt;

    printf("0 - Thread test.\n");
    printf("1 - Ornamental garden.\n");
    printf("2 - Producer/consumer.\n");
    printf("3 - Port test.\n");
    printf("Enter an option: ");
    scanf("%u", &opt);

    switch (opt) {
        case 0: ThreadTest(); break;
        case 1: Garden(); break;
        case 2: ProdCons(); break;
        case 3: PortTest(); break;
        default: printf("Invalid option.\n");
    }
}
