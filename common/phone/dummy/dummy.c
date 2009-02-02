/* Copyright (c) 2009 by Michal Čihař */

/**
 * \file dummy.c
 * @author Michal Čihař
 */
/**
 * @addtogroup Phone
 * @{
 */
/**
 * \defgroup DummyPhone Dummy phone driver
 * This driver emulates phone operations on storage on filesyste,
 *
 * @{
 */

#include "../../gsmstate.h"

#include <string.h>
#include <time.h>
#include <errno.h>

#include "../../gsmcomon.h"
#include "../../misc/coding/coding.h"
#include "../../misc/locales.h"
#include "../../misc/misc.h"
#include "../pfunc.h"
#include "dummy.h"


GSM_Error DUMMY_Error(GSM_StateMachine *s)
{
	int i;
	i = errno;
	GSM_OSErrorInfo(s, "Failed to open log");
	if (i == ENOENT) {
		return ERR_EMPTY;
	} else if (i == EACCES) {
		return ERR_PERMISSION;
	} else {
		return ERR_UNKNOWN;
	}
}

char * DUMMY_GetFilePath(GSM_StateMachine *s, const char *filename)
{
	char *log_file;

	log_file = (char *)malloc(strlen(filename) + strlen(s->CurrentConfig->Device) + 2);

	strcpy(log_file, s->CurrentConfig->Device);
	strcat(log_file, "/");
	strcat(log_file, filename);

	return log_file;
}

int DUMMY_GetCount(GSM_StateMachine *s, const char *dirname)
{
	char *full_name;
	int i;
	FILE *f;
	int count = 0;

	full_name = (char *)malloc(strlen(dirname) + strlen(s->CurrentConfig->Device) + 20);

	for (i = 1; i <= DUMMY_MAX_LOCATION; i++) {
		sprintf(full_name, "%s/%s/%d", s->CurrentConfig->Device, dirname, i);
		f = fopen(full_name, "r");
		if (f == NULL) continue;
		count++;
		fclose(f);
	}
	return count;
}

int DUMMY_GetFirstFree(GSM_StateMachine *s, const char *dirname)
{
	char *full_name;
	int i;
	FILE *f;

	full_name = (char *)malloc(strlen(dirname) + strlen(s->CurrentConfig->Device) + 20);

	for (i = 1; i <= DUMMY_MAX_LOCATION; i++) {
		sprintf(full_name, "%s/%s/%d", s->CurrentConfig->Device, dirname, i);
		f = fopen(full_name, "r");
		if (f == NULL) return i;
		fclose(f);
	}
	return -1;
}

int DUMMY_GetNext(GSM_StateMachine *s, const char *dirname, int current)
{
	char *full_name;
	int i;
	FILE *f;

	full_name = (char *)malloc(strlen(dirname) + strlen(s->CurrentConfig->Device) + 20);

	for (i = current + 1; i <= DUMMY_MAX_LOCATION; i++) {
		sprintf(full_name, "%s/%s/%d", s->CurrentConfig->Device, dirname, i);
		f = fopen(full_name, "r");
		if (f != NULL) {
			return i;
			fclose(f);
		}
	}
	return -1;
}

char * DUMMY_GetSMSPath(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	char smspath[100];
	bool setfolder = (sms->Folder == 0);
	while (sms->Location >= DUMMY_MAX_SMS) {
		sms->Location -= DUMMY_MAX_SMS;
		if (setfolder) {
			sms->Folder++;
		}
	}

	sprintf(smspath, "sms/%d/%d", sms->Folder, sms->Location);
	return DUMMY_GetFilePath(s, smspath);
}

/**
 * Initialises dummy module.
 */
