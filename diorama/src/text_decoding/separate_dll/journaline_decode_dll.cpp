/******************************************************************************/
/*                                                                            */
/*  University of Kaiserslautern, Institute of Communications Engineering     */
/*  Copyright (C) 2005 Torsten Schorr                                         */
/*                                                                            */
/*  Author(s)    : Torsten Schorr (schorr@eit.uni-kl.de)                      */
/*  Project start: 09.02.2005                                                 */
/*  Last change  : 18.02.2005                                                 */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */ 
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*  journaline_decode_dll.cpp                                                 */
/*                                                                            */
/******************************************************************************/
/*  Description:                                                              */
/*  DAB data group decoding and journaline news service decoding              */
/*                                                                            */
/*  Usage:                                                                    */
/*                                                                            */
/*  news_objects = journaline_decode(input, extended_header_length);          */
/*                                                                            */
/*  The decoder is fed with an uint8 vector "input" and the length of the     */
/*  extended header. It returns a cell array "news_objects" containing        */
/*  structures with the following elements:                                   */
/*                                                                            */
/*  filename: string with proposed filename                                   */
/*  update:   update state of this news object (+1: create, -1: delete)       */
/*  filebody: uint8 array with the file data to store                         */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*  This program is based on the 'NewsService Journaline(R) Decoder'          */
/*  Copyright (c) 2003, 2004 by Fraunhofer IIS, Erlangen, Germany             */
/*                                                                            */
/*                                                                            */
/*  To use the 'NewsService Journaline(R) Decoder' software for               */
/*  COMMERCIAL purposes, please contact Fraunhofer IIS for a                  */
/*  commercial license (see below for contact information)!                   */
/*                                                                            */
/* --------------------------------------------------------------------       */
/*                                                                            */
/*  Contact:                                                                  */
/*   Fraunhofer IIS, Department 'Broadcast Applications'                      */
/*   Am Wolfsmantel 33, 91058 Erlangen, Germany                               */
/*   http://www.iis.fraunhofer.de/dab                                         */
/*   mailto:bc-info@iis.fraunhofer.de                                         */
/*                                                                            */
/******************************************************************************/


#include <stdlib.h>
#include "zlib.h"
#include "journaline_20040318/NML.h"
#include "journaline_20040318/newssvcdec.h"
#include "journaline_20040318/dabdatagroupdecoder.h"
#include "journaline_decode_dll.h"
#include "journaline_decode.h"

//#define _DEBUG_

