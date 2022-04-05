#define SHA256_INPUT_SIZE	160 
#define SHA256_RESULT_SIZE  32
#define SHA256_TARGET_SIZE  32

using namespace std;

// include files
#include <iostream>
#include <stdlib.h>
#include <CL/cl.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <limits.h>
#include <bits/stdc++.h>


static size_t global_work_size = 1; // 
static size_t local_work_size = 1; //
static cl_platform_id platform_id = NULL;
static cl_device_id device_id = NULL;
static cl_context context;
static cl_int clerr;
static cl_int kernerr;
static cl_program program;
static cl_kernel kernel;
static cl_command_queue command_queue;
static cl_mem buffer_input, buffer_output, buffer_info, buffer_target, buffer_outheader;
static cl_mem inp_device, op_device, tgt_device, op_device2;
static cl_uint *ptr_op_device;
static char *ptr_inp_device, *ptr_op_device2;
static cl_uint *tgt_inp_device;
static unsigned int inp_size;
static unsigned int tgt_size;

unsigned char* LoadBinProgramFromFile(const char* fileName,size_t * binary_length) 
{
    *binary_length = 0;
    FILE* fp = fopen(fileName, "rb");
    if (fp == NULL) 
    {
        printf("Failed to open \"%s\" file (fopen).\n",fileName);
        return (unsigned char *) '\0';
    }
    fseek(fp, 0, SEEK_END);
    *binary_length = ftell(fp);
    unsigned char *KernelBin = (unsigned char*) malloc(sizeof(unsigned char) * *binary_length);
    if(KernelBin==0) 
    {
        printf("Malloc Failed!!\n");
        return (unsigned char *) '\0';
    }
    rewind(fp);
    if (fread((void*)KernelBin, *binary_length, 1, fp) == 0) 
    {
        printf("Failed to read from \"%s\" file (fread).\n",fileName);
        return (unsigned char *) '\0';
    }
    fclose(fp);
    return KernelBin;
}
void initDevice()
{
	 cl_uint err_devices;
   cl_uint err_platforms;
   clerr = clGetPlatformIDs(1, &platform_id, &err_platforms); 
   clerr = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, 1, &device_id, &err_devices); 
   context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &clerr);  
   
}
void initKernel()
{

	size_t CodeBinSize;
	unsigned char *bin = NULL;
	bin = LoadBinProgramFromFile("sha256_kernel.aocx", &CodeBinSize);
  program = clCreateProgramWithBinary(context, 1, &device_id, &CodeBinSize, (const unsigned char **) &bin, &kernerr, &clerr); 
	clerr = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	kernel = clCreateKernel(program, "sha256_kernel", &clerr);
  
}
void initBuffers()
{
	command_queue = clCreateCommandQueue(context, device_id, 0, &clerr);
	
  inp_device = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, SHA256_INPUT_SIZE, NULL, &clerr);
  tgt_device = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, SHA256_TARGET_SIZE, NULL, &clerr);
	
	ptr_inp_device =  (char*)clEnqueueMapBuffer(command_queue, inp_device, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, SHA256_INPUT_SIZE, 0, NULL, NULL, &clerr);
	tgt_inp_device =  (cl_uint*)clEnqueueMapBuffer(command_queue, tgt_device, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, SHA256_TARGET_SIZE, 0, NULL, NULL, &clerr);
  
	
	memset(ptr_inp_device, 0, SHA256_INPUT_SIZE);
	memset(tgt_inp_device, 0, SHA256_TARGET_SIZE);
	
	op_device = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, SHA256_RESULT_SIZE, NULL, &clerr);
  op_device2 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, SHA256_INPUT_SIZE, NULL, &clerr);

	ptr_op_device = (cl_uint *) clEnqueueMapBuffer(command_queue, op_device, CL_TRUE, CL_MAP_READ, 0, SHA256_RESULT_SIZE, 0, NULL, NULL, &clerr);
  ptr_op_device2 = (char*) clEnqueueMapBuffer(command_queue, op_device2, CL_TRUE, CL_MAP_READ, 0, SHA256_INPUT_SIZE, 0, NULL, NULL, &clerr);
 

	memset(ptr_op_device, 0, SHA256_RESULT_SIZE);
  memset(ptr_op_device2, 0, SHA256_INPUT_SIZE);
 
 
  //buffers for input data
  buffer_input = clCreateBuffer(context, CL_MEM_READ_ONLY, SHA256_INPUT_SIZE, NULL, &clerr);
  buffer_target = clCreateBuffer(context, CL_MEM_READ_ONLY, SHA256_TARGET_SIZE, NULL, &clerr);
	buffer_output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, SHA256_RESULT_SIZE, NULL, &clerr);
	buffer_info = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(unsigned int), NULL, &clerr);
  buffer_outheader = clCreateBuffer(context, CL_MEM_WRITE_ONLY, SHA256_INPUT_SIZE, NULL, &clerr);
  
  clSetKernelArg(kernel, 0, sizeof(buffer_input), (void *) &buffer_input);
  clSetKernelArg(kernel, 1, sizeof(buffer_target), (void *) &buffer_target);
	clSetKernelArg(kernel, 2, sizeof(buffer_output), (void *) &buffer_output);
  clSetKernelArg(kernel, 3, sizeof(buffer_info), (void *) &buffer_info);  
  clSetKernelArg(kernel, 4, sizeof(buffer_outheader), (void *) &buffer_outheader);  	
}
void performHash()
{
	clerr = clEnqueueWriteBuffer(command_queue, buffer_info, CL_TRUE, 0, sizeof(unsigned int), &inp_size, 0, NULL, NULL);
	clerr = clEnqueueWriteBuffer(command_queue, buffer_input, CL_TRUE, 0, SHA256_INPUT_SIZE, ptr_inp_device, 0, NULL, NULL);
	clerr = clEnqueueWriteBuffer(command_queue, buffer_target, CL_TRUE, 0, SHA256_TARGET_SIZE, tgt_inp_device, 0, NULL, NULL);
	
  clerr = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_work_size, &local_work_size, 0, NULL, NULL);
	
	clerr = clFinish(command_queue);
	clerr = clEnqueueReadBuffer(command_queue, buffer_output, CL_TRUE, 0, SHA256_RESULT_SIZE, ptr_op_device, 0, NULL, NULL);   
  clerr = clEnqueueReadBuffer(command_queue, buffer_outheader, CL_TRUE, 0, SHA256_INPUT_SIZE, ptr_op_device2, 0, NULL, NULL); 
	
}

