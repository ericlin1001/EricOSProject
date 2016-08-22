#ifndef SHELL_H
#define SHELL_H
#include "type.h"

void shell();

void dealWithCMD(const char *cmd);

void help();

void monitorSchedule();
void jumpToShell();
void help();

void initShell();
#endif