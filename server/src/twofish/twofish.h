
#if !defined(__TWOFISH_H__)
#define __TWOFISH_H__

void keySched( unsigned char[], int, unsigned int**, unsigned int[40], int* );
void fullKey( unsigned int[4], int, unsigned int[4][256] );
void decrypt( unsigned int[40], unsigned int[4][256], unsigned char* );
void encrypt( unsigned int[40], unsigned int[4][256], unsigned char* );


#endif
