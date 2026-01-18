#include "SD.h"

#include "main.h"



void SD_card_Init()
{





}













void file_write_float(TCHAR* filename,float* data,int length){
	FIL file;
	FRESULT res = f_open(&file,filename,FA_OPEN_ALWAYS|FA_WRITE|FA_READ);
	if(res == FR_OK){
		UINT bw=0;
		for(uint16_t i = 0;i<length;i++){
			char text[40];
			sprintf(text,"%f\n",data[i]);
			//f_printf(&file,"data=%f\n",data[i]);
			f_puts(text,&file);
		}
		printf("write OK\r\n");
	}else{
		printf("open file fail:%d\r\n",res);
	}
	f_close(&file);
}

void file_read_float(TCHAR* filename,float* data,int length){
	FIL file;
	FRESULT res = f_open(&file,filename,FA_OPEN_ALWAYS|FA_WRITE|FA_READ);
	if(res == FR_OK){
		UINT bw=0;
		char text[40];
		for(uint16_t i = 0;i<length;i++){
			f_gets(text,40,&file);
			sscanf(text,"%f\n",&data[i]);
		}
//		for(int i=0;i<length;i++){
//			printf("%f\r\n",data[i]);
//		}
		printf("read OK\r\n");
	}else{
		printf("open file fail:%d\r\n",res);
	}
	f_close(&file);
}



//    file_write_float("s11o.txt",adc,4096);
//    
//    file_read_float("s11o.txt",bbb,4096);