extern "C" {

int journaline_decode(unsigned char *dab_data_group, int N, int extended_header_len, struct Output_Files **output, int *count)
{
	int objID, debug = 0, debug_mode = 0, m, n, return_value = 0;
			  
	unsigned long max_memory = 1024*1024; // 1 MB memory for news decoder
	unsigned long max_objects = 0; // No limit for number of NML objects

	struct Updated_Objects *objects_root = NULL;
	struct LinkList *list_ptr;

	
	// Check for proper number of arguments
	

	if (N < 0) {

		debug = 1;
		debug_mode = *((int *)dab_data_group);
	}




	if (firstcall) {

		return_value = 1;
#ifdef _DEBUG_
		fid = fopen ("debug_journaline_decode_dll.txt","w");
#endif

		firstcall = 0;
		// DAB data group decoder
		dgdec = DAB_DATAGROUP_DECODER_createDec(datagroup_callback, (void *)&newsdec);
		if (!dgdec){
#ifdef _DEBUG_
			fprintf(fid, PROGNAME ": creation of DAB data group decoder failed!\n");
			fclose(fid);
#endif
			return return_value;
		}

		// Journaline (R)
		newsdec = NEWS_SVC_DEC_createDec(object_avail_callback, max_memory, &max_objects, extended_header_len, (void *)&objects_available);
		if (!newsdec){
			DAB_DATAGROUP_DECODER_deleteDec(dgdec);
#ifdef _DEBUG_
			fprintf(fid, PROGNAME ": creation of news service decoder failed!\n");
			fclose(fid);
#endif
			return return_value;
		}
		update_watchlist(0, &watchlist, &length_of_watchlist);
		if (!NEWS_SVC_DEC_watch_objects(newsdec, length_of_watchlist, watchlist)) {
			NEWS_SVC_DEC_deleteDec(newsdec);
			DAB_DATAGROUP_DECODER_deleteDec(dgdec);
#ifdef _DEBUG_
			fprintf(fid, PROGNAME ": Applying watch list failed!\n");
			fclose(fid);
#endif
			return return_value;
		}

	} else {
#ifdef _DEBUG_
		fid = fopen ("debug_journaline_decode_dll.txt","a");
#endif
	}

	news_lists.watchlist = &watchlist;
	news_lists.length_of_watchlist = &length_of_watchlist;
	news_lists.linklist = &pending_link_list;
	news_lists.objects_root = &objects_root;
	news_lists.no_of_updates = 0;
	objects_available.number_of_elements = 0;

	if ((debug) && (debug_mode == 1)) {	
		update_watchlist((unsigned short)extended_header_len, &watchlist, &length_of_watchlist);
		if (!NEWS_SVC_DEC_get_object_availability(newsdec, length_of_watchlist, watchlist)) {
#ifdef _DEBUG_
			fprintf(fid, PROGNAME ": Applying watch list failed!\n");
#endif
		}
#ifdef _DEBUG_
		fprintf(fid, "watchlist = ");
		for (m=0;m < *news_lists.length_of_watchlist; m++)
			fprintf(fid, "[%i] %04x:%i ", m, (*news_lists.watchlist)[m].object_id, (*news_lists.watchlist)[m].status);
		fprintf(fid, "\n");
		fclose(fid);
#endif
		return return_value;
	}

	if ((debug) && (debug_mode == 3)) {	
#ifdef _DEBUG_		
		fprintf(fid, "pending link list = ");
		m = 0;
		list_ptr = pending_link_list;
		while (list_ptr) {
			fprintf(fid, "[%i] %04x:%04x ", m, list_ptr->linkObjID, list_ptr->parentObjID);
			m++;
			list_ptr = list_ptr->next;
		}
		fprintf(fid, "\n");
		fclose(fid);
#endif
		return return_value;
	}

	
	if (!debug) {

		if (!DAB_DATAGROUP_DECODER_putData(dgdec, N, dab_data_group))
		{
			*count = 0;
			return_value = 2;
#ifdef _DEBUG_
			fprintf(fid, PROGNAME ": putting data to DAB data group decoder failed!\n");
			fclose(fid);
#endif
			return return_value;
		}
	}


	if ((debug) && (debug_mode == 2)) { 
	
		explore_news_tree(&news_lists, (unsigned short)extended_header_len);
	
	} else {
		n = length_of_watchlist;
		if (objects_available.number_of_elements) {
			for (m = 0; m < (int)objects_available.number_of_elements; m++) {
				objID = objects_available.chg_list[m].object_id;	
				explore_news_tree(&news_lists, objID);
			}
			delete(objects_available.chg_list);
		}
		if (length_of_watchlist > n) {
			if (!NEWS_SVC_DEC_watch_objects(newsdec, length_of_watchlist, watchlist)) {
#ifdef _DEBUG_
				fprintf(fid, PROGNAME ": Applying watch list failed!\n");
#endif
			}
		}

	}

	// create Matlab output:
	create_output(output, count, &news_lists);
#ifdef _DEBUG_
	fclose(fid);
#endif
	return return_value;
}



void journaline_decode_exit (void) 
{
	update_watchlist(-1, &watchlist, &length_of_watchlist);
	update_linklist (-1, &pending_link_list, 0 ,0);

	NEWS_SVC_DEC_deleteDec(newsdec);
	DAB_DATAGROUP_DECODER_deleteDec(dgdec);
	firstcall = 1;
	return;
}



void free_output(struct Output_Files *output, int count)
{

	for (int i = 0; i < count; i++) {
		delete (output[i].filebody);
	}

	delete (output);

}

}

void datagroup_callback(const DAB_DATAGROUP_DECODER_msc_datagroup_header_t *, const unsigned long len, const unsigned char *buf, void *input)
{
	NEWS_SVC_DEC_decoder_t *newsdec_ptr = (NEWS_SVC_DEC_decoder_t *) input;

	if (!NEWS_SVC_DEC_putData(*newsdec_ptr, len, buf)) {
#ifdef _DEBUG_
		fprintf(fid, PROGNAME ": datagroup_callback: NEWS_SVC_DEC_putData failed!");
#endif
	}
}