void dec_to_hex(int n, char hex[9]);
int target(int exp, char bits[9]);
void targetToUint(char hex[65], uint * o);

int main () 
{

  time_t start, end;
	
	char header[161];                                                                              // block header
	char version[9]="02000000";																		                                  // version block
	char previous_hash[65] = "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b";	  // previous hash block
	char merkle_root[65] = "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b";		  // merkle root block
	char time_hex[9] = "00000000";																				                          // time block
	char bits_hex[9]= "00000000";																				                            // target
	char nonce_hex[9]= "00000000";																			                            // nonce  
  char result[65];
	char target_hex[65];
	int exp =0;
  
 result[31] ='\0';
  unsigned int bits = 386736012 ;	// target 386736012
	unsigned long int nonce = 0;		    // nonce start at 0
 
 // init target to 0
  for(int i =0; i < 64; i++)
	 target_hex[i] = '0';
	target_hex[64] = '\0';
	
 
 // init header to 0
	for(int i =0; i < 159; i++)
	 header[i] = '0';
	header[159] = '\0';
   
  //get time stamp
	time_t t = time(0);  // t is an integer type

  dec_to_hex(t, time_hex);       // convert time to hex
  dec_to_hex(bits, bits_hex);		 //convert bits to hex
 
 //convert target to expanded hex
  exp = target(exp, bits_hex);
  int startInd = 64-(2*exp);
  target_hex[startInd]= bits_hex[2];
  target_hex[startInd+1]= bits_hex[3];
  target_hex[startInd+2]= bits_hex[4];
  target_hex[startInd+3]= bits_hex[5];
  target_hex[startInd+4]= bits_hex[6];
  target_hex[startInd+5]= bits_hex[7];
  

	dec_to_hex(nonce, nonce_hex);  // convert nonce to hex
  
  // create header
 	strcpy(header, version);
	strcat(header, previous_hash);
	strcat(header, merkle_root);
	strcat(header, time_hex);
	strcat(header, bits_hex);
	strcat(header, nonce_hex);

  initDevice();
  initKernel();
  initBuffers();

 	inp_size = strlen(header);  // get length of header
  tgt_size = strlen(target_hex); // get length of target

  unsigned int * o_hex = new unsigned int[8];
  targetToUint(target_hex, o_hex);
  for(int i = 0; i < 8; i++)
  {
    sprintf(result+i*8,"%08x", o_hex[i]);
  }


  printf("%s\n", result);
  printf("%s\n", target_hex);
  
  memcpy(tgt_inp_device, o_hex, sizeof(unsigned int)*8);    // copy target to target pointer
  memcpy(ptr_inp_device, header, strlen(header)+1);        // copy header to header pointer

  time(&start);
    
  performHash();    // perform hash on header

  time(&end);

  double time_taken = double(end - start);
  cout << "Time taken by module is : " << fixed << time_taken << setprecision(5);
  cout << " sec " << endl;
     
  cout <<"version: " << version <<endl;                // output version
	cout <<"previous hash: " << previous_hash <<endl;    // output previsous hash
	cout <<"merkle root: " << merkle_root <<endl;        // out merkle root
	cout <<"time: " << time_hex <<endl;                  // output time in hex format
	cout <<"Bits: " << bits_hex <<endl;                  // output bits
 	cout <<"Header: "<< header << endl << endl;         // output header
  
  // output result
  for(int i = 0; i < 8; i++)
  {
    sprintf(result+i*8,"%08x", ptr_op_device[i]);
  }
  printf("Result: %s\n", result);

  for(int i = 0; i < 160; i++)
  {
    sprintf(header+i,"%c", ptr_op_device2[i]);

  }
  printf("Header: %s\n", header);

    
   

  // End the program.

	bool end_program = false;
	
	if (end_program == false)
	{
		cout << "Enter 1 and press return to end the program." << endl;
		cin >> end_program;
	}
	  
 }
 // function to convert decimal into hex format
