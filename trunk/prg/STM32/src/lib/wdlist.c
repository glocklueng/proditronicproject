#include <stdlib.h>
#include <stdio.h>

#include "wdlist.h"

//------------------------------------------------------------------------------

void wdlist_init(wdlist_s *wdlist)
	{
	wdlist->first_entry= NULL;
	wdlist->last_entry= NULL;
	wdlist->entries_number= 0;
	}

//------------------------------------------------------------------------------

void wdlist_append(wdlist_s *wdlist, void *new_entry_data_ptr)
	{
	wdlist_entry_s *new_entry;

	if (!new_entry_data_ptr)
		return;

	new_entry= malloc(sizeof(wdlist_entry_s));
	if (!new_entry)
		return;

	new_entry->data= new_entry_data_ptr;
	new_entry->next= NULL;

	if (wdlist->first_entry == NULL)
		{
		new_entry->prev= NULL;
		wdlist->first_entry= new_entry;
		wdlist->last_entry= new_entry;
		}
	else
		{
		new_entry->prev= wdlist->last_entry;
		wdlist->last_entry->next= new_entry;
		wdlist->last_entry= new_entry;
		}

	wdlist->entries_number++;

	}

//------------------------------------------------------------------------------

void wdlist_delete(wdlist_s *wdlist, wdlist_entry_s *entry)
	{
	if (!entry)
		return;

	if (entry->data)
		{
		//printf("\n<%s> data pointer != NULL\n", __FUNCTION__);
		return;
		}

	if (entry == wdlist->first_entry)
		{
		if (entry->next != NULL)
			{
			wdlist->first_entry= entry->next;
			wdlist->first_entry->prev= NULL;
			}
		else
			{
			wdlist->first_entry= NULL;
			wdlist->last_entry= NULL;
			}
		}
	else
	if (entry == wdlist->last_entry)
		{
		wdlist->last_entry= entry->prev;
		wdlist->last_entry->next= NULL;
		}
	else
		{
		wdlist_entry_s *prev= entry->prev;
		wdlist_entry_s *next= entry->next;

		prev->next= next;
		next->prev= prev;
		}

	free(entry);
	entry= NULL;

	wdlist->entries_number--;
	}

//------------------------------------------------------------------------------

void wdlist_clear(wdlist_s *wdlist)
	{
	wdlist_entry_s *entry_ptr;
	while (entry_ptr= wdlist->first_entry)
		{
		if (entry_ptr->data)
			{
			free(entry_ptr->data);
			entry_ptr->data= NULL;
			}
		wdlist_delete(wdlist, entry_ptr);
		wdlist->entries_number--;
		}
	}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


