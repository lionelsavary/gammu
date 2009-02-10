/* (c) 2002-2004 by Marcin Wiacek */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#ifdef HAVE_WCHAR_H
#  include <wchar.h>
#endif
#ifdef HAVE_WCTYPE_H
#  include <wctype.h>
#endif

#include <gammu-config.h>
#include <gammu-inifile.h>
#include "coding/coding.h"

/**
 * Read information from file in Windows INI format style
 */
GSM_Error INI_ReadFile(const char *FileName, bool Unicode, INI_Section **result)
{
	FILE		*f;
	bool		FFEEUnicode=false;
	int		level = -1, buffer1used, buffer2used;
	size_t		bufferused, i, buffused=1000,buffread=1000, num;
	unsigned char	ch[3], *buffer = NULL;
	unsigned char	*buffer2 = NULL, *buffer1 = NULL, buff[1001];
        INI_Section 	*INI_info = NULL, *INI_head = NULL, *heading;
        INI_Entry 	*entry;
	GSM_Error	error = ERR_NONE;

	*result = NULL;

	f = fopen(FileName,"rb");
	if (f == NULL) return ERR_CANTOPENFILE;

	num = 0;
	while(1) {
		/* We read one line from file */
		bufferused = 0;
		while (1) {
			if (buffused == buffread) {
				buffused = fread(buff,1,1000,f);
				buffread = 0;
				if (buffused == 0) {
					error = ERR_NONE;
					goto done;
				}
			}
			if (Unicode) {
				if (num == 0) {
					if (buffused == buffread) continue;
					ch[0] = buff[buffread++];
					num = 1;
				}
				if (num == 1) {
					if (buffused == buffread) continue;
					ch[1] = buff[buffread++];
					num = 0;
				}
				if (level == -1) {
					if (ch[0] == 0xFF && ch[1] == 0xFE) FFEEUnicode = true;
					level = 0;
					continue;
				}
				if (FFEEUnicode) {
					ch[2] = ch[0]; ch[0] = ch[1]; ch[1] = ch[2];
				}
			} else {
				if (buffused == buffread) continue;
				ch[0] = 0;
				ch[1] = buff[buffread++];
				if (level == -1) level = 0;
			}
			if ((ch[0] == 0 && ch[1] == 13) ||
			    (ch[0] == 0 && ch[1] == 10)) {
				break;
			}
			buffer 			= realloc(buffer,bufferused+2);
                        if (buffer == NULL) {
                                error = ERR_MOREMEMORY;
				goto done;
                        }
			buffer[bufferused] 	= ch[0];
			buffer[bufferused+1] 	= ch[1];
			bufferused		= bufferused + 2;
		}

		buffer1used = 0;
		buffer2used = 0;
		if (level == 1) level = 0;
		if (level == 3 || level == 4 || level == 5) level = 2;

		/* We parse read line */
		for (i=0;i<bufferused/2;i++) {
			ch[0] = buffer[i*2];
			ch[1] = buffer[i*2+1];
			if (level == 0) { /* search for name of section */
				if (ch[0] == 0 && ch[1] == '[') level = 1;
				if (ch[0] == 0 && ch[1] == ';') break;
				if (ch[0] == 0 && ch[1] == '#') break;
				continue;
			}
			if (level == 1) { /* section name */
				if (ch[0] == 0 && ch[1] == ']') {
					if (buffer1used == 0) break;
					if (Unicode) {
						buffer1 		= realloc(buffer1,buffer1used+2);
						buffer1[buffer1used] 	= 0;
						buffer1[buffer1used+1] 	= 0;
						buffer1used		= buffer1used + 2;
					} else {
						buffer1 		= realloc(buffer1,buffer1used+1);
						buffer1[buffer1used] 	= 0x00;
						buffer1used		= buffer1used + 1;
					}
					heading = (INI_Section *)malloc(sizeof(*heading));
		                        if (heading == NULL) {
						error = ERR_MOREMEMORY;
						goto done;
		                        }
					heading->SectionName = (char *)malloc(buffer1used);
					memcpy(heading->SectionName,buffer1,buffer1used);
		                        heading->Prev = INI_info;
					heading->Next = NULL;
		                        if (INI_info != NULL) {
		                                INI_info->Next  = heading;
		                        } else {
		                                INI_head 	= heading;
		                        }
		                        INI_info 		= heading;
					INI_info->SubEntries 	= NULL;
					level 	 		= 2;
					break;
				}
				if (Unicode) {
					buffer1 		= realloc(buffer1,buffer1used+2);
					buffer1[buffer1used] 	= ch[0];
					buffer1[buffer1used+1] 	= ch[1];
					buffer1used		= buffer1used + 2;
				} else {
					buffer1 		= realloc(buffer1,buffer1used+1);
					buffer1[buffer1used] 	= ch[1];
					buffer1used		= buffer1used + 1;
				}
				continue;
			}
			if (level == 2) { /* search for key name */
				if (ch[0] == 0 && ch[1] == ';') break;
				if (ch[0] == 0 && ch[1] == '#') break;
				if (ch[0] == 0 && ch[1] == '[') {
					level = 1;
					continue;
				}
				if (Unicode) {
			                if (myiswspace(ch)) continue;
				} else {
			                if (isspace((int) ch[1])) continue;
				}
				level = 3;
			}
			if (level == 3) { /* key name */
				if (ch[0] == 0 && ch[1] == '=') {
					if (buffer1used == 0) break;
					while(1) {
						if (Unicode) {
					                if (!myiswspace(buffer1+(buffer1used-2))) break;
							buffer1used = buffer1used - 2;
						} else {
					                if (!isspace((int)buffer1[buffer1used-1])) break;
							buffer1used = buffer1used - 1;
						}
					}
					level = 4;
					continue;
				}
				if (Unicode) {
					buffer1 		= realloc(buffer1,buffer1used+2);
					buffer1[buffer1used] 	= ch[0];
					buffer1[buffer1used+1] 	= ch[1];
					buffer1used		= buffer1used + 2;
				} else {
					buffer1 		= realloc(buffer1,buffer1used+1);
					buffer1[buffer1used] 	= ch[1];
					buffer1used		= buffer1used + 1;
				}
			}
			if (level == 4) { /* search for key value */
				if (Unicode) {
			                if (myiswspace(ch)) continue;
				} else {
			                if (isspace((int) ch[1])) continue;
				}
				level = 5;
			}
			if (level == 5) { /* key value */
				if (Unicode) {
					buffer2 		= realloc(buffer2,buffer2used+2);
					buffer2[buffer2used] 	= ch[0];
					buffer2[buffer2used+1] 	= ch[1];
					buffer2used		= buffer2used + 2;
				} else {
					buffer2 		= realloc(buffer2,buffer2used+1);
					buffer2[buffer2used] 	= ch[1];
					buffer2used		= buffer2used + 1;
				}
			}
		}
		if (level == 5) {
			if (buffer2used == 0) continue;

			entry = (INI_Entry *)malloc(sizeof(*entry));
                        if (entry == NULL) {
                                error = ERR_MOREMEMORY;
				goto done;
                        }
			if (Unicode) {
				buffer1 		= realloc(buffer1,buffer1used+2);
				buffer1[buffer1used] 	= 0;
				buffer1[buffer1used+1] 	= 0;
				buffer1used		= buffer1used + 2;
				buffer2 		= realloc(buffer2,buffer2used+2);
				buffer2[buffer2used] 	= 0;
				buffer2[buffer2used+1] 	= 0;
				buffer2used		= buffer2used + 2;
			} else {
				buffer1 		= realloc(buffer1,buffer1used+1);
				buffer1[buffer1used] 	= 0x00;
				buffer1used		= buffer1used + 1;
				buffer2 		= realloc(buffer2,buffer2used+1);
				buffer2[buffer2used] 	= 0x00;
				buffer2used		= buffer2used + 1;
			}

			entry->EntryName = (char *)malloc(buffer1used);
			if (entry->EntryName == NULL) {
                                error = ERR_MOREMEMORY;
				goto done;
                        }
			memcpy(entry->EntryName,buffer1,buffer1used);

			entry->EntryValue = (char *)malloc(buffer2used);
			if (entry->EntryValue == NULL) {
                                error = ERR_MOREMEMORY;
				goto done;
                        }
			memcpy(entry->EntryValue,buffer2,buffer2used);

			entry->Prev = NULL;
                        entry->Next = INI_info->SubEntries;
                        if (INI_info->SubEntries != NULL) INI_info->SubEntries->Prev = entry;
                        INI_info->SubEntries = entry;
		}
	}
done:
	free(buffer); free(buffer1); free(buffer2);
	fclose(f);
	if (error == ERR_NONE) {
		*result = INI_head;
		if (INI_head == NULL) {
			error = ERR_FILENOTSUPPORTED;
		}
	}
	return error;
}

