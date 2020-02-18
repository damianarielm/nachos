static inline unsigned
strlen(const char* s) {
    unsigned i;
    for (i = 0; s[i]; i++);
    return i;
}

void itoa(char str[], int num) {
    int rem, len = 0, n;

    n = num;
    while (n != 0) {
        len++;
        n /= 10;
    }

    for (int i = 0; i < len; i++) {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }

    if (len)
        str[len] = '\0';
    else {
        str[0] = '0';
        str[1] = '\0';
    }
}
