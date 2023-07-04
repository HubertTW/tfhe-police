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
	lbcrypto::LWECiphertext tempCaseCipher[num][3];
	lbcrypto::LWECiphertext tempLocCipher[num][8];
	lbcrypto::LWECiphertext tempTimeCipher[num][13];
	lbcrypto::LWECiphertext qNameCipher[8][5];
	for(int i = 0;i <8 ;i++){
		for(int j = 0;j < 5;j++)
			qNameCipher[i][j] = mycipher.nc[i][j];
	}
	
	
	int thread = 8;
	std::vector <std::future <bool>> threads;
	int blockSize = (num - 1) / thread + 1;
    lbcrypto::LWECiphertext cmpResult;


    puts("start string comparing....");
    system("mkdir cmpResults");
    system("mkdir evalData");



	for(int row = 0 ; row < thread ; row++)
	{
		threads.push_back(std::async([&](int row) {
			for(int b = 0 ; b < blockSize ; b++){
				if(b + row * blockSize >= num)
				{
					break;
				}
				auto cmpResult = str_comp(tempdata[b + row * blockSize].nameCipher , qNameCipher , secretKey , cryptoContext);
			
                		asprintf(&filename , "cmpResults/%d-cmpResult" , b + row * blockSize);
                		lbcrypto::Serial::SerializeToFile(filename , cmpResult , lbcrypto::SerType::BINARY);
				for(int byte = 0 ; byte < 8 ; byte++)	
				{
					for(int bit = 0 ; bit < 5 ; bit++)
					{
						tempNameCipher[b + row * blockSize][byte][bit] = cryptoContext.EvalBinGate(AND , cmpResult, tempdata[b + row * blockSize].nameCipher[byte][bit]);
			
                		                asprintf(&filename , "evalData/%d-name-%d-%d" ,b + row * blockSize, byte, bit);
                		                lbcrypto::Serial::SerializeToFile(filename , tempNameCipher[b + row * blockSize][byte][bit] , lbcrypto::SerType::BINARY);
						
					}
				}
				for(int bit = 0 ; bit < 3 ; bit++)
				{
					tempCaseCipher[b + row * blockSize][bit] = cryptoContext.EvalBinGate(AND , cmpResult, tempdata[b + row * blockSize].caseCipher[bit]);
		
                		        asprintf(&filename , "evalData/%d-case-%d" , b + row * blockSize, bit);
                		        lbcrypto::Serial::SerializeToFile(filename , tempCaseCipher[b + row * blockSize][bit]  , lbcrypto::SerType::BINARY);
				
				}
				for(int bit = 0 ; bit < 8 ; bit++)
				{
					tempLocCipher[b + row * blockSize][bit] = cryptoContext.EvalBinGate(AND , cmpResult, tempdata[b + row * blockSize].locationCipher[bit]);
		
        		                asprintf(&filename , "evalData/%d-location-%d" , b + row * blockSize, bit);
        		                lbcrypto::Serial::SerializeToFile(filename , tempLocCipher[b + row * blockSize][bit] , lbcrypto::SerType::BINARY);
					
				}
				for(int bit = 0 ; bit < 13 ; bit++)
				{
					tempTimeCipher[b + row * blockSize][bit] = cryptoContext.EvalBinGate(AND , cmpResult, tempdata[b + row * blockSize].timeCipher[bit]);
		
        		                asprintf(&filename , "evalData/%d-time-%d" ,b + row * blockSize, bit);
        		                lbcrypto::Serial::SerializeToFile(filename , tempTimeCipher[b + row * blockSize][bit] , lbcrypto::SerType::BINARY);
        		                
				}
			}
			return true;
		}, row));
	}

	for(int row = 0; row < thread; ++row) {
                threads[row].get();
        }
	return 0;
}
