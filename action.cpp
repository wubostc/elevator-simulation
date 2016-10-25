#include "action.h"
#include <string.h>

/*
comment:
通过一个具体的楼层来得到指向这个楼层的指针 layer_infomation * ptr

param: unsigned int

return: layer_infomation * ptr */
#define get_layerinfo(FLOOR) (g_layerInfo + (FLOOR) - 1)

/*
comment:
通过 layer_infomation * ptr 来得到它指向的楼层

param: layer_infomation * ptr

return: int */
#define get_floor(PLAYERINFO)	((PLAYERINFO) - g_layerInfo + 1)

// general function for g_layerInfo[FLOORS]
#define end()		(g_layerInfo + (FLOORS))
#define begin()		(g_layerInfo)
#define rend()		(begin() - 1)
#define rbegin()	(end() - 1)

void save_to_recent(layer_information *base)
{
	u16 countout = 0, countin = 0;
	E_dir dir = g_elevator.dir;
	layer_information *next = base, *last = base;
    u16 i;

	memset(g_inter, 0, sizeof(g_inter));
	memset(g_outer, 0, sizeof(g_outer));

	for (i = 0; i < 2; ++i)
	{	
		while ((next != end()) && (next != rend()))
		{
			if ((countin < 3) && (next->inkey != e_NOTHING))
			{
				if (g_inter[0] != get_floor(next) && g_inter[1] != get_floor(next))
				{
					g_inter[countin++] = get_floor(next);
				}
			}

			if ((countout < 3) && (next->outkey != e_NOTHING))
			{
				if (g_outer[0].floor != get_floor(next) && g_outer[1].floor != get_floor(next))
				{
					if (next->outkey == e_UPKEY)
					{
						g_outer[countout].dir = e_UP;
					}
					else if (next->outkey == e_DOWNKEY)
					{
						g_outer[countout].dir = e_DOWN;
					}
					else // next->outkey == e_UPDOWNKEY
					{
						g_outer[countout].dir = dir;
					}
					g_outer[countout++].floor = get_floor(next);
				}
			}
			last = next;
			next = search_next(next, dir);
			if (next == last) next = end(); // for g_elevator.dir == e_STOP
		}
		dir = (dir == e_UP ? e_DOWN : e_UP);
		next = last;
	}
}

void delete_recent()
{
	if (g_outer[0].floor == g_elevator.curr_floor)
	{
		g_outer[0] = g_outer[1];
		g_outer[1] = g_outer[2];
		g_outer[2].floor = 0;
	}

	if (g_inter[0] == g_elevator.curr_floor)
	{
		g_inter[0] = g_inter[1];
		g_inter[1] = g_inter[2];
		g_inter[2] = 0;
	}
}


layer_information *search_next(layer_information *it, E_dir dir)
{
	switch (dir)
	{
	case e_UP:
		it = search(it + 1, end(), __up);
		break;
		
	case e_DOWN:
		it = search(it - 1, rend(), __down);
		break;

	case e_STOP:
		it = search(begin(), end(), __all);
		break;
	}

	return it;
}

// 找到离电梯所处楼层的最近可开门条件的楼层并更新电梯的目标楼层，如果没有找到则会在下次调用时搜索反方向
void update_for_next(layer_information **it)
{
	static BOOL f = FALSE; // 搜索不到同方向的目标楼层时，f = TRUE

	*it = search_next(get_layerinfo(g_elevator.curr_floor), g_elevator.dir);
	
	if ((*it == end()) || (*it == rend()))
	{
		if (f == FALSE)
		{
			f = TRUE;

			switch (g_elevator.dir)
			{
			case e_UP:
				g_elevator.dir = e_DOWN;
				break;

			case e_DOWN:
				g_elevator.dir = e_UP;
				break;
			
            case e_STOP:
                break;
            }
		}
		else if (f == TRUE) // 如果往电梯所处楼层还是搜索不到目标楼层
		{
			g_elevator.dir = e_STOP;
		}
	}
	else
	{
		f = FALSE;
		g_elevator.dest_floor = get_floor(*it);
	}
}

// 根据已经找到的楼层请求来更新电梯的状态 status 和 运动方向 dir
void on_wait()
{
	if (g_elevator.curr_floor > g_elevator.dest_floor)
	{
		g_elevator.status = e_RUN;
		g_elevator.dir = e_DOWN;
	}
	else if (g_elevator.curr_floor < g_elevator.dest_floor)
	{
		g_elevator.status = e_RUN;
		g_elevator.dir = e_UP;
	}
	else
	{
		g_elevator.status = e_OPEN;
		g_elevator.dir = e_STOP;
	}
}

// run -> open
void on_run()
{
	switch(g_elevator.dir)
	{
	case e_UP:
		++(g_elevator.curr_floor);
		break;
		
	case e_DOWN:
		--(g_elevator.curr_floor);
		break;

    case e_STOP:
	    break;
    }

	if (g_elevator.curr_floor == g_elevator.dest_floor)
	{
		g_elevator.status = e_OPEN;
	}
}

