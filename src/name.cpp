#include <iostream>
#include <cstring>
#include <stdlib.h>
#include "binfhecontext-ser.h"
#include "name.h"
#include "encode.h"

int encryptName(char* name)
{
	char temp[8] = {0};
	strncpy(temp , name , 8);
	int* pDName = (int*)calloc(40 , sizeof(int));
	decodeName(temp , pDName);
	std::cout << "Fetching key and cryptoContext ...\n";
	lbcrypto::LWEPrivateKey SecretKey;
	lbcrypto::BinFHEContext CryptoContext;
	if(lbcrypto::Serial::DeserializeFromFile(std::string("myKey") , SecretKey , lbcrypto::SerType::BINARY) == 0)
	{
		std::cerr << "Error getting key.\n";
		return 1;
	}
	if(lbcrypto::Serial::DeserializeFromFile(std::string("CC") , CryptoContext , lbcrypto::SerType::BINARY) == 0)
	{
		std::cerr << "Error getting CC.\n";
		return 2;
	}
	CryptoContext.BTKeyGen(SecretKey);
	std::cout << "Encrypting ...\n";
	lbcrypto::LWECiphertext DName[40];
	system("mkdir cts");
	char* fname = NULL;
	for(int i = 0 ; i < 40 ; i++)
	{
		DName[i] = CryptoContext.Encrypt(SecretKey , pDName[i]);
		asprintf(&fname , "cts/temp%d" , i);
		lbcrypto::Serial::SerializeToFile(fname , DName[i] , lbcrypto::SerType::BINARY);
	}
	//system("zip -r cts cts");
	//system("rm -f -R cts");
	std::cout << "Completed.\n";
	free(pDName);
	free(fname);
	return 0;
}