GSM_Error DUMMY_Initialise(GSM_StateMachine *s)
{
	GSM_Phone_DUMMYData	*Priv = &s->Phone.Data.Priv.DUMMY;
	char *log_file;
	int i;

	log_file = DUMMY_GetFilePath(s, "operations.log");

	smprintf(s, "Log file %s\n", log_file);

	Priv->log_file = fopen(log_file, "w");
	free(log_file);
	if (Priv->log_file == NULL) {
		i = errno;
		GSM_OSErrorInfo(s, "Failed to open log");
		if (i == ENOENT) return ERR_DEVICENOTEXIST;		/* no such file or directory */
		if (i == EACCES) return ERR_DEVICENOPERMISSION;	/* permission denied */
		return ERR_DEVICEOPENERROR;
	}

	strcpy(s->Phone.Data.IMEI, "999999999999999");
	strcpy(s->Phone.Data.Manufacturer, "Gammu");
	strcpy(s->Phone.Data.Model, "Dummy");
	strcpy(s->Phone.Data.Version, VERSION);
	strcpy(s->Phone.Data.VerDate, __DATE__);

	EncodeUnicode(Priv->SMSC.Number, "123456", 6);
	EncodeUnicode(Priv->SMSC.Name, "Default", 7);
	Priv->SMSC.Validity.Format = SMS_Validity_NotAvailable;
	Priv->SMSC.Validity.Relative = SMS_VALID_Max_Time;
	Priv->SMSC.DefaultNumber[0] = 0;
	Priv->SMSC.DefaultNumber[1] = 0;
	Priv->SMSC.Format = SMS_FORMAT_Text;

	s->Phone.Data.VerNum = VERSION_NUM;

	return ERR_NONE;
}

/**
 * Terminates dummy module.
 */
GSM_Error DUMMY_Terminate(GSM_StateMachine *s)
{
	GSM_Phone_DUMMYData	*Priv = &s->Phone.Data.Priv.DUMMY;
	fclose(Priv->log_file);
	return ERR_NONE;
}


/**
 * We should not dispatch any messages.
 */
GSM_Error DUMMY_DispatchMessage(GSM_StateMachine *s)
{
	return ERR_UNKNOWN;
}

GSM_Error DUMMY_GetProductCode(GSM_StateMachine *s, char *value)
{
       strcpy(value, "DUMMY-001");
       return ERR_NONE;
}

GSM_Error DUMMY_GetIMEI (GSM_StateMachine *s)
{
	strcpy(s->Phone.Data.IMEI, "999999999999999");
	return ERR_NONE;
}

GSM_Error DUMMY_GetOriginalIMEI (GSM_StateMachine *s, char *value)
{
	strcpy(value, "666666666666666");
	return ERR_NONE;
}

GSM_Error DUMMY_GetManufactureMonth (GSM_StateMachine *s, char *value)
{
	strcpy(value, "April");
	return ERR_NONE;
}

GSM_Error DUMMY_GetHardware (GSM_StateMachine *s, char *value)
{
	strcpy(value, "FOO DUMMY BAR");
	return ERR_NONE;
}

GSM_Error DUMMY_GetPPM (GSM_StateMachine *s, char *value)
{
	strcpy(value, "EN CS");
	return ERR_NONE;
}

GSM_Error DUMMY_GetFirmware(GSM_StateMachine *s)
{
	strcpy(s->Phone.Data.Version, VERSION);
	strcpy(s->Phone.Data.VerDate, __DATE__);
	return ERR_NONE;
}

GSM_Error DUMMY_GetModel(GSM_StateMachine *s)
{
	strcpy(s->Phone.Data.Model, "Dummy");
	return ERR_NONE;
}

GSM_Error DUMMY_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	GSM_GetCurrentDateTime(date_time);
	return ERR_NONE;
}