void object_avail_callback(unsigned long number_of_elements, NEWS_SVC_DEC_obj_availability_t *chg_list, void *input )
{
	struct Objects_Available *objects_avail =  (struct Objects_Available *)input;
	
	objects_avail->number_of_elements = number_of_elements;
	objects_avail->chg_list = (NEWS_SVC_DEC_obj_availability_t *)new(NEWS_SVC_DEC_obj_availability_t[number_of_elements]);
	memcpy(objects_avail->chg_list, chg_list, number_of_elements * sizeof(NEWS_SVC_DEC_obj_availability_t));
#ifdef _DEBUG_
	fprintf(fid, "object_avail_callback. status = %i, %i\n", chg_list->status, NEWS_SVC_DEC_OBJ_RECEIVED);
#endif
	return;
}



void explore_news_tree(struct News_Lists *news_lists, unsigned short objID)
{
	NML::RawNewsObject_t rno;
	unsigned long elen = 0;
	unsigned long len = 0;
	int i; unsigned short linkedobjID;
	struct Updated_Objects *uo_ptr, *help_ptr;
	NEWS_SVC_DEC_obj_availability_t check_ID;

	// remove updates of the same object from update list (possibly never happens)
	uo_ptr = *(news_lists->objects_root);
	while (uo_ptr && uo_ptr->next) {
		if (uo_ptr->next->objID == objID) {
			help_ptr = uo_ptr->next;
			uo_ptr->next = uo_ptr->next->next;
			delete (help_ptr->nml);
			delete(help_ptr);
			(news_lists->no_of_updates)--;
		}
		uo_ptr = uo_ptr->next;
	}
	if (*(news_lists->objects_root) && ((*(news_lists->objects_root))->objID == objID)) {
		help_ptr = *(news_lists->objects_root);
		*(news_lists->objects_root) = (*(news_lists->objects_root))->next;
		delete (help_ptr->nml);
		delete(help_ptr);
		(news_lists->no_of_updates)--;
	}

	// insert into update list
	if (*(news_lists->objects_root) == NULL) {
		*(news_lists->objects_root) = (struct Updated_Objects *)new (struct Updated_Objects);
		uo_ptr = *(news_lists->objects_root);
	} else {
		uo_ptr = *(news_lists->objects_root);
		while (uo_ptr->next != NULL) {
			uo_ptr = uo_ptr->next;
		}
		uo_ptr->next = (struct Updated_Objects *)new (struct Updated_Objects);
		uo_ptr = uo_ptr->next;
	}
	uo_ptr->objID = objID;
	uo_ptr->next = NULL;
	(news_lists->no_of_updates)++;


	if (NEWS_SVC_DEC_get_news_object(newsdec, objID, &elen, &len, rno.nml)) {
#ifdef _DEBUG_
		fprintf(fid, "object #%x available\n", objID);
#endif
		rno.nml_len = static_cast<unsigned short>(len);
		rno.extended_header_len = static_cast<unsigned short>(elen);

		NML *nml = NMLFactory::Instance()->CreateNML(rno, &sequence_handler);	
		if (nml->isMenu()) {
			for (i = 0; i < (int)nml->GetNrOfItems(); i++) {
				linkedobjID = nml->GetLinkId (i);
				check_ID.object_id = linkedobjID;
				NEWS_SVC_DEC_get_object_availability(newsdec, 1, &check_ID);

				if ((check_ID.status == NEWS_SVC_DEC_OBJ_RECEIVED) || (check_ID.status == NEWS_SVC_DEC_OBJ_UPDATED)) {
					nml->SetLinkAvailability(i, 1);
				} else {
					update_linklist(1, news_lists->linklist, linkedobjID, objID);
					nml->SetLinkAvailability(i, 0);
				}

				if ((update_watchlist(nml->GetLinkId(i), news_lists->watchlist, news_lists->length_of_watchlist) < 0) && (nml->isLinkIdAvailable(i))) {
					explore_news_tree(news_lists, linkedobjID);
				}
			}
		}	

		uo_ptr->status = NEWS_SVC_DEC_OBJ_RECEIVED;
		uo_ptr->nml = nml;

		while ((i = update_linklist(2, news_lists->linklist, objID, 0)) >= 0) {
			explore_news_tree(news_lists, (unsigned short) i);
		}
	
	} else {

		uo_ptr->status = NEWS_SVC_DEC_OBJ_REMOVED;
#ifdef _DEBUG_
		fprintf(fid, "object #%x not available\n", objID);
#endif
	}

	return;
}



