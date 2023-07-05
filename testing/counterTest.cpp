#include "binfhecontext.h"

using namespace lbcrypto;

int main() {
    // Sample Program: Step 1: Set CryptoContext

    auto cc = BinFHEContext();

    cc.GenerateBinFHEContext(STD128);

    // Generate the secret key
    auto sk = cc.KeyGen();

    std::cout << "Generating the bootstrapping keys..." << std::endl;

    // Generate the bootstrapping keys (refresh and switching keys)
    cc.BTKeyGen(sk);

    std::cout << "Completed the key generation." << std::endl;

    lbcrypto::LWECiphertext count[8];/* cmpResult */
    for (int i = 0;i < 8; i++ ){
        count[i] = cc.Encrypt(sk, 0);
    }

    bool data[8] = {1,0,0,0,0,1,1,1};
    lbcrypto::LWECiphertext cmpRes[8];

    for (int i = 0; i < 8; i++ ) {
        cmpRes[i] = cc.Encrypt(sk, data[i]);


        printf("count %d", i);
        auto carry = cmpRes[i];
        for (int lsb = 0; lsb < 8; lsb++) {
            auto res = cc.EvalBinGate(XOR, count[lsb], carry);
            carry = cc.EvalBinGate(AND, count[lsb], carry); /* once the temp = false, it's always false */
            count[lsb] = res;
        }

    }



        puts("eval completed");

    

        /* client  only */

        LWEPlaintext result[8];
        int sum   = 0;
        int power = 1; /* 2^0 */
        for (int lsb = 0; lsb < 8; lsb++) {
            cc.Decrypt(sk, count[lsb], &result[lsb]);
            sum += (power * result[lsb]);

            power = power * 2;
        }
        std::cout << "the sum is : " << sum << std::endl;

        return 0;
    }


