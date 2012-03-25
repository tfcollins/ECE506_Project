#include "all.h"
#include <sys/time.h>

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

while(1){
cout<<current_time()<<endl;
sleep(1);
cout<<current_time()<<endl;
sleep(1);
}


int n = 123;

char c[20];
sprintf(c, "%d", n);

cout<<c<<endl;

return 0;

}


