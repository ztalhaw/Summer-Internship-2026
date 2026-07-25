#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/mman.h>
#include<sys/stat.h>

//this is a helper to read strings because gguf files dont
//have null terminator in their strings
const uint8_t* readstring(const uint8_t *ptr,char *out_buf,size_t max_len){
    //reading length header
    uint64_t len=*(const uint64_t *)ptr;
    ptr+=8;

    size_t copy_len=(len<max_len-1)? len: max_len-1;
    //destination,source,how much to copy
    memcpy(out_buf,ptr,copy_len);
    out_buf[copy_len]='\0'; 

    return ptr+len;
}

int main(int argc,char **argv){

    //check if the user entered the file name in cli after running the program
    if(argc<2){
        printf("usage: %s <path_to_model.gguf>\n",argv[0]);
        return 1;
    }

    const char *filename=argv[1];//in cli when we enter the file name it is stored in argv[1]

    //open
    int fd=open(filename,O_RDONLY);
    if(fd<0){
        perror("error opening file");
        return 1;
    }

    //get file size
    struct stat st;
    if(fstat(fd,&st)<0){
        perror("error checking the file size");
        close(fd);
        return 1;
    }
    size_t file_size=st.st_size;//st_size is a variable that comes in stat struct
    
    //mmap(where to store in memory,size,read flag,save changes in memory to the file,which file,offset)
    const uint8_t *file_data=mmap(NULL,file_size,PROT_READ,MAP_SHARED,fd,0);
    if(file_data==MAP_FAILED){
        perror("mmap failed");
        close(fd);
        return 1;
    }

    //pointer to parse through the memory, it starts from the first value of the file
    const uint8_t *ptr=file_data;

    uint32_t magic=*(const uint32_t *)ptr;
    ptr+=4;

    if(magic!=0x46554747){
        fprintf(stderr, "error: file is not a valid GGUF\n");
        munmap((void*)file_data,file_size);
        close(fd);
        return 1;
    }

    uint32_t version=*(const uint32_t*)ptr;
    ptr+=4;
    uint64_t tensor_count=*(const uint64_t*)ptr;
    ptr+=8;
    uint64_t metadata_count=*(const uint64_t*)ptr;
    ptr+=8;

    printf("GGUF HEADER INFO: \n");
    printf("GGUF version: %u\n",version);
    printf("Tensor count: %lu\n",tensor_count);
    printf("Metadata Items: %lu\n\n",metadata_count);

    printf("METADATA KEY-VLAUE PAIRS: \n");
    for(uint64_t i=0;i<metadata_count;i++){
        char key_name[256];
        ptr=readstring(ptr,key_name,sizeof(key_name));
        
        uint32_t type_id=*(const uint32_t*)ptr;
        ptr+=4;

        printf("[%-6s] (type %2u) : ",key_name,type_id);

        switch(type_id){
            case 0://UINT 8
            printf("%u\n",*ptr);
            ptr+=1;
            break;

            case 1://int 8
            printf("%d\n",*(const int8_t *)ptr);
            ptr+=1;
            break;

            case 2://uint 16
            printf("%u\n",*(const uint16_t *)ptr);
            ptr+=2;
            break;

            case 3://int 16
            printf("%d\n",*(const int16_t *)ptr);
            ptr+=2;
            break;

            case 4://uint 32
            printf("%u\n",*(const uint32_t *)ptr);
            ptr+=4;
            break;

            case 5://int 32
            printf("%d\n",*(const int32_t *)ptr);
            ptr+=4;
            break;

            case 6://float 32
            printf("%f\n",*(const float *)ptr);
            ptr+=4;
            break;

            case 7://bool
            printf("%s\n",*ptr?"true":"false");
            ptr+=1;
            break;

            case 8:{
                //string
                char str[256];
                ptr=readstring(ptr,str,sizeof(str));
                printf("\"%s\"\n",str);
                break;
            }

            case 9:{
                //array
                uint32_t item_type=*(const uint32_t*)ptr;
                ptr+=4;
                uint64_t array_len=*(const uint64_t*)ptr;
                ptr+=8;
                printf("[Array of %lu items (Type %u)]\n",array_len,item_type);

                for(uint64_t j=0;j<array_len;j++){
                            switch (item_type) {
                    case 0: case 1: case 7: // 1-byte types (UINT8, INT8, BOOL)
                        ptr += 1; 
                        break;
                    case 2: case 3: // 2-byte types (UINT16, INT16)
                        ptr += 2; 
                        break;
                    case 4: case 5: case 6: // 4-byte types (UINT32, INT32, FLOAT32)
                        ptr += 4; 
                        break;
                    case 8: { // STRING array
                        uint64_t s_len = *(const uint64_t *)ptr;
                        ptr += 8 + s_len; // 8 bytes length header + string characters
                        break;
                    }
                    case 10: case 11: case 12: // 8-byte types (UINT64, INT64, FLOAT64)
                        ptr += 8; 
                        break;
                    default:
                        fprintf(stderr, "Error: Unknown array item type %u\n", item_type);
                        return 1;
                }
                }
                break;
            }

            case 10:
            printf("%lu\n",*(const uint64_t*)ptr);
            ptr+=8;
            break;

            case 11:
            printf("%ld\n",*(const int64_t*)ptr);
            ptr+=8;
            break;

            case 12:
            printf("%f\n",*(const double*)ptr);
            ptr+=8;
            break;

            default:
            printf("<unknown type id %u\n>",type_id);
            munmap((void*)file_data,file_size);
            close(fd);
            return 1;
        }
    }

    munmap((void*)file_data,file_size);
    close(fd);

    printf("\n successful\n");

    return 0;
}