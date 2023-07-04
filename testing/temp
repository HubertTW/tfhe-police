#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include "binfhecontext-ser.h"
#include "encode.h"
#include "fetchName.h"
#include "data.h"
#include <future>
#include "str_cmp.h"
#include <pthread.h>
#include "binfhecontext.h"

using namespace lbcrypto;

lbcrypto::LWECiphertext count[10];
pthread_t tid[8];
int numThreads = 8;
int dataIdx[8] = {0,1,2,3,4,5,6,7};
int blockSize;
int num;
std::vector <dataCipher_> tempdata;
pthread_mutex_t lock;

lbcrypto::LWEPrivateKey secretKey;
lbcrypto::BinFHEContext cryptoContext;
lbcrypto::LWECiphertext qNameCipher[8][5];



int counter(){

    DataBase mydb("encData");
    nameCipher mycipher;
    //system("unzip encData -d encData");
    mydb.fetch();
    fetchName(&mycipher);
    char* filename;

    lbcrypto::Serial::DeserializeFromFile("myKey" , secretKey , lbcrypto::SerType::BINARY);
    lbcrypto::Serial::DeserializeFromFile("CC" , cryptoContext , lbcrypto::SerType::BINARY);
    cryptoContext.BTKeyGen(secretKey);

   tempdata = mydb.get();
   num = tempdata.size();

    for(int i = 0;i <8 ;i++){
        for(int j = 0;j < 5;j++)
            qNameCipher[i][j] = mycipher.nc[i][j];
    }

    blockSize = (num - 1) / numThreads + 1;


    system("mkdir countResult");

    int *tinfo = dataIdx;
    int i = 0;
    int error;
    while (i < numThreads) {
     eval()
    }

    for (int t = 0; t < numThreads; t++){
        pthread_join(tid[t], NULL);
    }


    puts("serializing count to file... ");

    for (int bit = 0; bit < 10; bit++) {
        asprintf(&filename, "countResult/%d-count", bit);
        lbcrypto::Serial::SerializeToFile(filename, count[bit], lbcrypto::SerType::BINARY);
    }

    return 0;
}

void *eval() {
    for (int i = 0; i < 7; i++) {

        for (int b = 0; b < blockSize; b++) {

            if (b + i * blockSize >= num) {
                break;
            }
            auto cmpResult = str_comp(tempdata[b + i * blockSize].nameCipher, qNameCipher, secretKey,
                                      cryptoContext);


            auto carry = cmpResult;

            for (int bit = 0; bit < 10; bit++) {
                auto res = cryptoContext.EvalBinGate(XOR, count[bit], carry);
                carry = cryptoContext.EvalBinGate(AND, count[bit], carry);
                count[bit] = res;

            }

        }

    }

    return NULL;

}
