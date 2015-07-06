/*
 *  Copyright (c) 2012-2013 wangsheng gao
 *  Copyright (c) 2012-2013 Anyka, Inc
 *
 *  aimer98 key's program
 *
 * See INSTALL for installation details or manually compile with
 * gcc -o dlna_key  dlna_key.c
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Should you need to contact me, the author, you can do so either by
 * e-mail - mail your message to <vojtech@ucw.cz>, or by paper mail:
 * Vojtech Pavlik, Simunkova 1594, Prague 8, 182 00 Czech Republic
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <pthread.h>
#include <linux/netlink.h>
#include <linux/version.h>
#include <linux/input.h>

#include <linux/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#define KEY_GPIO_DEV		"/dev/input/event0"
#define KEY_AD_DEV			"/dev/input/event1"
#define SELECT_WIFI_MODE	"/etc/init.d/select_wifi_mode.sh"
#define WIRELESS_SWITCH		"/etc/init.d/select_wifi_mode.sh"
#define WIRENET_SWITCH 		"/etc/init.d/select_wire_mode.sh"
#define RECOVER_MODE		"/etc/init.d/wifi_recover.sh"
#define WIFI_WPS_MODE		"/etc/init.d/wifi_wps.sh"
#define UPDATE_IMAGE_MODE	"/etc/init.d/update.sh /mnt/zImage"
#define SDP1_DEV_NAME       "/dev/mmcblk0p1"
#define SD_DEV_NAME         "/dev/mmcblk0"

#if 0
#define MOUNT_SDP1			"/bin/mount -rw /dev/mmcblk0p1 /mnt"
#define MOUNT_SD			"/bin/mount -rw /dev/mmcblk0 /mnt" 
#define UMOUNT_SD			"/bin/umount /mnt"
#else
#define MOUNT_SDP1			"mount -rw /dev/mmcblk0p1 /mnt" 
#define MOUNT_SD			"mount -rw /dev/mmcblk0 /mnt" 
#define UMOUNT_SD			"umount /mnt"
#endif

#define KERNEL_ZIMAGE_FILE			"/mnt/zImage"
#define SQSH_ZIMAGE_FILE			"/mnt/root.sqsh4"
#define JFFS_ZIMAGE_FILE			"/mnt/root.jffs2"

#define KERNEL_ZIMAGE		1
#define SQSH_ZIMAGE			2
#define JFFS_ZIMAGE			4
#define K_S_IMAGE		(KERNEL_ZIMAGE|SQSH_ZIMAGE)
#define K_J_IMAGE		(KERNEL_ZIMAGE|JFFS_ZIMAGE)
#define S_J_IMAGE		(SQSH_ZIMAGE|JFFS_ZIMAGE)
#define K_S_J_IMAGE		(KERNEL_ZIMAGE|SQSH_ZIMAGE|JFFS_ZIMAGE)


#define WIFI_MODE			3
#define UPDATE_IMAGE		10

#define UEVENT_BUFFER_SIZE      2048

#define KEY_DEBUG(fmt...)	 //printf(fmt)

static int old_key = -1;
static struct timeval start_time;

/**
 * *  @brief       diff timeval
 * *  @author      gao wangsheng
 * *  @date        2012-10-15
 * *  @param[in]   struct timeval *old, *new
 * *  @return      diff time
 * */
static double difftimeval (struct timeval *old, struct timeval *new)
{
    return (new->tv_sec - old->tv_sec) * 1000. + (new->tv_usec - old->tv_usec) / 1000;
}

/* create a sd_test dir and mount the sd card in it */
void mount_sd(int flag)
{
    char cmd[128];

    if (flag)
        sprintf(cmd, "%s", MOUNT_SDP1);
    else
        sprintf(cmd, "%s", MOUNT_SD);

    system(cmd);
    printf("*** mount the sd to /mnt ***\n");
}

/* umount the sd card and delete the sd_test dir */
void umount_sd(void)
{
    char cmd[128];

    system("sync");
    sprintf(cmd, "%s", UMOUNT_SD);
    system(cmd);
    printf("*** umount the sd ***\n");
}

