#ifndef FORZB_CHANNEL_H_
#define FORZB_CHANNEL_H_

#include "zj_type.h"


/*S
int fzb_SetWifi(EN_ZJ_NETWORK_TYPE enWifiType, char* pcSSID, char* pcPwd, int iEncrytType);
int fzb_GetWifi(ST_ZJ_WIFI_INFO stWifiInfo[16], unsigned int *puiWifiCount);
int fzb_GetNetInfo(ST_ZJ_NETWORK_INFO* pstNetInfo);

*/
int fzb_OnRecvCustomData(const unsigned char* pucPeerID, unsigned char* pucData, int iLen);



#endif




