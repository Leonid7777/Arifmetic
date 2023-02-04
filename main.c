#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>

void
new_write(int out, long long *index, unsigned char *val){
    write(out, val, sizeof(char));
    (*index) = 8;
    (*val) = 0;
}


void
bits_plus_follow(unsigned char bit, int out, unsigned char *val, long long *count_byte, long long *bits_to_follow)
{
    if(!(*count_byte)){
        new_write(out, count_byte, val);
    }
    if(bit){
        (*val) |= (1 << ((*count_byte) - 1));
    }
    (*count_byte) -= 1;
    while((*bits_to_follow))
    {
        if (!(*count_byte)) {
            new_write(out, count_byte, val);
        }
        if (!bit) {
            (*val) |= (1 << ((*count_byte) - 1));
        }
        (*count_byte) -= 1;
        (*bits_to_follow) -= 1;
    }
}

void
change_of_mas(long long mas[], unsigned char c)
{
    long long max = 2000;
    long long agress = 10;
    for(int i = c + 1; i < 258; i++){
        mas[i] += agress;
    }
    while(mas[c] > max){
        long long old = mas[0];
        for(int i = 1; i < 258; i++){
            long long newl = mas[i] - old + 1;
            old = mas[i];
            mas[i] = mas[i - 1] + newl / 2;
        }
    }
}

void
compress_ari(char *ifile, char *ofile)
{
    int inf = open(ifile, O_RDONLY, 0777);
    int ouf = open(ofile, O_WRONLY | O_TRUNC | O_CREAT, 0777);
    long long First_qtr = 1073741824, Half = 2147483648, Third_qtr = 3221225472, count_byte = 8, bits_to_follow = 0, l = 0, h = 4294967296, b[258];
    unsigned char c, byte = 0;
    b[0] = 0;
    for(int i = 1; i < 258; i++){
        b[i] = b[i - 1] + 1;
    }
    while(read(inf, &c , sizeof(char))){
        long long old_l = l;
        l += b[c] * (h - l + 1) / b[257];
        h = old_l + b[c + 1] * (h - old_l + 1) / b[257] - 1;
        change_of_mas(b, c);
        for(;;) {
            if(h < Half){
                bits_plus_follow(0, ouf, &byte, &count_byte, &bits_to_follow);
            } else if(l >= Half) {
                bits_plus_follow(1, ouf, &byte, &count_byte, &bits_to_follow);
                l -= Half;
                h -= Half;
            }
            else if((l >= First_qtr) && (h < Third_qtr)){
                bits_to_follow++;
                l -= First_qtr;
                h -= First_qtr;
            } else {
                break;
            }
            l *= 2;
            h += h + 1;
        }
    }
    l += b[256] * (h - l + 1) / b[257];
    for(;;) {
        if(h < Half){
            bits_plus_follow(0, ouf, &byte, &count_byte, &bits_to_follow);
        } else if(l >= Half) {
            bits_plus_follow(1, ouf, &byte, &count_byte, &bits_to_follow);
            l -= Half;
            h -= Half;
        }
        else if((l >= First_qtr) && (h < Third_qtr)){
            bits_to_follow++;
            l -= First_qtr;
            h -= First_qtr;
        } else {
            break;
        }
        l *= 2;
        h += h + 1;
    }
    bits_to_follow += 1;
    if(l < First_qtr){
        bits_plus_follow(0, ouf, &byte, &count_byte, &bits_to_follow);
    } else {
        bits_plus_follow(1, ouf, &byte, &count_byte, &bits_to_follow);
    }
    write(ouf, &byte , sizeof(char));
}

void
decompress_ari(char *ifile, char *ofile){
    int inf = open(ifile, O_RDONLY, 0777);
    int ouf = open(ofile, O_WRONLY | O_TRUNC | O_CREAT, 0777);
    long long First_qtr = 1073741824, Half = 2147483648, Third_qtr = 3221225472, EOFC = 256, value = 0, index = 0, c, b[258], h = 4294967296, l = 0;;
    unsigned char tmp = 0;
    lseek(inf, 0, SEEK_SET);
    b[0] = 0;
    for(int i = 1; i < 258; i++){
        b[i] = b[i - 1] + 1;
    }
    value = 0;
    for(int i = 0; i < 4; i++){
        value <<= 8;
        read(inf, &tmp, sizeof(char));
        value |= tmp;
    }
    for(;;){
        long long newl = ((value - l + 1) * b[257] - 1) / (h - l + 1);
        c = 0;
        while(b[c + 1] <= newl){
            c += 1;
        }
        if(c == EOFC){
            break;
        }
        long long old_l = l;
        l += b[c] * (h - l + 1) / b[257];
        h = old_l + b[c + 1] * (h - old_l + 1) / b[257] - 1;
        change_of_mas(b, c);
        if(!index){
            index = 8;
            read(inf, &tmp, sizeof(char));
        }
        for(;;){
             if (h < Half) {
                 ;
             } else if(l >= Half) {
                value -= Half;
                l -= Half;
                h -= Half;
            }
            else if((l >= First_qtr) && (h < Third_qtr)){
                value -= First_qtr;
                l -= First_qtr;
                h -= First_qtr;
           } else {
                break;
            }
            l *= 2;
            h += h + 1;
            value *= 2;
            value |= (tmp >> 7);
            tmp <<= 1;
            index -= 1;
            if(!index){
                index = 8;
                read(inf, &tmp, sizeof(char));
            }
        }
        write(ouf, &c, sizeof(char));
   }
}