/*
 * Search for key value in file in Windows INI format style
 * Returns found value or NULL
 */
unsigned char *INI_GetValue(INI_Section *cfg, const unsigned char *section, const unsigned char *key, const bool Unicode)
{
        INI_Section 	*sec;
        INI_Entry  	*ent;

        if (cfg == NULL || section == NULL || key == NULL) return NULL;

	if (Unicode) {
	        /* Search for section */
		sec = cfg;
		while (sec != NULL) {
	                if (mywstrncasecmp(section, sec->SectionName, 0)) {
	                        /* Search for key inside section */
				ent = sec->SubEntries;
				while (ent != NULL) {
	                                if (mywstrncasecmp(key,ent->EntryName,0)) {
	                                        return ent->EntryValue;
	                                }
					ent = ent->Next;
	                        }
	                }
			sec = sec->Next;
		}
	} else {
	        /* Search for section */
		sec = cfg;
		while (sec != NULL) {
	                if (strcasecmp(section, sec->SectionName) == 0) {
	                        /* Search for key inside section */
				ent = sec->SubEntries;
				while (ent != NULL) {
	                                if (strcasecmp(key,ent->EntryName) == 0) {
	                                        return ent->EntryValue;
	                                }
					ent = ent->Next;
	                        }
	                }
			sec = sec->Next;
		}
	}
        return NULL;
}

