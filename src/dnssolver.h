/*
 * dnssolver.h
 *
 *  Created on: 12 mar. 2018
 *      Author: lvalles
 */

#include "netbase.h"

//Function Prototypes
bool ngethostbyname (const char *pszName, std::vector<CNetAddr>& vIP);
void ChangetoDnsNameFormat (unsigned char* dns,unsigned char* host);
bool set_dns_servers(const char *strDNSSeed);
unsigned char* ReadName (unsigned char* reader,unsigned char* buffer,int* count);
