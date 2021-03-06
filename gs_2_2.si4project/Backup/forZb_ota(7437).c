#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


#include "zj_type.h"
#include "zj_log.h"
#include "forZb_comCfg.h"

#include "forZb_ota.h"

#include "dev_zj_funReg.h"

#include "dev_systemSup.h"

 //#define "\mnt\1"   ///opt/httpServer/lighttpd/htdocs/sd

//#define FIRMWARE_PATH_FILE    "/opt/httpServer/lighttpd/htdocs/sd/testGrade/file.bin"
#define FIRMWARE_PATH_FILE    "/opt/httpServer/lighttpd/htdocs/sd/testGrade/GS_DEVICE_SDK.tar.bz2"

#define FIRMWARE_PATH         "/opt/httpServer/lighttpd/htdocs/sd/testGrade"

int ota_msg_callback_reset(ST_ota_message *pOtaMsg){
	pOtaMsg->triggerStaTsk = 0;
	pOtaMsg->omRecFileSize = 0;
	pOtaMsg->timeout_loop = 0;
    //delete file

	
	if(pOtaMsg->fHandle != NULL)
		fclose(pOtaMsg->fHandle);
	return 0;
}

int ota_msg_callback_init(ST_ota_message *pOtaMsg,u32 initFileSize){
	char acCmdBuf[256] = "";
	int retSys;
	pOtaMsg->triggerStaTsk = 1;
	pOtaMsg->omFileSize = initFileSize;
	pOtaMsg->omRecFileSize = 0;
	pOtaMsg->timeout_loop = 210;
	pOtaMsg->fmFileName = FIRMWARE_PATH_FILE;
	pOtaMsg->pFun_destory = ota_msg_callback_reset;

	if( !access(FIRMWARE_PATH,F_OK)){
		printf("FIRMWARE_PATH exist\n");
	}else{
		snprintf(acCmdBuf, sizeof(acCmdBuf), "mkdir -p %s", FIRMWARE_PATH);
		system(acCmdBuf);
	}		 //delete file	
	
	pOtaMsg->fHandle = fopen(pOtaMsg->fmFileName,"wb+");
	if(pOtaMsg->fHandle == NULL){
		ZJ_LOG_INF("delete file fail!\n");
		return -1;
	}  
	ZJ_LOG_INF("clear firmware file succeed!\n");

	memset(acCmdBuf,0,sizeof(acCmdBuf));	
	snprintf(acCmdBuf, sizeof(acCmdBuf), "chmod 777 %s", FIRMWARE_PATH_FILE);
	retSys = system(acCmdBuf);
	ZJ_LOG_INF("retsys:%d!\n",retSys);
	memset(acCmdBuf,0,sizeof(acCmdBuf));	
	snprintf(acCmdBuf, sizeof(acCmdBuf), "ls -l %s", FIRMWARE_PATH);
	retSys = system(acCmdBuf);
	ZJ_LOG_INF("retsys:%d!\n",retSys);	

	return 0;
}




int fzb_NewVersionCb(unsigned char *pucNewVersion,unsigned int uiFileSize){
	ZJ_LOG_INF("NewVersionCb exe,NewVersion[%s],FileSize[%d]\n",pucNewVersion,uiFileSize);

	if(pucNewVersion == NULL || uiFileSize == 0)
		return -1;	
	
	if( ota_msg_callback_init(RetG_ota_msg(),uiFileSize) != 0)
		return -1;		
	if(dj_ota_GtFTbl()->f_StartUpdate == NULL)
		return -1;	
	dj_ota_GtFTbl()->f_StartUpdate();

//	ZJ_StartUpdate();
	return 0;
}
int fzb_VersonDataDownCb(unsigned char *pucPackage,unsigned int uiPacklen,unsigned int uiEndFlag){
	int retfw;
//	ZJ_LOG_INF("VersonDataDownCb exe,Packlen[%d],EndFlag[%d],triggerStaTsk_1424[%d]\n",uiPacklen,uiEndFlag,RetG_ota_msg()->triggerStaTsk);
	if(RetG_ota_msg()->triggerStaTsk == 2){  //运行状态
		RetG_ota_msg()->omRecFileSize += uiPacklen;
		u32 pro_per = (RetG_ota_msg()->omRecFileSize) *100 /(RetG_ota_msg()->omFileSize) ;
		ZJ_LOG_INF("RetG_ota_msg()->omRecFileSize[%d],pro_per= %d",RetG_ota_msg()->omRecFileSize,pro_per);		
	//	ZJ_LOG_INF("pro_per= %d\n",pro_per);
		if(RetG_ota_msg()->fHandle == NULL){
			ZJ_LOG_INF("fHandle = NULL err!\n");
			return -1;
		}		
		retfw = fwrite(pucPackage, 1, uiPacklen, RetG_ota_msg()->fHandle );		
		if(retfw == 0 ){
			ZJ_LOG_INF("fwrite == 0\n");
			fclose(RetG_ota_msg()->fHandle);
			return -1;
		} 
		if(dj_ota_GtFTbl()->f_SetBurnningProgress == NULL)
			return -1;		
		dj_ota_GtFTbl()->f_SetBurnningProgress( pro_per );

		if(uiEndFlag == 1){	
			fclose(RetG_ota_msg()->fHandle);
			if(RetG_ota_msg()->omRecFileSize == RetG_ota_msg()->omFileSize){
				ZJ_LOG_INF("omRecFileSize==omFileSize,rec succeed\n");	
				RetG_ota_msg()->triggerStaTsk = 3; //last release state				
			}else{
				ZJ_LOG_INF("omRecFileSize[%d],omFileSize[%d]\n",RetG_ota_msg()->omRecFileSize,RetG_ota_msg()->omFileSize);	
				RetG_ota_msg()->triggerStaTsk = 5;
				return -1;
			}
		}
	}
	return 0;
}
int fzb_StopUpgrade(){
	ZJ_LOG_INF("StopUpgrade exe\n");
	RetG_ota_msg()->interruptTsk = 1;
	return 0;
}

int fzb_CoverImageNotice(unsigned int uiCoverFlag){
	ZJ_LOG_INF("CoverImageNotice exe,CoverFlag[%d]\n",uiCoverFlag);
	return 0;
}
