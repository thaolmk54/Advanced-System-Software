#include<stdio.h>
#include<string.h>
#include<stdlib.h>

char ToHex(int num) {
    if (num < 0 && num > 15) {
        return '!';
    }
    else if (num < 10) {
        return num + 48;
    }
    else {
        switch (num) {
        case 10: return 'A';
        case 11: return 'B';
        case 12: return 'C';
        case 13: return 'D';
        case 14: return 'E';
        case 15: return 'F';
        }
    }
    return '?';
}

int ToDec(char *str) {
    int num = 0;
    bool flag = true;
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (flag && str[i] == '0') {
            continue;
        }
        flag = false;
        if (str[i] > '9') {
            num = 16 * num + (str[i] - 55);
        }
        else {
            num = 16 * num + (str[i] - '0');
        }
    }
    return num;
}

double ReadTime(const char *path) {
    char size[9];
    char byteRate[9];
    unsigned char ch[100];
    FILE *file;

    file = fopen(path,"rb");
    
    for (int i = 0; i < 58; i++) {
        ch[i] = fgetc(file);
    }

    for (int i = 7, j = 0; i >= 4; i--, j += 2) {
        if (ch[i] < 16) {
            size[j] = '0';
            size[j + 1] = ToHex(ch[i]);
        }
        else {
            size[j] = ToHex(ch[i] / 16);
            size[j + 1] = ToHex(ch[i] % 16);
        }
    }
    size[8] = '\0';

    for (int i = 31, j = 0; i > 27; i--, j += 2) {
        if (ch[i] < 16) {
            byteRate[j] = '0';
            byteRate[j + 1] = ToHex(ch[i]);
        }
        else {
            byteRate[j] = ToHex(ch[i] / 16);
            byteRate[j + 1] = ToHex(ch[i] % 16);
        }
    }
    byteRate[8] = '\0';

    int n_size = ToDec(size);
    int n_byteRate = ToDec(byteRate);
    double time = (double)n_size / (double)n_byteRate;
    
    return time;
}