void dec_to_hex(int n, char hex[9])
{

	// char array to store hexadecimal number
    char hexaDeciNum[9]= "00000000";
 
    // counter for hexadecimal number array
    int i = 0;
    while (n != 0) 
	{
        // temporary variable to store remainder
        int temp = 0;
 
        // storing remainder in temp variable.
        temp = n % 16;
 
        // check if temp < 10
        if (temp < 10) 
		    {
            hexaDeciNum[i] = temp + 48;
            i++;
        }
        else 
		    {
            hexaDeciNum[i] = temp + 87;
            i++;
        }
 
        n = n / 16;
    }
  hex[0] = hexaDeciNum[7];
	hex[1] = hexaDeciNum[6];
	hex[2] = hexaDeciNum[5];
	hex[3] = hexaDeciNum[4];
	hex[4] = hexaDeciNum[3];
	hex[5] = hexaDeciNum[2];
	hex[6] = hexaDeciNum[1];
	hex[7] = hexaDeciNum[0];
}

void targetToUint(char hexl[65], unsigned int * o)
{
  for(int i = 0 ; i < 8 ; i++)
  {
    o[i] = 0;
    for(int j = 7 ; j >= 0 ; j--)
    {
      unsigned int myVal;
      stringstream ss;
      ss << hexl[i*8 + (7 - j)];
      ss >> hex >> myVal;
      o[i] += pow(16,j)*myVal;
    }
  }
}

// function to convert target into expanded hex
int target(int exp, char bits[9])
{

    if(bits[0] == '1')
      exp = exp + 16;
	
  switch(bits[1]) 
	{
      case '0' :
         exp = exp + 0; 
         break;
      case '1' :
		 exp = exp + 1;
		 break;
      case '2' :
         exp = exp + 2;
         break;
      case '3' :
         exp = exp + 3;
         break;
      case '4' :
         exp = exp + 4;
         break;
	  case '5' :
         exp = exp + 5;
         break;
	  case '6' :
         exp = exp + 6;
         break;
	  case '7' :
         exp = exp + 7;
         break;
	  case '8' :
         exp = exp + 8;
         break;
	  case '9' :
         exp = exp + 9;
         break;
	  case 'a'  :
         exp = exp + 10;
         break;
      case 'A'  :
         exp = exp + 10;
         break;
	  case 'b' :
         exp = exp + 11;
         break;
	  case 'B' :
         exp = exp + 11;
         break;
	  case 'c' :
         exp = exp + 12;
         break;
	  case 'C' :
         exp = exp + 12;
         break;
	  case 'd'  :
         exp = exp + 13;
         break;
	  case 'D'  :
         exp = exp + 13;
         break;
	  case 'e' :
         exp = exp + 14;
         break;
	  case 'E' :
         exp = exp + 14;
         break;
	  case 'f' :
         exp = exp + 15;
         break;
	  case 'F' :
         exp = exp + 15;
         break;
      default :
         cout << "Invalid input" << endl;
     
     
  }

    return exp;


}