GSM_Error DUMMY_GetSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms)
{
	GSM_SMS_Backup		Backup;
	char *filename;
	GSM_Error error;
	GSM_SMSMessage *SMS;
	int location, folder;
	int i = 0;

	location = sms->SMS[0].Location;
	folder = sms->SMS[0].Folder;

	filename = DUMMY_GetSMSPath(s, &(sms->SMS[0]));

	error = GSM_ReadSMSBackupFile(filename, &Backup);

	free(filename);

	if (error != ERR_NONE) {
		if (error == ERR_CANTOPENFILE) return ERR_EMPTY;
		return error;
	}

	sms->Number = 0;

	for (SMS = Backup.SMS[i]; SMS != NULL; SMS = Backup.SMS[++i]) {
		sms->Number++;
		sms->SMS[i] = *Backup.SMS[i];
		sms->SMS[i].Location = location + (folder * DUMMY_MAX_SMS);
		sms->SMS[i].Folder = folder;
	}

	return ERR_NONE;
}

GSM_Error DUMMY_DeleteSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	char *filename;
	GSM_Error error;

	filename = DUMMY_GetSMSPath(s, sms);

	if (unlink(filename) == 0) {
		error = ERR_NONE;
	} else {
		error = DUMMY_Error(s);
	}

	free(filename);

	return error;
}

GSM_Error DUMMY_SetSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	char *filename;
	GSM_Error error;
	GSM_SMS_Backup backup;

	error = DUMMY_DeleteSMS(s, sms);
	if (error != ERR_EMPTY && error != ERR_NONE) return error;

	filename = DUMMY_GetSMSPath(s, sms);

	backup.SMS[0] = sms;
	backup.SMS[1] = NULL;

	return GSM_AddSMSBackupFile(filename, &backup);
}

GSM_Error DUMMY_AddSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	char dirname[20];

	sprintf(dirname, "sms/%d", sms->Folder);
	sms->Location = DUMMY_GetFirstFree(s, dirname);

	if (sms->Location == -1) return ERR_FULL;

	return DUMMY_SetSMS(s, sms);
}

GSM_Error DUMMY_GetSignalStrength(GSM_StateMachine *s, GSM_SignalQuality *sig)
{
	sig->SignalStrength = 42;
	sig->SignalPercent = 42;
	sig->BitErrorRate = 0;
	return ERR_NONE;
}

GSM_Error DUMMY_GetSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
	PHONE_GetSMSFolders(s,folders);
	folders->Number = 5;

	CopyUnicodeString(folders->Folder[2].Name,folders->Folder[0].Name);
	folders->Folder[2].InboxFolder = folders->Folder[0].InboxFolder;
	folders->Folder[2].OutboxFolder = folders->Folder[0].OutboxFolder;
	folders->Folder[2].Memory = MEM_ME;

	CopyUnicodeString(folders->Folder[3].Name,folders->Folder[1].Name);
	folders->Folder[3].InboxFolder = folders->Folder[1].InboxFolder;
	folders->Folder[3].OutboxFolder = folders->Folder[1].OutboxFolder;
	folders->Folder[3].Memory = MEM_ME;

	EncodeUnicode(folders->Folder[4].Name,_("Templates"),strlen(_("Templates")));
	folders->Folder[4].InboxFolder = false;
	folders->Folder[4].OutboxFolder = false;
	folders->Folder[4].Memory = MEM_ME;
	return ERR_NONE;
}

GSM_Error DUMMY_GetNextSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start)
{
	char dirname[20];

	if (start) {
		sms->SMS[0].Folder = 1;
		sms->SMS[0].Location = 0;
	}

folder:
	sprintf(dirname, "sms/%d", sms->SMS[0].Folder);

	/* Convert location */
	free(DUMMY_GetSMSPath(s, &(sms->SMS[0])));

	sms->SMS[0].Location = DUMMY_GetNext(s, dirname, sms->SMS[0].Location);

	if (sms->SMS[0].Location == -1) {
		if (sms->SMS[0].Folder >= 5) return ERR_EMPTY;
		sms->SMS[0].Folder++;
		goto folder;
	}

	return DUMMY_GetSMS(s, sms);
}

