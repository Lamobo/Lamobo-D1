#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>

#include <time.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/ioctl.h>

#include <linux/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>


#define I2C_DEV		"/dev/i2c-0"



static inline __s32 i2c_smbus_access(int file, char read_write, __u8 command,
                                     int size, union i2c_smbus_data *data)
{
        struct i2c_smbus_ioctl_data args;

        args.read_write = read_write;
        args.command = command;
        args.size = size;
        args.data = data;
        return ioctl(file,I2C_SMBUS,&args);
}

static inline __s32 i2c_smbus_read_byte(int file)
{
        union i2c_smbus_data data;
        if (i2c_smbus_access(file,I2C_SMBUS_READ,0,I2C_SMBUS_BYTE,&data))
                return -1;
        else
                return 0x0FF & data.byte;
}

static inline __s32 i2c_smbus_write_byte(int file, __u8 value)
{
        return i2c_smbus_access(file,I2C_SMBUS_WRITE,value,
                                I2C_SMBUS_BYTE,NULL);
}

static inline __s32 i2c_smbus_read_byte_data(int file, __u8 command)
{
        union i2c_smbus_data data;
        if (i2c_smbus_access(file,I2C_SMBUS_READ,command,
                             I2C_SMBUS_BYTE_DATA,&data))
                return -1;
        else
                return 0x0FF & data.byte;
}

static inline __s32 i2c_smbus_write_byte_data(int file, __u8 command,
                                              __u8 value)
{
        union i2c_smbus_data data;
        data.byte = value;
        return i2c_smbus_access(file,I2C_SMBUS_WRITE,command,
                                I2C_SMBUS_BYTE_DATA, &data);
}

void print_help(){
	printf("Usage: i2capp r <i2caddr> <regaddr> [regaddr end]\n"
	       "       i2capp w <i2caddr> <regaddr> <data>\n"
	       "       i2capp s <i2caddr> <regaddr> <bit>\n"
	       "       i2capp c <i2caddr> <regaddr> <bit>\n"
	       );
	exit(0);

}

int main(int argc, char ** argv)
{
	int fd,ret;
	int start,end,bit;
	unsigned int i2c_addr;
	
	fd = open(I2C_DEV, O_RDWR);
	
	if(argc!= 5 && argc!=4)
		print_help();

	if(fd < 0){
		printf("Open i2c device failed ! \n");
		return -1;
	}
	else
		printf("Open i2c device successfully ! \n");

	sscanf(argv[2], "%i", &i2c_addr);
	
	if (ioctl(fd, I2C_SLAVE_FORCE, i2c_addr) < 0) {
		printf("Cannot set slave addr of i2c device! \n");
		close(fd);
		return -1;
  	}
	else
		printf("Set slave addr to 0x%02x! \n", i2c_addr);

	ioctl(fd, I2C_TENBIT, 0);

	if(argv[1][0] == 'r'){
		sscanf(argv[3], "%i", &start);
		if(argc==5)
			sscanf(argv[4], "%i", &end);
		else
			end = start;
		if(start>end)
			print_help();
		while(start<=end){
			ret = i2c_smbus_read_byte_data(fd, start);
			if(ret<0){
				printf("Read I2C error!\n");
				exit(0);
			}
			else{
				printf("Read 0x%04x from 0x%02x[0x%02x]\n", ret, i2c_addr, start);
			}
			++start;
		}
	}
	else if(argv[1][0] == 'w'){
		if(argc !=5)
			print_help();
		sscanf(argv[3], "%i", &start);
		sscanf(argv[4], "%i", &end);
		
		ret = i2c_smbus_write_byte_data(fd, start, end);
		if(ret<0){
			printf("Write I2C error!\n");
			exit(0);
		}
		else
			printf("Write 0x%04x to 0x%02x[0x%02x] successfully\n", end, i2c_addr, start);
	}
	else if(argv[1][0] == 's'){
		if(argc !=5)
			print_help();
		sscanf(argv[3], "%i", &start);
		sscanf(argv[4], "%i", &bit);
		end = i2c_smbus_read_byte_data(fd, start);
		if(end<0){
			printf("Read I2C error!\n");
			exit(0);
		}
		printf("Set bit %d of 0x%02x[0x%02x]  0x%04x -> 0x%04x\n", bit, i2c_addr, start, end, end|(1<<bit));
		end|=(1<<bit);
		ret = i2c_smbus_write_byte_data(fd, start, end);
	}
	else if(argv[1][0] == 'c'){
		if(argc !=5)
			print_help();
		sscanf(argv[3], "%i", &start);
		sscanf(argv[4], "%i", &bit);
		end = i2c_smbus_read_byte_data(fd, start);
		if(end<0){
			printf("Read I2C error!\n");
			exit(0);
		}
		printf("Clear bit %d of 0x%02x[0x%02x]  0x%04x -> 0x%04x\n", bit, i2c_addr, start, end, end&~(1<<bit));
		end&=~(1<<bit);
		ret = i2c_smbus_write_byte_data(fd, start, end);
	}
	else
		print_help();
	

	close(fd);
	return 0;
}
		
