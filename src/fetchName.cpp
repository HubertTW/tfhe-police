#include <iostream>
#include <cstring>
#include <stdlib.h>
#include "binfhecontext-ser.h"
#include "encode.h"
#include "fetchName.h"

int fetchName(nameCipher* myCipher){
	system("unzip cts.zip -d cts");
	char* filename;
	bool check = 0;
	for(int i = 0 ; i < 8 ; i++)
	{
		for(int j = 0 ; j < 5 ; j++)
		{
			asprintf(&filename , "cts/temp%d" , i * 5 + j);
			check = lbcrypto::Serial::DeserializeFromFile(filename , myCipher->nc[i][j] , lbcrypto::SerType::BINARY);
			if(check == false)
			{
				std::cout << "Error when deserial file, name :" << filename << "\n";
			}
		}
	}
	free(filename);
	//system("rm -f -R cts");
	return 0;
}
