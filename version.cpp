#define CPS_DATAMANUALSYNC_VERSION_STR "0.1.1"
#define CPS_DATAMANUALSYNC_REMARK_STR  "Add mvpoint and dpspoint data pickup 、add collect data for shanghai. by yanglibin 2020-02-26,V0.1.1.\
Create project for monitorjson by wanglei 2020-01-01,V0.1.0."

#include "version.h"

//打印版本信息函数
bool echoVersion(int argc, char *argv[]){
	for(int i = 0;i < argc;i++){
		if(qstricmp(argv[i],"-v") == 0 || qstricmp(argv[i],"--version") == 0 ){
			std::cout << "Version: " << CPS_DATAMANUALSYNC_VERSION_STR << std::endl;
			std::cout << "Remark : " << CPS_DATAMANUALSYNC_REMARK_STR  << std::endl;	
			return true;
		}
	}
	return  false;
}