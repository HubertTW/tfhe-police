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
#include "binfhecontext.h"

using namespace lbcrypto;

int query(){

    DataBase mydb("encData");
    nameCipher mycipher;
    system("unzip encData -d encData");
    mydb.fetch();
    fetchName(&mycipher);
    char* filename;

    lbcrypto::LWEPrivateKey secretKey;
    lbcrypto::BinFHEContext cryptoContext;
    lbcrypto::Serial::DeserializeFromFile("myKey" , secretKey , lbcrypto::SerType::BINARY);
    lbcrypto::Serial::DeserializeFromFile("CC" , cryptoContext , lbcrypto::SerType::BINARY);
    cryptoContext.BTKeyGen(secretKey);

    std::vector<dataCipher_> tempdata = mydb.get();
    const int num = tempdata.size();
    lbcrypto::LWECiphertext tempNameCipher[num][8][5];
    lbcrypto::LWECiphertext qNameCipher[8][5];
    for(int i = 0;i <8 ;i++){
        for(int j = 0;j < 5;j++)
            qNameCipher[i][j] = mycipher.nc[i][j];
    }


    int thread = 8;
    std::vector <std::future <bool>> threads;
    int blockSize = (num - 1) / thread + 1;
    lbcrypto::LWECiphertext cmpResult;
    lbcrypto::LWECiphertext count;



    puts("start string comparing....");
    system("mkdir cmpResults");
    system("mkdir countResult");



    for(int row = 0 ; row < thread ; row++)
    {
        threads.push_back(std::async([&](int row) {
            for(int b = 0 ; b < blockSize ; b++){
                if(b + row * blockSize >= num)
                {
                    break;
                }
                auto cmpResult = str_comp(tempdata[b + row * blockSize].nameCipher , qNameCipher , secretKey , cryptoContext);

                lbcrypto::LWECiphertext count[10];

                 auto carry = cmpResult;
                 for(int i = 0; i < 10; i++) {
                        auto res = cryptoContext.EvalBinGate(XOR, count[i], carry);
                        carry    = cryptoContext.EvalBinGate(AND, count[i], carry);
                        count[i] = res;
                 }

                asprintf(&filename , "countResult/%d-count" , b + row * blockSize);
                lbcrypto::Serial::SerializeToFile(filename , count , lbcrypto::SerType::BINARY);

            }
            return true;
        }, row));
    }

    for(int row = 0; row < thread; ++row) {
        threads[row].get();
    }
    return 0;
}
