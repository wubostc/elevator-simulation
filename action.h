#ifndef __ACTION_H__
#define __ACTION_H__

typedef unsigned char	u8;
typedef signed char		s8;

typedef unsigned int	u16;
typedef signed int		s16;

typedef unsigned long	u32;
typedef signed long		s32;

typedef unsigned char	BOOL;
#define TRUE	1
#define FALSE	0

#define TIMER_LOADING	5
#define TIMER_OPEN		2
#define TIMER_CLOSE		2
#define TIMER_RUN		2


#define FLOORS	10


#define MS_OK	'#'
#define MS_UP	'*'
#define MS_DOWN '#'
#define MS_DEL	'*'


#define LOADING		1
#define NONLOADING	0


typedef enum { e_WAIT, e_RUN, e_OPEN, e_CLOSE, e_LOAD } E_status;

typedef enum { e_STOP, e_UP, e_DOWN } E_dir;

typedef enum { e_NOTHING, e_UPKEY, e_DOWNKEY, e_UPDOWNKEY, e_NUMKEY } E_key;

typedef struct
{
	u16 curr_floor;	// dangqiande louceng
	u16 dest_floor;	// yaoqude louceng
	E_status status;// e_WAIT or e_RUN or e_OPEN or e_CLOSE or e_LOAD
	E_dir dir;		// e_STOP or e_UP or e_DOWN
} elevator;

typedef struct
{
	u8		floor_flag; // LOADING or NONLOADING
	E_key	inkey;	// shuzijian huo fangxiangjian
	E_key	outkey;	// shuzijian huo fangxiangjian
} layer_information;//layerInformation

typedef struct
{
	u16	  floor;
	E_dir dir;
} recent;

extern elevator				g_elevator;
extern layer_information	g_layerInfo[FLOORS];
extern recent				g_outer[3];
extern u16					g_inter[3];

typedef BOOL (*Pred)(layer_information *, layer_information **);

void control();

void on_load();

void on_close(layer_information *it);

void on_open();

void on_run();

void on_wait();

void update_for_next(layer_information **it);

layer_information *search_next(layer_information *it, E_dir dir);

layer_information *search(layer_information *begin, layer_information *end, Pred pr);

BOOL __all(layer_information *, layer_information **);

BOOL __up(layer_information *, layer_information **);

BOOL __down(layer_information *, layer_information **);

void delete_recent();

void save_to_recent(layer_information *base);

#endif // __ACTION_H__
