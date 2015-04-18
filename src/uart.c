#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<termios.h>

int set_uart(const char *dev, int speed, int databits, int stopbits, int parity)
{
	int i,fuart;
	int status;
	int speed_arr[] = { B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300 };
	int name_arr[] = { 115200, 38400, 19200, 9600, 4800, 2400, 1200, 300 };
	struct termios opt;
	fuart=open(dev,O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
	tcgetattr(fuart, &opt);
	for(i = 0; i<sizeof(speed_arr)/sizeof(int); i++)
	{
		if (speed==name_arr[i])
		{
			tcflush(fuart, TCIOFLUSH);
			cfsetispeed(&opt, speed_arr[i]);
			cfsetospeed(&opt, speed_arr[i]);
			status = tcsetattr(fuart, TCSANOW, &opt);
			if (status = 0) tcflush(fuart, TCIOFLUSH);
		
		}
	}
	if( tcgetattr(fuart, &opt)  !=  0) return(-1);
	opt.c_cflag &= ~CSIZE;
	opt.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN );
	opt.c_oflag &= ~OPOST;
	switch (databits)
	{
	case 5:
		opt.c_cflag |= CS5;
		break;
	case 6:
		opt.c_cflag |= CS6;
		break;
	case 7:
		opt.c_cflag |= CS7;
		break;
	case 8:
		opt.c_cflag |= CS8;
		break;
	default:
		return(-1);
	}
	switch (parity)
	{
	case 'n':
	case 'N':
		opt.c_cflag &= ~PARENB;   
		opt.c_iflag &= ~INPCK;     
		break;
	case 'o':
	case 'O':
		opt.c_cflag |= (PARODD | PARENB);  /* parity checking */ 
		opt.c_iflag |= INPCK;      /* Disnable parity checking */
		break;
	case 'e':
	case 'E':
		opt.c_cflag |= PARENB;     /* Enable parity */
		opt.c_cflag &= ~PARODD;   /*  */  
		opt.c_iflag |= INPCK;       /* Disnable parity checking */
	case 'S':
	case 's':  
		opt.c_cflag &= ~PARENB;
		opt.c_cflag &= ~CSTOPB;
		break;
	default:
		return(-1);
	}
	switch (stopbits)
	{
	case 1:
		opt.c_cflag &= ~CSTOPB;
		break;
	case 2:
		opt.c_cflag |= CSTOPB;
		break;
	default:
		return(-1);
	}
	if (parity != 'n')
		opt.c_iflag |= INPCK;
	opt.c_iflag |= IGNBRK;
	opt.c_iflag |= IGNPAR;
	opt.c_iflag &=~(IXON|INLCR|IGNCR|ICRNL | IXOFF);
	opt.c_oflag &=~(ONLCR|OCRNL | ONOCR | ONLRET | OFILL);
	tcflush(fuart, TCIFLUSH); 
	opt.c_cc[VTIME] =1; //  mseconds 100
	opt.c_cc[VMIN] = 0;//  0
	if(tcsetattr(fuart, TCSANOW, &opt) != 0)
		return(-1);
	return(fuart);
}

void clean_uart(int fuart)
{
	 tcflush(fuart, TCIOFLUSH);
}