void create_output(struct Output_Files **output, int *count, struct News_Lists *news_lists)
{
	int i, item, status, title_template_length, title_refresh_template_length;
	int refresh, end_template_length, body_length, heading_length, link_length, item_length;
	struct Updated_Objects *uo_ptr;
	char filename[10], *filebody_ptr;
	NML *nml;

	title_template_length = strlen(HTML_title_template) - 2;
	title_refresh_template_length = strlen(HTML_title_refresh_template) - 2;
	end_template_length = strlen(HTML_end_template);
	heading_length = strlen(HTML_heading_template) - 2;
	link_length = strlen(HTML_link_template) - 2;
	item_length = strlen(HTML_item_template) - 2;

	*output = (struct Output_Files *) new struct Output_Files[news_lists->no_of_updates];
	*count = news_lists->no_of_updates;

	i = 0; uo_ptr = *(news_lists->objects_root);
	while (uo_ptr) {
		
		if (i < news_lists->no_of_updates) {

			sprintf(filename, "%04x.html", uo_ptr->objID);
			strcpy((*output)[i].filename, filename);
			
			refresh = 0;
			body_length = 0;
			if ((uo_ptr->status == NEWS_SVC_DEC_OBJ_NOT_YET_AVAILABLE) || (uo_ptr->status == NEWS_SVC_DEC_OBJ_REMOVED)) {
				status = -1;
			} else {
				status = 1;
				nml = uo_ptr->nml;	

				if (nml->isMenu()) {
					body_length += heading_length + 2 * nml->GetTitle().length();
					for (item = 0; item < (int)nml->GetNrOfItems(); item++) {
						if (nml->isLinkIdAvailable(item)) {
							body_length += link_length + nml->GetItemText(item).length();
						} else {
							body_length += item_length + nml->GetItemText(item).length();
							refresh = 1;
						}
					}
				} else if (nml->GetNrOfItems() > 0) {
					body_length += heading_length + 2 * nml->GetTitle().length();
					for (item = 0; item < (int)nml->GetNrOfItems(); item++) {
						body_length += item_length + nml->GetItemText(item).length();
					}
				} else {
					body_length += nml->GetTitle().length();
				}

				if (refresh) { 
					body_length += title_refresh_template_length + end_template_length;
				} else {
					body_length += title_template_length + end_template_length;
				}

			}
			(*output)[i].update = (double) status;

			(*output)[i].filebody = (char *) new char[body_length];
			(*output)[i].body_length = body_length;
			filebody_ptr = (*output)[i].filebody;

			if (status == 1) {
				if (refresh) {
					sprintf(filebody_ptr, HTML_title_refresh_template,nml->GetTitle().c_str());
					filebody_ptr += title_refresh_template_length + nml->GetTitle().length();
				} else {
					sprintf(filebody_ptr, HTML_title_template,nml->GetTitle().c_str());
					filebody_ptr += title_template_length + nml->GetTitle().length();
				}

				if (nml->isMenu()) {
					sprintf(filebody_ptr, HTML_heading_template, nml->GetTitle().c_str());
					filebody_ptr += heading_length + nml->GetTitle().length();

					for (item = 0; item < (int)nml->GetNrOfItems(); item++) {
						if (nml->isLinkIdAvailable(item)) {
							sprintf(filebody_ptr, HTML_link_template, nml->GetLinkId(item),nml->GetItemText(item).c_str());
							filebody_ptr += link_length + nml->GetItemText(item).length();
						} else {
							sprintf(filebody_ptr, HTML_item_template,nml->GetItemText(item).c_str());
							filebody_ptr += item_length + nml->GetItemText(item).length();
						}
					}

				} else if (nml->GetNrOfItems() > 0) {
					sprintf(filebody_ptr, HTML_heading_template, nml->GetTitle().c_str());
					filebody_ptr += heading_length + nml->GetTitle().length();

					for (item = 0; item < (int)nml->GetNrOfItems(); item++) {
						sprintf(filebody_ptr, HTML_item_template,nml->GetItemText(item).c_str());
						filebody_ptr += item_length + nml->GetItemText(item).length();
					}
				} else {
					sprintf(filebody_ptr, "%s",nml->GetTitle().c_str());
					filebody_ptr += nml->GetTitle().length();
				}

				memcpy(filebody_ptr, HTML_end_template, end_template_length * sizeof(char));
				filebody_ptr += end_template_length;

				delete (nml);
			} else {
				//mexWarnMsgTxt(PROGNAME ": some error\n");
			}

		}

		*(news_lists->objects_root) = uo_ptr;
		uo_ptr = uo_ptr->next;	
		delete(*(news_lists->objects_root));
		i++;
	}

	*(news_lists->objects_root) = NULL;
	news_lists->no_of_updates = 0;
	return;
}



