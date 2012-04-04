#include "all.h"
#include <sys/time.h>
#include <bitset>
/*
struct timeval {
  time_t tv_sec;
  suseconds_t tv_usec;
};
*/

//Get current time
long current_time(){
        
        struct timeval tv;
        struct timezone tz;
        struct tm *tm;
        gettimeofday(&tv,&tz);
        tm=localtime(&tv.tv_sec);
        long total=(tm->tm_hour*10000000000000000+ tm->tm_min*1000000000+tm->tm_sec*1000000+tv.tv_usec);
        //cout<<total<<endl;

}

typedef struct{
        int seq_NUM;
        int ack_NUM;
        char data[256];
} frame;

//Deconstruct Frame from PHY Layer
frame deconstruct_frame(string input){

	frame buffer; 
        int i=0;
	char temp[200];

	//Find  Sequence Num
        for (i=0;i<input.size();i++){
                if (input[i]=='\a'){
                        cout<<atoi(temp)<<endl;
			buffer.seq_NUM=atoi(temp);
                	cout<<"BREAK"<<endl;
                        break;
		}
                else
			temp[i]=input[i];
	}
        //Find ACK Number
	char temp2[200];//clear
	i++;
	int j=i;
        while(i<input.size()){
	        if (input[i]=='\a'){
                        cout<<atoi(temp2)<<endl;
                        buffer.ack_NUM=atoi(temp2);
                	cout<<"BREAK"<<endl;
			i++;
		        break;
		}
                else{
			temp2[i-j]=input[i];
			i++;
		}
	}
        //Find Message
        j=i;
	while(i<(input.size())){
        	cout<<input[i]<<":"<<(i-j)<<endl;
                buffer.data[i-j]=input[i];
		i++;
	}

	return buffer;
}




void pbits(std::string const& s) { 
    for(std::size_t i=0; i<s.size(); i++) 
            std::cout << std::bitset<CHAR_BIT>(s[i]) << " "; 
	    } 


int main(){

//frame frame1;

//string input=string("15")+'\a'+"27"+'\a'+"Some message of crap";
//cout<<input<<endl;

//frame1=deconstruct_frame(input);

//cout<<frame1.seq_NUM<<endl;
//cout<<frame1.ack_NUM<<endl;
//cout<<frame1.data<<endl;


//struct timeval tv;
//struct timezone tz;
//struct tm *tm;
//gettimeofday(&tv,&tz);
//tm=localtime(&tv.tv_sec);
//printf("StartTime: %d:%02d:%02d %d \n", tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec);

/*while(1){
cout<<current_time()<<endl;
sleep(1);
cout<<current_time()<<endl;
sleep(1);
}*/


//queue<string> phy_array[10];

//phy_array[0].push("Test1");

//cout<<"Array0: "<<phy_array[0].size()<<endl;
//cout<<"Array1: "<<phy_array[1].size()<<endl;


//int n = 123;

//char c[20];
//sprintf(c, "%d", n);

//cout<<c<<endl;

string input="This is a string";
//char *input="This is a string";
string tstr;

bitset<8> mybits=0;
bitset<8> mybits2=0;
int checksum = 0;
for(int i=0; i<input.size(); i++) {
	mybits=bitset<8>(input[i]);
	cout<<mybits<<endl;
	tstr=mybits.to_string<char,char_traits<char>,allocator<char> >();
	mybits2 = mybits2 ^ mybits;
}
cout<<"Total: "<<mybits2<<endl;
int mybit=0;
for (int i=0; i<8;i++){
mybit= mybit ^ mybits2[i];

}
cout<<mybit<<endl;


//input="This is a string";
input[1]++;
cout<<input<<endl;
cout<<input[input.size()-1]<<endl;
string input2=input;
if (input2.size () > 0)  input2.resize (input2.size () - 1);
cout<<input2<<endl;
checksum = 0;
mybits=0;
mybits2=0;

for(int i=0; i<input.size(); i++) {
//cout << bitset<8>(input[i]) << '\n';
	mybits=bitset<2>(input[i]);
	cout<<mybits<<endl;
	//cout<<(char)mybits.to_ulong()<<endl;
	tstr=mybits.to_string<char,char_traits<char>,allocator<char> >();
	mybits2 = mybits2 ^ mybits;
}
cout<<"Total: "<<mybits2<<endl;

mybit=0;
for (int i=0; i<2;i++){
mybit= mybit ^ mybits2[i];

}
cout<<mybit<<endl;
return 0;

}