GSM_Error DUMMY_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
	char dirname[20];
	sprintf(dirname, "sms/%d", 5);
	status->TemplatesUsed	= DUMMY_GetCount(s, dirname);

	sprintf(dirname, "sms/%d", 1);
	status->SIMUsed		= DUMMY_GetCount(s, dirname);
	sprintf(dirname, "sms/%d", 2);
	status->SIMUsed		+= DUMMY_GetCount(s, dirname);
	status->SIMUnRead 	= 0;
	status->SIMSize		= DUMMY_MAX_SMS;

	sprintf(dirname, "sms/%d", 3);
	status->PhoneUsed	= DUMMY_GetCount(s, dirname);
	sprintf(dirname, "sms/%d", 4);
	status->PhoneUsed	+= DUMMY_GetCount(s, dirname);
	status->PhoneUsed	+= status->TemplatesUsed;
	status->PhoneUnRead 	= 0;
	status->PhoneSize	= DUMMY_MAX_SMS;
	return ERR_NONE;
}

GSM_Error DUMMY_DialVoice(GSM_StateMachine *s, char *number, GSM_CallShowNumber ShowNumber)
{
	GSM_Phone_DUMMYData	*Priv = &s->Phone.Data.Priv.DUMMY;

	fprintf(Priv->log_file, "Dialling %s\n", number);

	return ERR_NONE;
}

GSM_Error DUMMY_DialService(GSM_StateMachine *s, char *number)
{
	GSM_Phone_DUMMYData	*Priv = &s->Phone.Data.Priv.DUMMY;

	fprintf(Priv->log_file, "Dialling service %s\n", number);

	return ERR_NONE;
}

GSM_Error DUMMY_AnswerCall(GSM_StateMachine *s, int ID, bool all)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetNetworkInfo(GSM_StateMachine *s, GSM_NetworkInfo *netinfo)
{
	strcpy(netinfo->CID, "FACE");
	strcpy(netinfo->NetworkCode, "999 99");
	netinfo->State = GSM_HomeNetwork;
	strcpy(netinfo->LAC, "B00B");
	EncodeUnicode(netinfo->NetworkName, "NasraT3l", 8);

	return ERR_NONE;
}

GSM_Error DUMMY_GetDisplayStatus(GSM_StateMachine *s, GSM_DisplayFeatures *features)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_SetAutoNetworkLogin(GSM_StateMachine *s)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_PressKey(GSM_StateMachine *s, GSM_KeyCode Key, bool Press)
{
	GSM_Phone_DUMMYData	*Priv = &s->Phone.Data.Priv.DUMMY;

	if (Press) {
		fprintf(Priv->log_file, "Pressing key %d\n", Key);
	} else {
		fprintf(Priv->log_file, "Releasing key %d\n", Key);
	}

	return ERR_NONE;
}

GSM_Error DUMMY_Reset(GSM_StateMachine *s, bool hard)
{
	GSM_Phone_DUMMYData	*Priv = &s->Phone.Data.Priv.DUMMY;

	if (hard) {
		fprintf(Priv->log_file, "Performing hard reset!\n");
	} else {
		fprintf(Priv->log_file, "Performing soft reset!\n");
	}

	return ERR_NONE;
}

GSM_Error DUMMY_CancelCall(GSM_StateMachine *s, int ID, bool all)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_SendSavedSMS(GSM_StateMachine *s, int Folder, int Location)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_SendSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_SetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	return ERR_NONE;
}

GSM_Error DUMMY_SetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	GSM_Phone_DUMMYData	*Priv = &s->Phone.Data.Priv.DUMMY;

	if (smsc->Location != 1) return ERR_NOTSUPPORTED;

	Priv->SMSC = *smsc;

	return ERR_NONE;
}

GSM_Error DUMMY_GetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	GSM_Phone_DUMMYData	*Priv = &s->Phone.Data.Priv.DUMMY;

	if (smsc->Location != 1) return ERR_EMPTY;

	*smsc = Priv->SMSC;

	return ERR_NONE;
}

