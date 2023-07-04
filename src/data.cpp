#include "data.h"
#include "encode.h"
#include <future>
#include "str_cmp.h"

#define DEBUG 0

DataBase::DataBase(const char* dirName)
{
	__dirName = std::string(dirName);
}

std::vector <dataCipher_> DataBase::get()
{
    return __dataCipher;
}

int DataBase::fetch()
{
    char* fileName = NULL;
    asprintf(&fileName , "%s/length" , __dirName.c_str());
    FILE* fptr = fopen(fileName , "r");
    int length = 0;
    fscanf(fptr , "%d" , &length);
    for(int i = 0 ; i < length ; i++)
    {
        dataCipher_ temp;
        for(int j = 0 ; j < 40 ; j++)
        {
            asprintf(&fileName , "encData/%dN%02d" , i , j);
            lbcrypto::Serial::DeserializeFromFile(fileName , temp.nameCipher[j / 5][j % 5] , lbcrypto::SerType::BINARY);
        }
        for(int j = 0 ; j < 3 ; j++)
        {
            asprintf(&fileName , "encData/%dC%02d" , i , j);
            lbcrypto::Serial::DeserializeFromFile(fileName , temp.caseCipher[j] , lbcrypto::SerType::BINARY);
        }
        for(int j = 0 ; j < 13 ; j++)
        {
            asprintf(&fileName , "encData/%dT%02d" , i , j);
            lbcrypto::Serial::DeserializeFromFile(fileName , temp.timeCipher[j] , lbcrypto::SerType::BINARY);
        }
        for(int j = 0 ; j < 8 ; j++)
        {
            asprintf(&fileName , "encData/%dL%02d" , i , j);
            lbcrypto::Serial::DeserializeFromFile(fileName , temp.locationCipher[j] , lbcrypto::SerType::BINARY);
        }
        __dataCipher.push_back(temp);
    }
    return 0;
}

/*
int encrypt(lbcrypto::LWEPrivateKey sk , lbcrypto::BinFHEContext cc , const char* fileName , const char* dirName)
{
	std::vector <data_> data;
	FILE* fptr = fopen(fileName , "r");
	if(fptr == nullptr)
	{
		std::cout << "Error when open file " << fileName << "\n";
		return 0;
	}
	for(int i = 0 , c = 0 ; (c = fgetc(fptr)) != EOF ; i++)
	{
		data_ temp;
		for(int j = 0 ; j < 9 ; j++)
		{
			temp.name[j] = 0;
		}
		for(int j = 0 ; c != ',' ; j++)
		{
			temp.name[j] = c;
			c = fgetc(fptr);
		}
		fscanf(fptr , "%d,%d,%d\n" , &(temp.caseNum) , &(temp.location) , &(temp.time));
		data.push_back(temp);
	}

	fclose(fptr);

	#if DEBUG

	for(auto i = data.begin() ; i != data.end() ; i++)
	{
		std::cout << i -> name << "\t";
		std::cout << i -> caseNum << "  ";
		std::cout << i -> location << "\t";
		std::cout << i -> time << "\n";
	}

	#endif

	cc.BTKeyGen(sk);
	char* temp;
	asprintf(&temp , "mkdir %s" , dirName);
	system(temp);
	asprintf(&temp , "%s/length" , dirName);
	fptr = fopen(temp , "wb");
	fprintf(fptr , "%zu" , data.size());
	fclose(fptr);

	int thread = 16;
	std::vector <std::future <bool>> threads;
	int blockSize = (data.size() - 1) / thread + 1;

	#if DEBUG

	std::cout << "Data size : " << data.size() << "\n";
	std::cout << "Block size : " << blockSize << "\n";

	#endif

	for(int i = 0 ; i < thread ; i++)
	{
		threads.push_back(std::async([&](int i)
		{
			lbcrypto::LWECiphertext tempCipher;
			char* filename = NULL;
			int decTime[13] = {0};
			int decCase[3] = {0};
			int decName[40] = {0};
			int decLocation[8] = {0};
			for(int b = 0 ; b < blockSize ; b++)
			{
				if(b + i * blockSize >= data.size())
				{
					break;
				}
				// std::cout << b + i * blockSize << " ";
				decodeName(data[b + i * blockSize].name , decName);
				for(int j = 0 ; j < 40 ; j++)
				{
					asprintf(&filename , "%s/%dN%02d" , dirName , b + i * blockSize , j);
					tempCipher = cc.Encrypt(sk , decName[j]);
					lbcrypto::Serial::SerializeToFile(filename , tempCipher , lbcrypto::SerType::BINARY);
				}
				decodeCase(data[b + i * blockSize].caseNum , decCase);
				for(int j = 0 ; j < 3 ; j++)
				{
					asprintf(&filename , "%s/%dC%02d" , dirName , b + i * blockSize , j);
					tempCipher = cc.Encrypt(sk , decCase[j]);
					lbcrypto::Serial::SerializeToFile(filename , tempCipher , lbcrypto::SerType::BINARY);
				}
				decodeTime(data[b + i * blockSize].time , decTime);
				for(int j = 0 ; j < 13 ; j++)
				{
					asprintf(&filename , "%s/%dT%02d" , dirName , b + i * blockSize , j);
					tempCipher = cc.Encrypt(sk , decTime[j]);
					lbcrypto::Serial::SerializeToFile(filename , tempCipher , lbcrypto::SerType::BINARY);
				}
				decodeLocation(data[b + i * blockSize].location , decLocation);
				for(int j = 0 ; j < 8 ; j++)
				{
					asprintf(&filename , "%s/%dL%02d" , dirName , b + i * blockSize , j);
					tempCipher = cc.Encrypt(sk , decLocation[j]);
					lbcrypto::Serial::SerializeToFile(filename , tempCipher , lbcrypto::SerType::BINARY);
				}
			}
			return true;
		} , i));		
	}

	for(int i = 0 ; i < 8 ; i++)
	{
		threads[i].get();
	}

	return 0;
}
*/

