#include "action.h"

#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <thread>
#include <chrono>

#include <time.h>

#include <fstream>

//#define DEBUG1

elevator g_elevator = { 1, 0, e_WAIT, e_STOP };
layer_information	 g_layerInfo[FLOORS] = { 0 };
recent				 g_outer[3] = { 0 };
u16					 g_inter[3] = { 0 };


void out(char *buf, E_dir dir)
{
	int i = atoi(buf) - 1;
	g_layerInfo[i].floor_flag = LOADING;

	if (g_layerInfo[i].outkey != e_NOTHING)
		g_layerInfo[i].outkey = e_UPDOWNKEY;
	else
		g_layerInfo[i].outkey = dir == e_UP ? e_UPKEY : e_DOWNKEY;
}

void in(char *buf)
{
	int i = atoi(buf) - 1;
	g_layerInfo[i].floor_flag = LOADING;
	g_layerInfo[i].inkey = e_NUMKEY;
}

/*
D:DN:02S:WAITING
U14U22$$02/05/32 
*/
void schedule_update1()
{
	char buf1[17], buf2[17];
    const char *status[6] = { "WAITING", "RUNNING", "OPENING", "LOADING", "CLOSING", "" }, *pStatus;
	char dir;

	int i = 0;
	const char *w = "...", *pw = w + 2;



	//puts(date);
	//FILE *p;
	//p = fopen("elevatorlog", "a+");
	//

	while (true)
	{
#ifdef DEBUG1
		time_t t = time(0);
		char date[64] = { 0 };
		strftime(date, sizeof(date), "----%Y/%m/%d %X----", localtime(&t));

		std::ofstream fout;
		fout.open("elevator.log", std::ios::app);
		if (!fout)
			return;
#endif // DEBUG1

		std::this_thread::sleep_for(std::chrono::milliseconds(2000));

		if (g_elevator.dir == e_UP)
		{
			dir = 'U';
		}
		else if (g_elevator.dir == e_DOWN)
		{
			dir = 'D';
		}
		else
		{
			dir = ' ';
		}

		switch (g_elevator.status)
		{
		case e_WAIT:
			pStatus = status[0];
			break;

		case e_RUN:
			pStatus = status[1];
			break;

		case e_OPEN:
			pStatus = status[2];
			break;

		case e_LOAD:
			pStatus = status[3];
			break;

		case e_CLOSE:
			pStatus = status[4];
			break;

		default:
			pStatus = status[5];
			break;
		}

		sprintf(buf1, "D:%cN:%02dS:%s", dir, g_elevator.curr_floor, pStatus);

		sprintf(buf2, "%c%02d%c%02d$$%02d/%02d/%02d",
				g_outer[0].dir == e_UP ? 'U' : (g_outer[0].dir == e_DOWN ? 'D' : ' '), g_outer[0].floor,
				g_outer[1].dir == e_UP ? 'U' : (g_outer[1].dir == e_DOWN ? 'D' : ' '), g_outer[1].floor,
				//g_outer[2].dir == e_UP ? 'U' : 'D', g_outer[2].floor,
				g_inter[0], g_inter[1], g_inter[2]);


		printf("%s\n%s\n%s\n\n", pw + i--, buf1, buf2);
		if (i == -3) i = 0;

#ifdef DEBUG1	
		fout << date << "\r\n" << buf1 << "\r\n" << buf2 << "\r\n\r\n";
		fout.close();
		memset(date, 0, sizeof(date));
#endif 

		memset(buf1, 0, sizeof(buf1));
		memset(buf2, 0, sizeof(buf2));
	}
}

void schedule_update2()
{
	while (1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		control();
	}
}


int main()
{
	//printf("%d", sizeof(E_dir));


	printf("直接按数字键后按回车键是内部请求\n按 u 或者 d 再加数字键是外部请求\n");
	
	std::thread th1(schedule_update1), th2(schedule_update2);
	
	char buf[10];
	int i;
	while (1)
	{
		for (i = 0; i < 10; ++i)
		{
			buf[i] = getchar();
			if (buf[i] == '\n') break;
		}

		if (buf[0] == 'u')
		{
			out(buf + 1, e_UP);
		}
		else if (buf[0] == 'd')
		{
			out(buf + 1, e_DOWN);
		}
		else if (isdigit(buf[0]))
		{
			in(buf);
		}
		else
		{
			printf("retry!!!\n");
		}
	}

	th1.join();
	th2.join();

	return 0;
}