int update_watchlist(int objID, NEWS_SVC_DEC_obj_availability_t **list, int *length)
{
	const int list_extension = 128;
	NEWS_SVC_DEC_obj_availability_t *help_ptr;
	int i;

	if ((objID < 0)||(objID > 65535)) {
		if (*list) delete (*list);
		*length = 0;
		*list = NULL;
		return -1;	
	}
	if ((*list == NULL)||(*length <= 0)) {
		*list = (NEWS_SVC_DEC_obj_availability_t *)new(NEWS_SVC_DEC_obj_availability_t[list_extension]);
		(*list)[0].object_id = 0;
		(*list)[0].status = NEWS_SVC_DEC_OBJ_NOT_YET_AVAILABLE;
		*length = 1;
		return -1;
	}
	
	for (i = 0; i < *length; i++) {
		if ((*list)[i].object_id == objID) {
			(*list)[i].status = NEWS_SVC_DEC_OBJ_NOT_YET_AVAILABLE;
			return i;
		}
	}
	if ((int)((*length - 1)/list_extension) != (int)((*length)/list_extension)) {
		help_ptr = (NEWS_SVC_DEC_obj_availability_t *)new(NEWS_SVC_DEC_obj_availability_t[*length + list_extension]);
		memcpy(help_ptr, *list, (*length) * sizeof(NEWS_SVC_DEC_obj_availability_t));
		delete (*list);
		*list = help_ptr;
	}
	(*list)[*length].object_id = objID;
	(*list)[*length].status = NEWS_SVC_DEC_OBJ_NOT_YET_AVAILABLE;
	(*length)++;
	return -1;	
}



int update_linklist(int action, struct LinkList **pending_link_list, unsigned short linkObjID, unsigned short parentObjID)
{
	struct LinkList *list_ptr1, *list_ptr2;
	int result = -1;
	
	if (!pending_link_list) {
		return -1;
	}

	switch (action) {

	case -1:// delete link list

			list_ptr1 = *pending_link_list;
			while (list_ptr1) {
				list_ptr2 = list_ptr1->next;
				delete(list_ptr1);
				list_ptr1 = list_ptr2;
			}
			*pending_link_list = NULL;
			break;

	case 1:	// insert pending link if not already inserted
			
			
			if (*pending_link_list == NULL) {
				*pending_link_list = (struct LinkList *)new(struct LinkList);
				list_ptr1 = *pending_link_list;
			} else {
				list_ptr1 = *pending_link_list;
				while(list_ptr1->next) {
					if ((list_ptr1->linkObjID == linkObjID) && (list_ptr1->parentObjID == parentObjID)) {
						break;
					}
				list_ptr1 = list_ptr1->next;
				}
				if ((list_ptr1->linkObjID != linkObjID) || (list_ptr1->parentObjID != parentObjID)) {
					list_ptr1->next = (struct LinkList *)new(struct LinkList);
					list_ptr1 = list_ptr1->next;
				} else {
					break;
				}
			}
			list_ptr1->next = NULL;
			list_ptr1->linkObjID = linkObjID;
			list_ptr1->parentObjID = parentObjID;

			break;

	case 2:	// check for pending link and return and delete first appearance

			list_ptr1 = *pending_link_list;

			if (list_ptr1 == NULL) {
				result = -1;
				break;
			}

			if (list_ptr1->linkObjID == linkObjID) {

				*pending_link_list = list_ptr1->next;
				result = list_ptr1->parentObjID;
				delete(list_ptr1);
				break;
			}

			while(list_ptr1->next && (list_ptr1->next->linkObjID != linkObjID)) {

				list_ptr1 = list_ptr1->next;
			}

			if (list_ptr1->next) {
				result = list_ptr1->next->parentObjID;
				list_ptr2 = list_ptr1->next;
				list_ptr1->next = list_ptr1->next->next;
				delete(list_ptr2);
			} else {
				result = -1;
			}
			break;

	default:

			break;


	}

	return result;

}
