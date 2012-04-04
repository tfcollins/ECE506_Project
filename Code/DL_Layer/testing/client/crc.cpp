#include "all.h"

#include <bitset>


int main(){


	string str="\r1\b0\bACL";

	bitset<8> mybits=0;
	bitset<8> crc=0;

	for (int i=0;i<str.size();i++){

		mybits=bitset<8>(str[i]);
		for(int j=0;j<8;j++){
			crc= crc[0] ^ mybits[j];
			//cout<<crc<<endl;

		}

	}
	cout<<crc<<endl;
	cout<<crc[0]<<endl;

	int g=(int) crc[0];

	cout<<g<<endl;
return 0;

}