/* compare two file and make sure is it the same */
int check_file(void)
{
    int fd;
    int ret = 0;

    if ((fd = open(KERNEL_ZIMAGE_FILE, O_RDONLY)) >= 0)
        ret = KERNEL_ZIMAGE;  
    else {
        printf("not have the kernel zImage in /mnt\n");
    }
    close(fd);

    if ((fd = open(SQSH_ZIMAGE_FILE, O_RDONLY)) >= 0)
        ret = ret|SQSH_ZIMAGE;
    else{
        printf("not have root.sqsh4 in /mnt\n");
    }
    close(fd);

    if ((fd = open(JFFS_ZIMAGE_FILE, O_RDONLY)) >= 0)
        ret = ret|JFFS_ZIMAGE;
    else{
        printf("not have root.jffs2 in /mnt\n");
    }
    close(fd);

    if (ret == 0)
        ret = -1;

    return ret;
}

/* create the socket to recevie the uevent */
static int init_hotplug_sock(void)
{
    struct sockaddr_nl snl;
    const int buffersize = 2048;
    int retval;

    memset(&snl, 0x00, sizeof(struct sockaddr_nl));

    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;

    int hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (hotplug_sock == -1)	{
        printf("error getting socket: %s", strerror(errno));
        return -1;
    }

    /* set receive buffersize */
    setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));

    retval = bind(hotplug_sock, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl));
    if (retval < 0) {
        printf("bind failed: %s", strerror(errno));
        close(hotplug_sock);
        hotplug_sock = -1;
    }

    return hotplug_sock;
}


/* waitting the uevent and do something */
void *pth_func(void *data)
{
    char buf[UEVENT_BUFFER_SIZE * 2] = {0};
    char temp_buf[20];
    char *p;
    int i;
    int hotplug_sock = init_hotplug_sock();

    if (hotplug_sock < 0)
        return NULL;

    if (access (SDP1_DEV_NAME, F_OK) == 0)
        mount_sd(1);
    else if (access (SD_DEV_NAME, F_OK) == 0)
        mount_sd(0);

    while(1) {
        memset(buf, 0, sizeof(buf));

        recv(hotplug_sock, &buf, sizeof(buf), 0);
        p = strrchr(buf, '/');

        for (i = 0; buf[i] != '@' && buf[i] != 0; i++)
            temp_buf[i] = buf[i];
        temp_buf[i] = 0;

        if (strcmp(temp_buf, "change"))
            printf("%s\n", buf);

        if (!strcmp(temp_buf, "add")) {
            if (!strcmp(p, "/mmcblk0p1")) {
                sleep(1);
                mount_sd(1);
                continue;
            } else if (!strcmp(p, "/mmcblk0")) {
                sleep(1);
                mount_sd(0);
            }
        }

        if (!strcmp(temp_buf, "remove")) {
            if ((!strcmp(p, "/mmcblk0p1")) || (!strcmp(p, "/mmcblk0")))
                umount_sd();
        }
    }
}
#define I2C_DEV "/dev/i2c-0"

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


/**
 * *  @brief       do key_0
 * *  @author      gao wangsheng
 * *  @date        2012-10-15
 * *  @param[in]   pressing period
 * *  @return      0 on success
 * */
static int __do_gpio_key_0(double period)
{
    char cmd[128];
    int fd,ret;
    int start;

    if (period < 0)
    {
        perror("Error period");
        return -1;
    }
    else if (period < WIFI_MODE)
    {
        int mode = system("/etc/init.d/mode.sh");
        //station = 1
        //softap  = 2
        mode >>= 8;
        if(mode == 1){
            system("killall udhcpc wpa_supplicant");
            system("/etc/init.d/change_wifi_mode.sh softap");
            printf("[##]change to softap\n");
        }
        else if(mode == 2){
            system("killall hostapd udhcpd");
            system("/etc/init.d/change_wifi_mode.sh station");
            printf("[##]change to station\n");
        }

        sprintf(cmd,"%s","/etc/init.d/wifi_start.sh keypress");
    }
    else if (period > UPDATE_IMAGE)
    {
        ret = check_file();
        if (ret < 0)
            return -1;

        switch(ret) {
            case KERNEL_ZIMAGE:
                sprintf(cmd, "%s", "/etc/init.d/update.sh /mnt/zImage");
                break;
            case SQSH_ZIMAGE:
                sprintf(cmd, "%s", "/etc/init.d/update.sh /mnt/root.sqsh4");
                break;
            case JFFS_ZIMAGE:
                sprintf(cmd, "%s", "/etc/init.d/update.sh /mnt/root.jffs2");
                break;
            case K_S_IMAGE:
                sprintf(cmd, "%s", "/etc/init.d/update.sh /mnt/zImage /mnt/root.sqsh4");
                break;
            case K_J_IMAGE:
                sprintf(cmd, "%s", "/etc/init.d/update.sh /mnt/zImage /mnt/root.jffs2");
                break;
            case S_J_IMAGE:
                sprintf(cmd, "%s", "/etc/init.d/update.sh /mnt/root.sqsh4 /mnt/root.jffs2");
                break;
            case K_S_J_IMAGE:
                sprintf(cmd, "%s", "/etc/init.d/update.sh /mnt/zImage /mnt/root.sqsh4 /mnt/root.jffs2");
                break;
            default:
                break;
        }
    }
    else
    {
        sprintf(cmd, "%s ", WIFI_WPS_MODE);
    }

    system(cmd);
    return 0;
}

/**
 * *  @brief       do key wireless
 * *  @author      gao wangsheng
 * *  @date        2012-12-19
 * *  @param[in]   struct input_event *event
 * *  @return      0 on success
 * */
static int do_adkey_wireless(struct input_event *event)
{
    char cmd[128];

    if ((event->value == 1) && (old_key !=event->code))
    {
        KEY_DEBUG("%s: code=%d; value=%d.\n", 
                __func__, event->code, event->value);

        old_key =event->code;
        sprintf(cmd, "%s %s", WIRELESS_SWITCH, "wireless");
        system(cmd);
        return 0;
    }
    return 1;
}

/**
 * *  @brief       do key wirenet
 * *  @author      gao wangsheng
 * *  @date        2012-12-19
 * *  @param[in]   struct input_event *event
 * *  @return      0 on success
 * */
static int do_adkey_wirenet(struct input_event *event)
{
    char cmd[128];

    if ((event->value == 1) && (old_key !=event->code))
    {
        KEY_DEBUG("%s: code=%d; value=%d.\n", 
                __func__, event->code, event->value);

        old_key =event->code;
        sprintf(cmd, "%s %s", WIRENET_SWITCH, "wirenet");
        system(cmd);
        return 0;
    }
    return 1;
}

/**
 * *  @brief       	do_gpio_key_0
 * *  @author      	gao wangsheng
 * *  @date        	2012-12-19
 * *  @param[in]   struct input_event *event
 * *  @return      	0 on success
 * */
static int do_gpio_key_0(struct input_event *event)
{
    double period  = 0;

    if (event->value == 1)
    {
        start_time.tv_sec = event->time.tv_sec;
        start_time.tv_usec = event->time.tv_usec;			
    }
    else if (event->value == 0)
    {
        period = difftimeval(&start_time, &event->time) / 1000;
        printf("period = %f(s).\n", period);
        __do_gpio_key_0(period);
        return 0;
    }

    return 1;
}




static int do_gpio_key_1(struct input_event *event){
    printf("key_1 pressed.\n");
    int fd,ret;
    int start,end;
    unsigned int i2c_addr = 0x34;
    //0xc1 
    //reg 0x12 get 0x5f(on) or 0x5d(off)
    //

    printf("code:%d,value:%d\n",event->code,event->value); 
    system("/etc/init.d/wifi_led.sh wps_led blink 300 300");
    sleep(1);
    system("/etc/init.d/wifi_start.sh keypress &");
    return 0;//cause of 0x34 error, we just start wifi and return.
    if(event->value==0)
        return -1;
    fd = open("/dev/i2c-0",O_RDWR);

    if(fd < 0){
        perror("open i2c device failed.\n");
        return -1;
    }
    if (ioctl(fd, I2C_SLAVE_FORCE, i2c_addr) < 0) {
        printf("Cannot set slave addr of i2c device! \n");
        close(fd);
        return -1;
    }
    else
        printf("Set slave addr to 0x%02x! \n", i2c_addr);

    ioctl(fd, I2C_TENBIT, 0);

    start = 0x06;
    ret = i2c_smbus_read_byte_data(fd, start);
    if(ret != 0xf0){
        printf("i2c value not correct.\n");
        close(fd);
        return -1;
    }
    start = 0x07;
    ret = i2c_smbus_read_byte_data(fd, start);
    if(ret != 0x0f){
        printf("i2c value not correct.\n");
        close(fd);
        return -1;
    }
    start = 0x08;
    ret = i2c_smbus_read_byte_data(fd, start);
    if(ret != 0x00){
        printf("i2c value not correct.\n");
        close(fd);
        return -1;
    }
    start = 0x09;
    ret = i2c_smbus_read_byte_data(fd, start);
    if(ret != 0xff){
        printf("i2c value not correct.\n");
        close(fd);
        return -1;
    }

    start = 0x12;
    ret = i2c_smbus_read_byte_data(fd, start);
    if(ret == 0x5d){
        end = 0x5f;
        ret = i2c_smbus_write_byte_data(fd, start, end);
        if(ret<0){
            perror("Write I2C error!\n");
            close(fd);
            return -1;
        }
        else{
            printf("Write 0x%04x to 0x%02x[0x%02x] successfully\n", end, i2c_addr, start);
            system("/etc/init.d/wifi_led.sh wps_led blink 300 300");
            sleep(2);
            system("/etc/init.d/wifi_start.sh keypress &");
        }
    }else if(ret == 0x5f){
        end = 0x5d;
        ret = i2c_smbus_write_byte_data(fd, start, end);
        if(ret<0){
            perror("Write I2C error!\n");
            close(fd);
            return -1;
        }
        else{
            printf("Write 0x%04x to 0x%02x[0x%02x] successfully\n", end, i2c_addr, start);
            system("killall -9 finish_station.sh hostapd udhcpd wpa_supplicant udhcpc");
            system("/etc/init.d/wifi_led.sh wps_led off");
            printf("[##]wifi_led off\n");
        }
    }

    close(fd);
    return 0;

}
/**
 * *  @brief       do_key
 * *  @author      gao wangsheng
 * *  @date        2012-12-19
 * *  @param[in]   struct input_event *event, int key event count
 * *  @return      0 on success
 * */