/* Return last value in specified section */
INI_Entry *INI_FindLastSectionEntry(INI_Section *file_info, const unsigned char *section, const bool Unicode)
{
	INI_Section 	*h;
	INI_Entry	*e;

	e = NULL;
	/* First find our section */
        for (h = file_info; h != NULL; h = h->Next) {
		if (Unicode) {
			if (mywstrncasecmp(section, h->SectionName, 0)) {
				e = h->SubEntries;
				break;
			}
		} else {
			if (strcasecmp(section, h->SectionName) == 0) {
				e = h->SubEntries;
				break;
			}
		}
	}

	if (e == NULL) return NULL;

	/* Goes into last value in section */
	while (e->Next != NULL) e = e->Next;
	return e;
}

void INI_Free_Entries(INI_Entry *entry)
{
	INI_Entry *cur = entry, *next;

	if (cur == NULL) return;
	while (cur != NULL) {
		next = cur->Next;
		free(cur->EntryName);
		free(cur->EntryValue);
		free(cur);
		cur = next;
	}
}

void INI_Free(INI_Section *head)
{
	INI_Section *cur = head, *next;

	if (cur == NULL) return;
	while (cur != NULL) {
		next = cur->Next;
		free(cur->SectionName);
		INI_Free_Entries(cur->SubEntries);
		free(cur);
		cur = next;
	}
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */