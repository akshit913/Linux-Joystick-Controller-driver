#include <iostream>
#include <unistd.h>
#include <cstring>
#include <libusb-1.0/libusb.h>
#include <linux/uinput.h>
#include <fcntl.h>
#include <stdio.h>
using namespace std;
void emit(int fd, int type, int code, int val)
{
   struct input_event ie;

   ie.type = type;
   ie.code = code;
   ie.value = val;
   ie.time.tv_sec = 0;
   ie.time.tv_usec = 0;

   write(fd, &ie, sizeof(ie));
}
int getDecimal(bool a, bool b) 
{
    int x=(int)a;
    int y=(int)b;
    int num=a*10+b;
    int decvalue = 0;
    int base = 1;
    int temp = num;
    while (temp) {
        int lastdigit = temp % 10;
        temp = temp / 10;
        decvalue += lastdigit * base;
        base = base * 2;
    }
    return decvalue;
}
bool getBit(unsigned char *byte, int position)
	{
    		return (*byte >> position) & 0x1;
	}
int main() {
	struct uinput_setup usetup;
	int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	libusb_device **devs;
	libusb_device_handle *dev_handle;
	libusb_context *ctx = NULL;
	int r;
	ssize_t cnt;
	r = libusb_init(&ctx);
	if(r < 0) {
		cout<<"Init Error "<<r<<endl;
		return 1;
	}
	libusb_set_debug(ctx, 3);

	cnt = libusb_get_device_list(ctx, &devs);
	if(cnt < 0) {
		cout<<"Get Device Error"<<endl;
		return 1;
	}
//	cout<<cnt<<" Devices in list."<<endl;

	dev_handle = libusb_open_device_with_vid_pid(ctx, 39546, 47639);
	if(dev_handle == NULL)
		cout<<"Cannot open device"<<endl;
	else
//		cout<<"Device Opened"<<endl;
	libusb_free_device_list(devs, 1);

	unsigned char *data=new unsigned char[0];
	data[0]='0';
	int actual;
	if(libusb_kernel_driver_active(dev_handle, 0) == 1) {
		cout<<"Kernel Driver Active"<<endl;
		if(libusb_detach_kernel_driver(dev_handle, 0) == 0)
			cout<<"Kernel Driver Detached!"<<endl;
	}
	r = libusb_claim_interface(dev_handle, 0);
	if(r < 0) {
		cout<<"Cannot Claim Interface"<<endl;
		return 1;
	}
	ioctl(fd, UI_SET_EVBIT, EV_KEY);
   	ioctl(fd, UI_SET_KEYBIT, BTN_JOYSTICK);
	ioctl(fd, UI_SET_EVBIT, EV_ABS);
	ioctl(fd, UI_SET_ABSBIT, ABS_X);
	ioctl(fd, UI_SET_ABSBIT, ABS_Y);
   	memset(&usetup, 0, sizeof(usetup));
  	usetup.id.bustype = BUS_USB;
  	usetup.id.vendor = 0x9A7A; /* sample vendor */
  	usetup.id.product = 0xBA17; /* sample product */
	string appname="chompapp";
	appname.copy(usetup.name,appname.length(),0);
	ioctl(fd, UI_DEV_SETUP, &usetup);
   	ioctl(fd, UI_DEV_CREATE);
	int x=0;
	while(x==0){
	x=libusb_interrupt_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN), data, 1, &actual, 0);
		int x_axis=getDecimal(getBit(data,3),getBit(data,2));
		int y_axis=getDecimal(getBit(data,1),getBit(data,0));
		int buttonVal=(int)getBit(data,4);
		if(x_axis == 1){
			emit(fd,EV_ABS,ABS_X,-32767);
		}else if(x_axis == 2){
			emit(fd,EV_ABS,ABS_X,0);
		}else if(x_axis == 3){
			emit(fd,EV_ABS,ABS_X,32767);
		}else{
			cout<<"error"<<endl;
		}

		if(y_axis == 1){
			emit(fd,EV_ABS,ABS_Y,32767);
		}else if(y_axis == 2){
			emit(fd,EV_ABS,ABS_Y,0);
		}else if(y_axis == 3){
			emit(fd,EV_ABS,ABS_Y,-32767);
		}else{
			cout<<"error"<<endl;
		}
		emit(fd, EV_KEY,BTN_JOYSTICK, buttonVal);
     		 emit(fd, EV_SYN, SYN_REPORT, 0);
	}
	r = libusb_release_interface(dev_handle, 0); //release the claimed interface
	if(r==0) {
		cout<<"Cannot Release Interface"<<endl;
		return 1;
	}
	cout<<"Released Interface"<<endl;
	ioctl(fd, UI_DEV_DESTROY);
   	close(fd);
	libusb_close(dev_handle);
	libusb_exit(ctx);
	delete data;
	return 0;
}

