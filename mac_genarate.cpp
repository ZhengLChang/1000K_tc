#include "mac_genarate.hpp"

MacGenerate::MacGenerate(const int plusNum, const char baseMac[]){
	unsigned mac_calc[6] = {0};
	int count;
  int _plusNum =  plusNum;
  strncpy(mac, baseMac, sizeof(mac));
		count = sscanf(mac, "%x:%x:%x:%x:%x:%x", 
				&mac_calc[5], &mac_calc[4], &mac_calc[3], &mac_calc[2], &mac_calc[1], &mac_calc[0]);
		if(count != 6)
		{
      std::cout << "read mac error" << std::endl;
      mac[0] = '\0';
      return;
		}
		mac_calc[0] += (_plusNum % (0xff + 1));
    _plusNum = _plusNum >> 8;
		mac_calc[1] += ((mac_calc[0] & (0x100))  >> 8) + (_plusNum % (0xff + 1));
		mac_calc[0] = mac_calc[0] & (0xff);
    _plusNum = _plusNum >> 8;
		mac_calc[2] += ((mac_calc[1] & (0x100))  >> 8) + (_plusNum % (0xff + 1));
		mac_calc[1] = mac_calc[1] & (0xff);
    _plusNum = _plusNum >> 8;
		mac_calc[3] += ((mac_calc[2] & (0x100))  >> 8) + (_plusNum % (0xff + 1));
		mac_calc[2] = mac_calc[2] & (0xff);
    _plusNum = _plusNum >> 8;
		mac_calc[4] += ((mac_calc[3] & (0x100))  >> 8) + (_plusNum % (0xff + 1));
		mac_calc[3] = mac_calc[3] & (0xff);
    _plusNum = _plusNum >> 8;
		mac_calc[5] += ((mac_calc[4] & (0x100))  >> 8) + (_plusNum % (0xff + 1));
		mac_calc[4] = mac_calc[4] & (0xff);
		if(mac_calc[5] > 0xff)
		{
      std::cout << mac_calc[5] << " invalid mac" << std::endl;
		}else{
		  sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", 
		  		mac_calc[5], mac_calc[4],mac_calc[3],mac_calc[2],mac_calc[1],mac_calc[0]);
    }
	return ;
}
MacGenerate::~MacGenerate(){}
char* MacGenerate::get(){
  return mac;
}
