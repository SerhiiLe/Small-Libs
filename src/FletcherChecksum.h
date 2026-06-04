#ifndef FletcherChecksum_h
#define FletcherChecksum_h

/*
Расчёт контрольной суммы
На входе указатель на буфер и его размер
На выходе контрольная сумма в одном или двух байтах
*/

// typedef unsigned short uint16_t;
// typedef unsigned char uint8_t;

struct FletcherChecksum
{
    // Calculating the checksum using the Flatcher8 formula
    static uint8_t fletcher8(uint8_t *data, size_t len) {
        uint16_t sum1 = 0xf, sum2 = 0xf;
        while( len-- ) {
            sum1 += *data++;
            sum2 += sum1;
        };
        sum1 = (sum1 & 0x0f) + (sum1 >> 4);
        sum1 = (sum1 & 0x0f) + (sum1 >> 4);
        sum2 = (sum2 & 0x0f) + (sum2 >> 4);
        sum2 = (sum2 & 0x0f) + (sum2 >> 4);
        return sum2<<4 | sum1;
    }

    // Calculating the checksum using the Flatcher16 formula
    static uint16_t fletcher16(uint8_t *data, size_t len) {
        uint16_t sum1 = 0;
        uint16_t sum2 = 0;
        while( len-- ) {
            sum1 = (sum1 + *(data++)) % 255;
            sum2 = (sum1 + sum2) % 255;
        }
        return (sum2 << 8) | sum1;
    }
};

#endif