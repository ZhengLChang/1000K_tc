#ifndef MAC_GENARATE_
#define MAC_GENARATE_
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

class MacGenerate{
  public:
    MacGenerate(const int plusNum = 0, const char baseMac[]="00:01:02:03:04:09");
    virtual ~MacGenerate();
    char *get();
  private:
    char mac[36];
};

#endif