static int do_key(struct input_event *key_event, int key_cnt)
{
    int i = 0;
    int ret = -1;
    struct input_event *event;

    if (key_cnt < (int) sizeof(struct input_event)) {
        printf("expected %d bytes, got %d\n", (int) sizeof(struct input_event), key_cnt);
        return -1;
    }

    for (i = 0; (i < key_cnt/sizeof(struct input_event)); i++) 
    {
        event = (struct input_event *)&key_event[i];
        if (EV_KEY != event->type)
        {
            continue;
        }

        KEY_DEBUG("count = %d, code = %d, value = %d!\n", 
                key_cnt/sizeof(struct input_event), event->code, event->value);

        printf("%s handler event:", __func__);
        switch(event->code)
        {
            case KEY_UP:
                printf(" KEY_UP\n");
                ret = do_adkey_wireless(event);
                break;
            case KEY_DOWN:
                printf(" KEY_DOWN\n");
                ret = do_adkey_wirenet(event);
                break;
            case KEY_0:
                printf(" KEY_0\n");
                ret = do_gpio_key_0(event);
                break;
            case KEY_1:
                printf(" KEY_1\n");
                ret = do_gpio_key_1(event);
                break;
            default:
                printf("%s %s: Error key code!\n", __FILE__, __func__);
                ret = -1;
                break;
        }

        if (!ret){
            break;
        }
    }
    return ret;
}

/**
 * *  @brief       program's access point
 * *  @author      gao wangsheng
 * *  @date        2012-10-15
 * *  @param[in]   not use
 * *  @return      0 on success
 * */
int main (int argc, char **argv)
{
    int ret = 0;
    int gpio_fd;
//    int ad_fd;
    pthread_t pth;
    fd_set readfds, tempfds;

    FD_ZERO(&readfds);

    if ((gpio_fd = open(KEY_GPIO_DEV, O_RDONLY)) < 0) {
        perror("Open gpio key dev fail");
        return -ENOENT;
    }
    FD_SET(gpio_fd, &readfds);

#if 0
    if ((ad_fd = open(KEY_AD_DEV, O_RDONLY)) < 0){
        perror("Open ad key dev fail");
        return -ENOENT;
    }
    FD_SET(ad_fd, &readfds);
#endif

    pthread_create(&pth, NULL, pth_func, NULL);

    struct input_event evt;
    evt.value = 1;

    do_gpio_key_1(&evt);//shut down wifi module power..

    while (1) 
    {
        int rd = 0;
        struct input_event key_event[64];

        tempfds = readfds;
        ret = select(FD_SETSIZE, &tempfds, (fd_set *)0, (fd_set *)0, (struct timeval*)0);
        if (ret < 1){
            perror("select error");
            exit(2);
        }

        if (FD_ISSET(gpio_fd, &tempfds))
        {
            rd = read(gpio_fd, key_event, sizeof(struct input_event) * sizeof(key_event));
            do_key(key_event, rd);
        }
#if 0
        if (FD_ISSET(ad_fd, &tempfds))
        {
            rd = read(ad_fd, key_event, sizeof(struct input_event) * sizeof(key_event));
            do_key(key_event, rd);
        }
#endif
    }

    pthread_cancel(pth);
    close(gpio_fd);
//  close(ad_fd);
    return ret;
}
