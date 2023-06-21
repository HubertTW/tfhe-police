#include <iostream>
#include "binfhecontext-ser.h"
#include "encode.h"
#include "data.h"
#include <future>
#include <getopt.h>
#include <stdlib.h>

void help();
int keygen();
int encrypt(const char* dirName);
int decrypt(const char* dirNname);

int main(int argc , char* argv[])
{
	int c = 0;
	option opt[] = 
	{
		{"help" , 0 , NULL , 'h'} , 
		{"keygen" , 0 , NULL , 'k'} , 
		{"name" , 1 , NULL , 'n'} , 
		{"decrypt" , 1 , NULL , 'd'} , 
        {"encrypt" , 1 , NULL , 'e'}
	};
	while((c = getopt_long(argc , argv , "hn:d:ke:" , opt , NULL)) != -1)
	{
		switch(c)
		{
			case 'h':
				help();
				break;
			case 'k':
				keygen();
				break;
			case 'n':
				// encrypt(optarg);
				break;
			case 'd':
				decrypt(optarg);
				break;
            case 'e':
				encrypt(optarg);
				break;
			case '?':
				std::cout << "unknown argument.\n";
				break;
			default:
				std::cout << "unknown error.\n";
		}
	}
	return 0;
}

void help()
{
	std::cout << "Usage: ./cleint [options] ...\n";
	std::cout << "Options:\n";
	std::cout << "\t-h, --help\t\tdisplay this help and exit.\n";
	std::cout << "\t-k, --keygen\t\tgenerate a key and save in \"./myKey\".\n";
	std::cout << "\t-n, --name NAME\t\tgenerate a file that can query by NAME(at most 8 characters). It also generate a file for decryption named \"CC\".\n";
	std::cout << "\t-d, --decrypt FILENAME\tdecrypt the result which is named FILENAME by \"./myKey\" and show the result.\n";
	return;
}

int keygen()
{
	std::cout << "Start generating the key...\n";
	auto CryptoContext = lbcrypto::BinFHEContext();
	CryptoContext.GenerateBinFHEContext(lbcrypto::MEDIUM);
	auto SecretKey = CryptoContext.KeyGen();
	std::cout << "Start saving the key...\n";
	if(lbcrypto::Serial::SerializeToFile(std::string("myKey") , SecretKey , lbcrypto::SerType::BINARY) == 0)
	{
		std::cerr << "Error saving key.\n";
		return 1;
	}
	if(lbcrypto::Serial::SerializeToFile(std::string("CC") , CryptoContext , lbcrypto::SerType::BINARY) == 0)
	{
		std::cerr << "Error saving CC.\n";
		return 2;
	}
	std::cout << "Completed.\n";
	return 0;
}

int encrypt(const char* dirName)
{
	lbcrypto::LWEPrivateKey sk;
	lbcrypto::BinFHEContext cc;
	lbcrypto::Serial::DeserializeFromFile("myKey" , sk , lbcrypto::SerType::BINARY);
	lbcrypto::Serial::DeserializeFromFile("CC" , cc , lbcrypto::SerType::BINARY);
	std::vector <data_> data;
	FILE* fptr = fopen("data.csv" , "r");
	if(fptr == nullptr)
	{
		std::cout << "Error when open file " << "data.csv" << "\n";
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

int decrypt(const char* dirNname)
{
    return 0;
}
