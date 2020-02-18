#include <stdio.h>

void ThreadTest();
void Garden();

void Menu() {
    unsigned opt;

    printf("0 - Thread test.\n");
    printf("1 - Ornamental garden.\n");
    printf("Enter an option: ");
    scanf("%u", &opt);

    switch (opt) {
        case 0: ThreadTest(); break;
        case 1: Garden(); break;
        default: printf("Invalid option.\n");
    }
}