// open -> load
void on_open()
{
	layer_information *current = get_layerinfo(g_elevator.curr_floor);
	layer_information *next = search_next(current, g_elevator.dir);

	if (current->inkey == e_NUMKEY)
	{
		current->inkey = e_NOTHING;
	}

	if (current->outkey == e_UPKEY)
	{
		if (g_elevator.dir == e_DOWN)
		{
			if (next == end() || next == rend())
			{
				current->outkey = e_NOTHING;
			}
		}
		else // g_elevator.dir == e_STOP or e_UP
		{
			current->outkey = e_NOTHING;
		}
	}
	else if (current->outkey == e_DOWNKEY)
	{
		if (g_elevator.dir == e_UP)
		{
			if (next == end() || next == rend())
			{
				current->outkey = e_NOTHING;
			}
		}
		else // g_elevator.dir == e_STOP or e_UP
		{
			current->outkey = e_NOTHING;
		}
	}
	else if (current->outkey == e_UPDOWNKEY)
	{
		if (g_elevator.dir == e_UP)
		{
			current->outkey = e_DOWNKEY;
		}
		else if (g_elevator.dir == e_DOWN)
		{
			current->outkey = e_UPKEY;
		}
		else // g_elevator.dir == e_STOP
		{
			current->outkey = e_NOTHING;
		}
	}

	if ((current->inkey == e_NOTHING) && (current->outkey == e_NOTHING))
	{
		current->floor_flag = NONLOADING;
	}
}

// close -> wait    close -> run
void on_close(layer_information *it)
{	
	if ((it == end()) || (it == rend())) // 没有找到目标楼层
	{
		g_elevator.status = e_WAIT;
		g_elevator.dir = e_STOP;
	}
	else if (it == get_layerinfo(g_elevator.curr_floor)) // 目标楼层与电梯所处楼层是同一层
	{
		g_elevator.status = e_WAIT;
	}
	else // 找到目标楼层
	{
		if (g_elevator.curr_floor > g_elevator.dest_floor)
		{
			g_elevator.dir = e_DOWN;
		}
		else // if (g_elevator.curr_floor < g_elevator.dest_floor)
		{
			g_elevator.dir = e_UP;
		}
		g_elevator.status = e_RUN;
	}	
}

// load -> close
void on_load()
{
	g_elevator.status = e_CLOSE;
}

void control()
{
    static u16 count = 0; // (20 * count) == 1s
	static layer_information *it;
	
	switch (g_elevator.status)
	{
	case e_WAIT:
		update_for_next(&it);

		if (it != end() && it != rend())
		{
            on_wait();
		}
		break;

	case e_RUN:
		if (++count == TIMER_RUN * 20)
		{
			count = 0;
			on_run();
		}
		else
		{
			update_for_next(&it);
		}
		break;
		
	case e_OPEN:
		if (count == 0) // 立刻打开门
		{
			on_open();
			delete_recent();
		}

		if (++count == TIMER_OPEN * 20)
		{
			count = 0;
			g_elevator.status = e_LOAD;
		}
		else
		{
			update_for_next(&it);
		}
		break;
		
	case e_CLOSE:
		if (++count == TIMER_CLOSE * 20)
		{
			count = 0;
			on_close(it);
		}
		else
		{
			update_for_next(&it);
		}
		break;
		
	case e_LOAD:
		if (++count == TIMER_LOADING * 20)
		{
			count = 0;
			on_load();
		}
		else
		{
			update_for_next(&it);
		}
		break;
	}
	
    if ((it != end()) && (it != rend())) save_to_recent(it);
}

BOOL __all(layer_information *it, layer_information **)
{
	return (it->floor_flag == LOADING ? TRUE : FALSE);
}

BOOL __up(layer_information *it, layer_information **last)
{
	BOOL f = FALSE;
	if (it->floor_flag == LOADING)
	{
		if (it->outkey == e_UPKEY || it->outkey == e_UPDOWNKEY || it->inkey == e_NUMKEY)
		{
			f = TRUE;
		}
		else if (it->outkey == e_DOWNKEY)
		{
			*last = it;
		}
	}
	return f;
}

BOOL __down(layer_information *it, layer_information **last)
{
	BOOL f = FALSE;
	if (it->floor_flag == LOADING)
	{
		if (it->outkey == e_DOWNKEY || it->outkey == e_UPDOWNKEY || it->inkey == e_NUMKEY)
		{
			f = TRUE;
		}
		else if (it->outkey == e_UPKEY)
		{
			*last = it;
		}
	}
	return f;
}

layer_information *search(layer_information *begin, layer_information *end, Pred pr)
{
	layer_information *it, *last = end;
	
	for (it = begin; it != end; begin > end ? --it : ++it)
	{
		if (pr(it, &last))
		{
			break;
		}
	}
	return (it == end ? last : it);
}