GSM_Error DUMMY_EnterSecurityCode(GSM_StateMachine *s, GSM_SecurityCode Code)
{
	GSM_Phone_DUMMYData	*Priv = &s->Phone.Data.Priv.DUMMY;

	fprintf(Priv->log_file, "Security code %d: %s\n", Code.Type, Code.Code);

	return ERR_NONE;
}

GSM_Error DUMMY_GetSecurityStatus(GSM_StateMachine *s, GSM_SecurityCodeType *Status)
{
	*Status = SEC_None;
	return ERR_NONE;
}

GSM_Error DUMMY_ResetPhoneSettings(GSM_StateMachine *s, GSM_ResetSettingsType Type)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_SendDTMF(GSM_StateMachine *s, char *sequence)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetSIMIMSI(GSM_StateMachine *s, char *IMSI)
{
	strcpy(IMSI, "994299429942994");
	return ERR_NONE;
}

GSM_Error DUMMY_SetIncomingCall (GSM_StateMachine *s, bool enable)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_SetIncomingCB (GSM_StateMachine *s, bool enable)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_SetIncomingSMS (GSM_StateMachine *s, bool enable)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_SetFastSMSSending(GSM_StateMachine *s, bool enable)
{
	return ERR_NONE;
}

GSM_Error DUMMY_GetManufacturer(GSM_StateMachine *s)
{
	strcpy(s->Phone.Data.Manufacturer, "Gammu");
	return ERR_NONE;
}

