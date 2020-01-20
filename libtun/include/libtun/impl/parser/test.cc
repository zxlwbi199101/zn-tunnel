//
// Created by 孔德凯 on 2019/12/10.
//
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
//#include <direct.h>
#include <iostream>
#include "ParseConfig.h"
#define DEFAULT_INI_FILE "Autoconfig.ini"

int main(int argc, char* argv[])
{

    pcx::ParseConfig config;
    config.ReadConfig(DEFAULT_INI_FILE);
    std::string ProductModel = config.ReadString("Setting", "ProductModel", "666");
    int MPVersion = config.ReadFloat("Setting", "MPVersion", 1.0);
    std::string TimeOut = config.ReadString("Setting", "TimeOut", "60");

    std::cout << "ProductModel=" << ProductModel << std::endl;
    std::cout << "Version=" << MPVersion << std::endl;
    std::cout << "TimeOut=" << TimeOut << std::endl;

    return 0;
    /*

    char ProductModel[80];
    char iniFile[256];
    char *buffer;
    buffer = getcwd(NULL, 0);
    strcpy(iniFile, buffer);
    strcat(iniFile,DEFAULT_INI_FILE);

    Get_private_profile_string("Setting", "ProductModel", "SS", ProductModel, sizeof(ProductModel), iniFile);
    printf("ProductModel:%s\n",ProductModel);

    unsigned char enExtendBlock = Get_private_profile_int("FwSetting", "EnExtendBlock", 1, iniFile);
    printf("enExtendBlock:%d\n",enExtendBlock);

    int MPVersion = 100;
    Write_private_profile_int("Setting", "MPVersion", MPVersion, iniFile);

    sprintf(ProductModel,"ABCSFE!!221");
    Write_private_profile_string("Setting", "ProductModel", ProductModel, iniFile);
    */
}
