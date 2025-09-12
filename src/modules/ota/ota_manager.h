// ota_manager.h
#pragma once
#include <HTTPUpdate.h>

void otaManagerInit();
void otaManagerHandle();
void otaManagerSetUrl(const char *url);
