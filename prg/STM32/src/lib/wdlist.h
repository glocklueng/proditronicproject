#ifndef WDLIST
#define WDLIST

#include <ktypes.h>

typedef struct wdlist_entry_s_ wdlist_entry_s;
struct wdlist_entry_s_
	{
	wdlist_entry_s *prev;
	wdlist_entry_s *next;
	void *data;
	};


typedef struct
	{
	wdlist_entry_s *first_entry;
	wdlist_entry_s *last_entry;
	k_ushort entries_number;
	} wdlist_s;


void wdlist_init(wdlist_s *wdlist);
void wdlist_append(wdlist_s *wdlist, void *new_entry);
void wdlist_delete(wdlist_s *wdlist, wdlist_entry_s *entry);
void wdlist_clear(wdlist_s *wdlist);

#endif
