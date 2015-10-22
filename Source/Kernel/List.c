//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __OS_LIST_H__
#define __OS_LIST_H__

// Include types
#include "Types.h"
#include "Kernel.h"
#include "Utilities.h"

/////////////////////////////////////////////////////////////////////////////////////
// Init list
/////////////////////////////////////////////////////////////////////////////////////
void OS_List_Init(List *list)
{
	// Set up
	list->Count = 0;
	list->Capacity = 2;
	
	// Allocate memory
	list->Data = OS_Allocate(list->Capacity);
}

/////////////////////////////////////////////////////////////////////////////////////
// Add object to the list
/////////////////////////////////////////////////////////////////////////////////////
void OS_List_Add(List *list, void *data)
{
	// Check if list has space
	if(list->Capacity == list->Count)
	{
		// Resize
		void* prev = list->Data;
		list->Capacity *= 2;
		list->Data = OS_Allocate(list->Capacity);
		OS_UTILS_MemoryCopy(list->Data, prev, list->Count);
		free(prev);
	}
	
	// Set item
	list->Data[list->Count] = data;
	
	// More items
	list->Count++;
}

/////////////////////////////////////////////////////////////////////////////////////
// Removes last object from the list (remember to remove also Data)
/////////////////////////////////////////////////////////////////////////////////////
void OS_List_RemoveLast(List *list)
{
	// Less items
	list->Count--;
}

/////////////////////////////////////////////////////////////////////////////////////
// Removes all elements from the list (remember to remove also all Data)
/////////////////////////////////////////////////////////////////////////////////////
void OS_List_Clear(List *list)
{
	// Check if has memory allocated
	if(list->Capacity > 0)
	{
		// Clean
		free(list->Data);
		list->Capacity = 0;
	}
	
	// No Items
	list->Count = 0;
	list->Data = Null;
}

/////////////////////////////////////////////////////////////////////////////////////
// Returns last item data
/////////////////////////////////////////////////////////////////////////////////////
void* OS_List_GetLast(List *list)
{	
	// Check if has items
	if(list->Count == 0)
	{
		// Nothing
		return Null;
	}
	else	
	{
		// Return data
		return list->Data[list->Count - 1];
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Returns item data at given index
/////////////////////////////////////////////////////////////////////////////////////
void* OS_List_GetAt(List *list, Word index)
{
	// Check if index is in collection
	if(index >= list->Count)
	{
		// Nothing
		return Null;
	}
	else
	{
		// Return
		return list->Data[index];
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Determines whether an element is in the list. Returns True or False.
/////////////////////////////////////////////////////////////////////////////////////
Byte OS_List_Contains(List *list, void* data)
{
	// Data
	Word i = 0;
	
	// Search for data
	while(i < list->Count)
	{
		// Compare
		if(list->Data[i] == data)
		{
			// Contains
			return True;
		}
	}
	
	// Not contains
	return False;
}

/////////////////////////////////////////////////////////////////////////////////////
// Searches for the specified object and returns the zero-based index of the first occurrence within the entire list
// Returns the zero-based index of the first occurrence of item within the entire list, if found; otherwise, –1.
/////////////////////////////////////////////////////////////////////////////////////
SWord OS_List_IndexOf(List *list, void* data)
{
	// Data
	Word i = 0;
	
	// Search for data
	while(i < list->Count)
	{
		// Compare
		if(list->Data[i] == data)
		{
			// Found
			return i;
		}
	}
	
	// Nothing
	return -1;
}
/*
/////////////////////////////////////////////////////////////////////////////////////
// Inserts an element into the list at the specified index
/////////////////////////////////////////////////////////////////////////////////////
void OS_List_Insert(List *list, Word index, void* data)
{
	// Data
	ListNode *wsk = list->Root, *New;
	
	// Check if index is bigger than list has items
	if(index > list->Count)
	{
		// Back
		return;
	}
	
	// Check if has nodes
	if(list->Count == 0)
	{
		// When index is 0 just add it
		if(index == 0)
		{
			// Add to list
			OS_List_Add(list, data);
		}
		
		// Back
		return;
	}
	
	// When index is last
	if(index == list->Count)
	{
		// Add to list
		OS_List_Add(list, data);
		
		// Back
		return;
	}
	
	// Check if list has space
	if(list->Capacity == list->Count)
	{
		// Resize
		void* prev = list->Data;
		list->Capacity *= 2;
		list->Data = OS_Allocate(list->Capacity);
		OS_UTILS_MemoryCopy(list->Data, prev, list->Count);
		free(prev);
	}
	/////////////////////////////
	// Set item
	///////////////////list->Data[lisgdgdfunt] = data;
	////////////////////
	// More items
	list->Count++;
}

/////////////////////////////////////////////////////////////////////////////////////
// Removes the element at the specified index of the list
/////////////////////////////////////////////////////////////////////////////////////
void OS_List_RemoveAt(List *list, Word index)
{
	////////////////////
	// Data
	ListNode *wsk = list->Root, *next;
	
	// Check if has nodes
	if(wsk == Null)
	{
		// Back
		return;
	}
	
	// Check if has only 1 one
	if(list->Count == 1)
	{
		// Clean
		free(list->Root);
		
		// No elements
		list->Root = Null;
	}
	// Check if index point at last item
	else if((index+1) == list->Count)
	{
		// Remove last
		OS_List_RemoveLast(list);
		
		// Back
		return;
	}
	else
	{
		// Find item at given index
		while (wsk->Next->Next != Null)
		{
			index--;
			
			// When index is 0, current item is needed to be removed
			if(index == 0)
			{
				// Remember node to link
				next =  wsk->Next->Next;
				
				// Clean
				free(wsk->Next);
				
				// Link
				wsk->Next = next;
				
				// Back
				break;
			}
			
			// Move
			
			wsk = wsk->Next;
		}
	}
	///////////////
	// Less items
	list->Count--;
}
*/
/////////////////////////////////////////////////////////////////////////////////////
// Removes element from the list if contains it. removes only first found.
/////////////////////////////////////////////////////////////////////////////////////
void OS_List_Remove(List *list, void* data)
{
	// Data
	Word a, i = 0;
	
	// Find item
	while(i < list->Count)
	{
		// Compare data
		if(list->Data[i] == data)
		{
			// Less items
			list->Count--;
			
			// Move items after current item forward
			for(a = i; a < list->Count; a++)
			{
				// Copy
				list[a] = list[a + 1];
			}
		}
		
		// Move
		i++;
	}
}

#endif
