#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "device_log.h"
#include "zj_type.h"
#include "zj_log.h"

#include "devCfg.h"
#include "dev_alarm.h"

#include "dev_zj_funReg.h"

#include "IPC_MediaBuff.h"
#include "sdk_commonstruct.h"
#include "sdkout_impl.h" 

#include "dev_network.h" 
#include "dev_hint.h"




int sysClrCmdByPath(char *path){
	if( access(path,F_OK) != 0){
		DPRI_INFO("path not exist\n");		
		return -1;
	}
	char acCmdBuf[256] = {0};

	snprintf(acCmdBuf, sizeof(acCmdBuf), "rm -r %s", path);
	system(acCmdBuf);	
	return 0;
}


static int AlarmExeBy_123(void *para){
	DPRI_INFO("test alarm exe1\n");
	return 0;
}


static int AlarmExeBy_motion(void *para){
	if( NULL == dj_camIot_GtFTbl()->f_IoTEventInPut){
		DPRI_INFO("f_IoTEventInPut no register!\n");
		return -1;			
	}	
	dj_camIot_GtFTbl()->f_IoTEventInPut(EN_ZJ_AIIOT_TYPE_MOTION, 0, EN_ZJ_MOTION_EVENT_MOTION);
	DPRI_INFO("AlarmExeBy_motion exe!");
	return 0;
}

static int AlarmExeBy_motion_body(void *para){
	if( NULL == dj_camIot_GtFTbl()->f_IoTEventInPut){
		DPRI_INFO("f_IoTEventInPut no register!\n");
		return -1;			
	}	
	dj_camIot_GtFTbl()->f_IoTEventInPut(EN_ZJ_AIIOT_TYPE_MOTION, 0, EN_ZJ_MOTION_EVENT_HUMAN);
	DPRI_INFO("AlarmExeBy_motion_body exe!\n");
	return 0;
}

static int AlarmExeBy_shortPre(void *para){
	DPRI_INFO("AlarmExeBy_shortPress:short reset\n");	
	dev_hintSoundP(DEVVOICE_POWERRESET);
	sysClrCmdByPath(PATH_RESETDELETE_SHORT);	
//SETNET_IMME_QRAP
#ifdef MMACRO_NOFLASH_T3_E1102
	d_triggerSetNet(SETNET_IMME_QRAP);//reset and set net flag to save //SETNET_QLINK
#endif	
		
	if(dj_system_GtFTbl()->f_CtrlDeviceId == NULL)
		return -1;
	dj_system_GtFTbl()->f_CtrlDeviceId(EN_ZJ_CTRLDID_EXITGROUP);
	return 0;
}
static int AlarmExeBy_longPre(void *para){
	DPRI_INFO("AlarmExeBy_longPress:long reset\n");
	sysClrCmdByPath(PATH_RESETDELETE_LONG);

#ifdef MMACRO_NOFLASH_T3_E1102
//	d_triggerSetNet(SETNET_QLINK);//reset and set net flag to save
#endif		
	if(dj_system_GtFTbl()->f_CtrlDeviceId == NULL)
		return -1;
	dj_system_GtFTbl()->f_CtrlDeviceId(EN_ZJ_CTRLDID_CLEAR);
	return 0;
}

#ifdef MMACRO_NOFLASH_T3_E1102	
static int AlarmExeBy_QRCode(void *para){

	T_SDK_ALARM_CALLBACK_PARAM *palarm = (T_SDK_ALARM_CALLBACK_PARAM *)para;	
	DPRI_INFO("palarm->QrString: %s\n",palarm->QrString);	
	d_QRcode_sendToSdk((u8 *)palarm->QrString);	
	
	return 0;
}

static int AlarmExeBy_commonRkey(void *para){
//	T_SDK_ALARM_CALLBACK_PARAM *palarm = (T_SDK_ALARM_CALLBACK_PARAM *)para;	
//	DPRI_INFO("palarm->QrString: %s\n",palarm->QrString);	
//	d_QRcode_sendToSdk((u8 *)palarm->QrString);	
	d_qlink_triggerRkey();
	return 0;
}

#endif


static ST_alarmExe gAlarmExe[] = {
{100,AlarmExeBy_123},
{E_VIDEO_MOTION,AlarmExeBy_motion},
{E_KEY_LONG_PRESS,AlarmExeBy_longPre},
{E_KEY_SHORT_PRESS,AlarmExeBy_shortPre},
{E_BODY_DETECT,AlarmExeBy_motion_body},
#ifdef MMACRO_NOFLASH_T3_E1102	
{E_QRCode_String,AlarmExeBy_QRCode},
{E_KEY_ORDINARY_PRESS,AlarmExeBy_commonRkey},

#endif	
};

ST_alarmExe *getAlarmArr(){
    return  &gAlarmExe[0];
}


u8 alarmExe_memSize(){
	return sizeof(gAlarmExe) / sizeof(ST_alarmExe);
}


int alarmcb_process(T_SDK_ALARM_CALLBACK_PARAM *param){
	int i;
//	DPRI_INFO("alarmExe_memSize() = %d",alarmExe_memSize());
	for (i = 0; i < alarmExe_memSize(); ++i){
	    if(  (getAlarmArr()+i)->al_type == param->e_alarm_type){
			(getAlarmArr()+i)->pfun(param);
			DPRI_INFO("has been alarmcb_process\n");
			return 0;
		}	 		
	}	
	DPRI_INFO("not exe alarmcb_process\n");
	return -1;
}





