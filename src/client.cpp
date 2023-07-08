#include <iostream>
#include "binfhecontext-ser.h"
#include "encode.h"
#include "data.h"
#include "name.h"
#include <future>
#include <getopt.h>
#include <stdlib.h>

void help();
int keygen();
int encrypt(const char*);
void decryptCount();

int main(int argc , char* argv[])
{
	int c = 0;
	option opt[] = 
	{
		{"keygen" , 0 , NULL , 'k'} , 
		{"encryptName" , 1 , NULL , 'n'} ,
        {"encrypt" , 1 , NULL , 'e'},
        {"decryptCount" , 1 , NULL , 'd'}
	};
	while((c = getopt_long(argc , argv , "hn:dke:" , opt , NULL)) != -1)
	{
		switch(c)
		{

			case 'k':
				keygen();
				break;
			case 'n':
				encryptName(optarg);
				break;
            case 'e':
				encrypt(optarg);
				break;
            case 'd':
                decryptCount();
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



void decryptCount() {


    lbcrypto::LWEPrivateKey sk;
    lbcrypto::BinFHEContext cc;
    lbcrypto::Serial::DeserializeFromFile("myKey" , sk , lbcrypto::SerType::BINARY);
    lbcrypto::Serial::DeserializeFromFile("CC" , cc , lbcrypto::SerType::BINARY);
    lbcrypto::LWECiphertext countCipher[10];
    lbcrypto::LWEPlaintext count[10];
    int sum   = 0;
    int power = 1; /* 2^0 */
    char* filename;


    puts("start decrypting...");
    puts("the binary result is: ");
    for (int bit = 0; bit < 10 ; bit++) {

        asprintf(&filename, "countResult/%d-count", bit);
        lbcrypto::Serial::DeserializeFromFile(filename, countCipher[bit], lbcrypto::SerType::BINARY);

        cc.Decrypt(sk, countCipher[bit], &count[bit]);
        std::cout << count[bit] <<"";
        sum += (power * count[bit]);
        power = power * 2;

    }

    std::cout << "\nthe count result (decimal) is : " << sum << std::endl;


}

int keygen()
{
	std::cout << "Start generating the key...\n";
	auto CryptoContext = lbcrypto::BinFHEContext();
	CryptoContext.GenerateBinFHEContext(lbcrypto::TOY);
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
	FILE* fptr = fopen("../../testing/data.csv" , "r");
	if(fptr == nullptr)
	{
		std::cout << "Error when open file " << "data.csv" << "\n";
		return 0;
	}
    for(int c = 0; (c = fgetc(fptr)) != EOF;)
	{
		data_ temp;

        /* padding */
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


