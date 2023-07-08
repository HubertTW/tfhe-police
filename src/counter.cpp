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

void *eval(void *);

lbcrypto::LWECiphertext count[10];
pthread_t tid[8];
pthread_mutex_t lock;
int numThreads = 8;
int dataIdx[8] = {0,1,2,3,4,5,6,7};
int blockSize;
int num;

std::vector <dataCipher_> tempdata;
lbcrypto::LWEPrivateKey secretKey;
lbcrypto::BinFHEContext cryptoContext;
lbcrypto::LWECiphertext qNameCipher[8][5];



int counter(){

    DataBase mydb("encData");
    nameCipher mycipher;
    mydb.fetch();
    fetchName(&mycipher);


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

    /* initialize the count as 0 */
    for (int i = 0;i < 10; i++ ){
        count[i] = cryptoContext.Encrypt(secretKey, 0);
    }


    system("mkdir countResult");

    puts("start multi-processing...");

/*
    int ret;
    pthread_attr_t attr;
    size_t stacksize = 102400;
    ret = pthread_attr_init(&attr);
    if (ret != 0) {
        fprintf(stderr, "Error: pthread_attr_init\n");
    }

    ret = pthread_attr_setstacksize(&attr, stacksize);
    if (ret != 0) {
        fprintf(stderr, "Error: pthread_attr_setstacksize\n");
    }

*/

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return 1;
    }

    int *tinfo = dataIdx;
    int i = 0;
    int error;
    while (i < numThreads) {
        error = pthread_create(&(tid[i]),
                               NULL,
                               &eval, (void*)&tinfo[i]);
        if (error != 0)
            printf("\nThread can't be created :[%s]",
                   strerror(error));
        i++;
    }


    for (int t = 0; t < numThreads; t++) {
        if (pthread_join(tid[t], NULL) != 0) {
            perror("pthread_join() error");
            exit(3);

        }
    }
    pthread_mutex_destroy(&lock);


   // pthread_attr_destroy(&attr);

    puts("serializing count to file... ");

    char* filename;
    for (int bit = 0; bit < 10; bit++) {
        asprintf(&filename, "countResult/%d-count", bit);
        lbcrypto::Serial::SerializeToFile(filename, count[bit], lbcrypto::SerType::BINARY);
    }

    return 0;
}

void *eval(void *arg) {

    int *data = (int *) arg;

    printf("thread %d is running\n", data[0]);

    for (int b = 0; b < blockSize; b++) {

        if (b + data[0] * blockSize >= num) {
            break;
        }
        auto cmpResult = str_comp(tempdata[b + data[0] * blockSize].nameCipher, qNameCipher, secretKey, cryptoContext);


        pthread_mutex_lock(&lock);
        auto carry = cmpResult;
        for (int bit = 0; bit < 10; bit++) {
            auto res = cryptoContext.EvalBinGate(XOR, count[bit], carry);
            carry = cryptoContext.EvalBinGate(AND, count[bit], carry);
            count[bit] = res;
        }
        pthread_mutex_unlock(&lock);


        printf("thread %d finished\n", data[0]);

        /* this is fault ! */
        //pthread_exit(NULL);



    }
    return NULL;
}