/*
#if 0

int DataBase::query(lbcrypto::LWEPrivateKey sk , lbcrypto::BinFHEContext cc , const char* dirName)
{
	char* fileName = NULL;
	bool check = false;
	lbcrypto::LWECiphertext tempCipher;
	for(int i = 0 ; i < __dataCipher.size() ; i++)
	{
		for(int j = 0 ; j < 8 ; j++)
		{
			for(int k = 0 ; k < 8 ; k++)
			{
				asprintf(&fileName , "%s/%dN%02d" , __dirName.c_str() , i , j * 8 + k);
				check = lbcrypto::Serial::DeserializeFromFile(fileName , __dataCipher[i].nameCipher[j][k] , lbcrypto::SerType::BINARY);
				if(check == false)
				{
					std::cout << "Error when deserial file, name :" << fileName;
				}
			}
		}
		for(int j = 0 ; j < 8 ; j++)
		{
			asprintf(&fileName , "%s/%dC%02d" , __dirName.c_str() , i , j);
			check = lbcrypto::Serial::DeserializeFromFile(fileName , __dataCipher[i].caseCipher[j] , lbcrypto::SerType::BINARY);
			if(check == false)
			{
				std::cout << "Error when deserial file, name :" << fileName;
			}
		}
		for(int j = 0 ; j < 32 ; j++)
		{
			asprintf(&fileName , "%s/%dT%02d" , __dirName.c_str() , i , j);
			check = lbcrypto::Serial::DeserializeFromFile(fileName , __dataCipher[i].timeCipher[j] , lbcrypto::SerType::BINARY);
			if(check == false)
			{
				std::cout << "Error when deserial file, name :" << fileName;
			}
		}
	}
	char* command = NULL;
	asprintf(&command , "unzip %s.zip" , dirName);
	system(command);

	lbcrypto::LWECiphertext qNameCipher[8][8];

	for(int i = 0 ; i < 8 ; i++)
	{
		for(int j = 0 ; j < 8 ; j++)
		{
			asprintf(&fileName , "%s/temp%d" , dirName , i * 8 + j);
			check = lbcrypto::Serial::DeserializeFromFile(fileName , qNameCipher[i][j] , lbcrypto::SerType::BINARY);
			if(check == false)
			{
				std::cout << "Error when deserial file, name :" << fileName << "\n";
			}
		}
	}
	asprintf(&command , "rm -f -R %s" , dirName);
	system(command);

	cc.BTKeyGen(sk);

	std::vector <std::future <bool>> threads;

	int blockSize = (__dataCipher.size() - 1) / __threads;

	system("mkdir result");
	
	for(int i = 0 ; i < __threads ; i++)
	{
		threads.push_back(std::async([&](int i)
		{
			lbcrypto::LWECiphertext checkCipher;
			lbcrypto::LWECiphertext tempCipher;
			for(int b = 0 ; b < blockSize ; b++)
			{
				if(b + i * blockSize >= __dataCipher.size())
				{
					break;
				}
				checkCipher = str_comp(__dataCipher[b + i * blockSize].nameCipher , qNameCipher , sk , cc);
				for(int j = 0 ; j < 64 ; j++)
				{
					tempCipher = cc.EvalBinGate(AND , checkCipher , __dataCipher[b + i * blockSize].nameCipher[j / 8][j % 8]);
					asprintf(&fileName , "result/%dN%02d" , b + i * blockSize , j);
					lbcrypto::Serial::SerializeToFile(fileName , tempCipher , lbcrypto::SerType::BINARY);
				}
				for(int j = 0 ; j < 8 ; j++)
				{
					tempCipher = cc.EvalBinGate(AND , checkCipher , __dataCipher[b + i * blockSize].caseCipher[j]);
					asprintf(&fileName , "result/%dC%02d" , b + i * blockSize , j);
					lbcrypto::Serial::SerializeToFile(fileName , tempCipher , lbcrypto::SerType::BINARY);
				}
				for(int j = 0 ; j < 32 ; j++)
				{
					tempCipher = cc.EvalBinGate(AND , checkCipher , __dataCipher[b + i * blockSize].caseCipher[j]);
					asprintf(&fileName , "result/%dT%02d" , b + i * blockSize , j);
					lbcrypto::Serial::SerializeToFile(fileName , tempCipher , lbcrypto::SerType::BINARY);
				}
			}
			return true;
		} , i));		
	}

	for(int i = 0 ; i < 8 ; i++)
	{
		threads[i].get();
	}

	return 0;
}



#endif
*/