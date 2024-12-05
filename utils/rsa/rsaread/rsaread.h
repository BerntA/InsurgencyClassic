#ifndef RSAREAD_H
#define RSAREAD_H

unsigned char* read_file_check_signature(unsigned char* abData, unsigned int ulLength, const unsigned char* abPQ, const unsigned char* abE, char **pszError);

#endif //RSAREAD_H