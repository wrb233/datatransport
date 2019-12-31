#define CPS_HISRECORD_VERSION_STR "0.0.1"
#define CPS_HISRECORD_REMARK_STR  "extract some data from oms. by wanglei 2019-12-30,V0.0.1."

#include "version.h"

//打印版本信息函数
bool echoVersion(int argc, char *argv[]){
	for(int i = 0;i < argc;i++){
		if(qstricmp(argv[i],"-v") == 0 || qstricmp(argv[i],"--version") == 0 ){
			std::cout << "Version: " << CPS_HISRECORD_VERSION_STR << std::endl;
			std::cout << "Remark : " << CPS_HISRECORD_REMARK_STR  << std::endl;	
			return true;
		}
	}
	return  false;
}