GSM_Error DUMMY_GetAlarm(GSM_StateMachine *s, GSM_Alarm *Alarm)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_SetAlarm(GSM_StateMachine *s, GSM_Alarm *Alarm)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_SetIncomingUSSD(GSM_StateMachine *s, bool enable)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, bool PhoneRingtone)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_SetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, int *maxlength)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_SetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_AddFilePart(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_SendFilePart(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetFilePart(GSM_StateMachine *s, GSM_File *File, int *Handle, int *Size)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetNextFileFolder(GSM_StateMachine *s, GSM_File *File, bool start)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_DeleteFile(GSM_StateMachine *s, unsigned char *ID)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_AddFolder(GSM_StateMachine *s, GSM_File *File)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *Status)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetNextMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry, bool start)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_SetMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_AddMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_DeleteMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_DeleteAllMemory(GSM_StateMachine *s, GSM_MemoryType type)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetToDoStatus(GSM_StateMachine *s, GSM_ToDoStatus *status)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetToDo (GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetNextToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo, bool start)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_DeleteAllToDo (GSM_StateMachine *s)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_AddToDo (GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_SetToDo (GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_DeleteToDo (GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetCalendarStatus(GSM_StateMachine *s, GSM_CalendarStatus *status)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_DeleteCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_AddCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_SetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_DeleteAllCalendar (GSM_StateMachine *s)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetNoteStatus(GSM_StateMachine *s, GSM_ToDoStatus *status)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetNote (GSM_StateMachine *s, GSM_NoteEntry *Note)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetNextNote(GSM_StateMachine *s, GSM_NoteEntry *Note, bool start)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_DeleteAllNotes(GSM_StateMachine *s)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_AddNote (GSM_StateMachine *s, GSM_NoteEntry *Note)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_SetNote (GSM_StateMachine *s, GSM_NoteEntry *Note)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_DeleteNote (GSM_StateMachine *s, GSM_NoteEntry *Note)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetLocale(GSM_StateMachine *s, GSM_Locale *locale)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_SetLocale(GSM_StateMachine *s, GSM_Locale *locale)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error DUMMY_GetFileSystemStatus(GSM_StateMachine *s, GSM_FileSystemStatus *Status)
{
	Status->Free = 10101;
	Status->Used = 1000000;
	Status->UsedImages = 0;
	Status->UsedSounds = 0;
	Status->UsedThemes = 0;
	return ERR_NONE;
}

GSM_Error DUMMY_GetBatteryCharge(GSM_StateMachine *s, GSM_BatteryCharge *bat)
{
	bat->BatteryPercent = 100;
	bat->ChargeState = GSM_BatteryConnected;
	bat->BatteryVoltage = 4200;
	bat->ChargeVoltage = 4200;
	bat->ChargeCurrent = 0;
	bat->PhoneCurrent = 500;
	bat->BatteryTemperature = 22;
	bat->PhoneTemperature = 22;
	bat->BatteryCapacity = 2000;
	bat->BatteryType = GSM_BatteryLiPol;
	return ERR_NONE;
}

/*@}*/

GSM_Reply_Function DUMMYReplyFunctions[] = {
{NULL,				"\x00"			,0x00,0x00,ID_None		 }
};

GSM_Phone_Functions DUMMYPhone = {
	"dummy",
	DUMMYReplyFunctions,
	DUMMY_Initialise,
	DUMMY_Terminate,
	DUMMY_DispatchMessage,
	NOTSUPPORTED,			/* 	ShowStartInfo		*/
	DUMMY_GetManufacturer,
	DUMMY_GetModel,
	DUMMY_GetFirmware,
	DUMMY_GetIMEI,
	DUMMY_GetOriginalIMEI,
	DUMMY_GetManufactureMonth,
        DUMMY_GetProductCode,
        DUMMY_GetHardware,
        DUMMY_GetPPM,
	DUMMY_GetSIMIMSI,
	DUMMY_GetDateTime,
	DUMMY_SetDateTime,
	DUMMY_GetAlarm,
	DUMMY_SetAlarm,
	DUMMY_GetLocale,
	DUMMY_SetLocale,
	DUMMY_PressKey,
	DUMMY_Reset,
	DUMMY_ResetPhoneSettings,
	DUMMY_EnterSecurityCode,
	DUMMY_GetSecurityStatus,
	DUMMY_GetDisplayStatus,
	DUMMY_SetAutoNetworkLogin,
	DUMMY_GetBatteryCharge,
	DUMMY_GetSignalStrength,
	DUMMY_GetNetworkInfo,
 	NOTSUPPORTED,       		/*  	GetCategory 		*/
 	NOTSUPPORTED,       		/*  	AddCategory 		*/
 	NOTSUPPORTED,        		/*  	GetCategoryStatus 	*/
	DUMMY_GetMemoryStatus,
	DUMMY_GetMemory,
	DUMMY_GetNextMemory,
	DUMMY_SetMemory,
	DUMMY_AddMemory,
	DUMMY_DeleteMemory,
	DUMMY_DeleteAllMemory,
	NOTSUPPORTED,			/* 	GetSpeedDial		*/
	NOTSUPPORTED,			/* 	SetSpeedDial		*/
	DUMMY_GetSMSC,
	DUMMY_SetSMSC,
	DUMMY_GetSMSStatus,
	DUMMY_GetSMS,
	DUMMY_GetNextSMS,
	DUMMY_SetSMS,
	DUMMY_AddSMS,
	DUMMY_DeleteSMS,
	DUMMY_SendSMS,
	DUMMY_SendSavedSMS,
	DUMMY_SetFastSMSSending,
	DUMMY_SetIncomingSMS,
	DUMMY_SetIncomingCB,
	DUMMY_GetSMSFolders,
 	NOTSUPPORTED,			/* 	AddSMSFolder		*/
 	NOTSUPPORTED,			/* 	DeleteSMSFolder		*/
	DUMMY_DialVoice,
	DUMMY_DialService,
	DUMMY_AnswerCall,
	DUMMY_CancelCall,
 	NOTSUPPORTED,			/* 	HoldCall 		*/
 	NOTSUPPORTED,			/* 	UnholdCall 		*/
 	NOTSUPPORTED,			/* 	ConferenceCall 		*/
 	NOTSUPPORTED,			/* 	SplitCall		*/
 	NOTSUPPORTED,			/* 	TransferCall		*/
 	NOTSUPPORTED,			/* 	SwitchCall		*/
 	NOTSUPPORTED,			/* 	GetCallDivert		*/
 	NOTSUPPORTED,			/* 	SetCallDivert		*/
 	NOTSUPPORTED,			/* 	CancelAllDiverts	*/
	DUMMY_SetIncomingCall,
	DUMMY_SetIncomingUSSD,
	DUMMY_SendDTMF,
	DUMMY_GetRingtone,
	DUMMY_SetRingtone,
	NOTSUPPORTED,			/* 	GetRingtonesInfo	*/
	NOTSUPPORTED,			/* 	DeleteUserRingtones	*/
	NOTSUPPORTED,			/* 	PlayTone		*/
	NOTSUPPORTED,			/* 	GetWAPBookmark		*/
	NOTSUPPORTED,			/* 	SetWAPBookmark		*/
	NOTSUPPORTED,			/* 	DeleteWAPBookmark	*/
	NOTSUPPORTED,			/* 	GetWAPSettings		*/
	NOTSUPPORTED,			/* 	SetWAPSettings		*/
	NOTSUPPORTED,			/*	GetSyncMLSettings	*/
	NOTSUPPORTED,			/*	SetSyncMLSettings	*/
	NOTSUPPORTED,			/*	GetChatSettings		*/
	NOTSUPPORTED,			/*	SetChatSettings		*/
	NOTSUPPORTED,			/* 	GetMMSSettings		*/
	NOTSUPPORTED,			/* 	SetMMSSettings		*/
	NOTSUPPORTED,			/*	GetMMSFolders		*/
	NOTSUPPORTED,			/*	GetNextMMSFileInfo	*/
	DUMMY_GetBitmap,
	DUMMY_SetBitmap,
	DUMMY_GetToDoStatus,
	DUMMY_GetToDo,
	DUMMY_GetNextToDo,
	DUMMY_SetToDo,
	DUMMY_AddToDo,
	DUMMY_DeleteToDo,
	DUMMY_DeleteAllToDo,
	DUMMY_GetCalendarStatus,
	DUMMY_GetCalendar,
	DUMMY_GetNextCalendar,
	DUMMY_SetCalendar,
	DUMMY_AddCalendar,
	DUMMY_DeleteCalendar,
	DUMMY_DeleteAllCalendar,
	NOTSUPPORTED,			/* 	GetCalendarSettings	*/
	NOTSUPPORTED,			/* 	SetCalendarSettings	*/
	DUMMY_GetNoteStatus,
	DUMMY_GetNote,
	DUMMY_GetNextNote,
	DUMMY_SetNote,
	DUMMY_AddNote,
	DUMMY_DeleteNote,
	DUMMY_DeleteAllNotes,
	NOTSUPPORTED,			/* 	GetProfile		*/
	NOTSUPPORTED,			/* 	SetProfile		*/
	NOTSUPPORTED,			/* 	GetFMStation		*/
	NOTSUPPORTED,			/* 	SetFMStation		*/
	NOTSUPPORTED,			/* 	ClearFMStations		*/
	DUMMY_GetNextFileFolder,
	NOTSUPPORTED,			/*	GetFolderListing	*/
	NOTSUPPORTED,			/*	GetNextRootFolder	*/
	NOTSUPPORTED,			/*	SetFileAttributes	*/
	DUMMY_GetFilePart,
	DUMMY_AddFilePart,
	DUMMY_SendFilePart,
	DUMMY_GetFileSystemStatus,
	DUMMY_DeleteFile,
	DUMMY_AddFolder,
	DUMMY_DeleteFile,	/* 	DeleteFolder		*/
	NOTSUPPORTED,			/* 	GetGPRSAccessPoint	*/
	NOTSUPPORTED			/* 	SetGPRSAccessPoint	*/
};

/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */