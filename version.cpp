#define CPS_HISRECORD_VERSION_STR "0.1.2"
#define CPS_HISRECORD_REMARK_STR  "modify format to yyyy-mm-dd hh24:mi:ss.ff. by yanglibin 2019-01-03,V0.1.2.\
Repair in the  successful cases still write error folder bug. by yanglibin 2018-04-20,V0.1.1.\
Create project for history record. by yanglibin 2018-04-20,V0.1.0."

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