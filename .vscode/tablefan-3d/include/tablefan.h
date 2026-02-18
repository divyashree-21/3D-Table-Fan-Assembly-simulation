#ifndef TABLEFAN_H
#define TABLEFAN_H

#include <stdbool.h>

// Structure to represent a table fan
typedef struct {
    char name[64];
    int speed; // Speed level of the fan
    bool isRunning; // Status of the fan
} TableFan;

// Function prototypes
void initializeFan(TableFan *fan, const char *name);
void startFan(TableFan *fan);
void stopFan(TableFan *fan);
void setFanSpeed(TableFan *fan, int speed);
void displayFanStatus(const TableFan *fan);

#endif // TABLEFAN_H