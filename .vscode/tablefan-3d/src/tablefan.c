#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tablefan.h"
#include "renderer.h"

// Function to simulate the assembly of the table fan
void assembleFan(struct Queue *assembly) {
    const char *parts[] = {"Base", "Stand", "Motor", "Blades", "Front Cover"};
    int nparts = sizeof(parts) / sizeof(parts[0]);
    
    for (int i = 0; i < nparts; i++) {
        enqueue(assembly, parts[i]);
        printf("🧩 Part assembled: %s (order %d)\n", parts[i], i + 1);
    }
}

// Function to simulate the disassembly of the table fan
void disassembleFan(struct Stack *disassembly) {
    char temp[64];
    int dorder = 1;

    printf("\n🔧 Disassembly steps:\n");
    while (pop(disassembly, temp)) {
        printf("  🧭 Removed: %s (step %d)\n", temp, dorder);
        dorder++;
    }
}

// Main function to run the table fan simulation
int main(void) {
    struct Queue assembly;
    struct Stack disassembly;
    initQueue(&assembly);
    initStack(&disassembly);

    printf("=== TABLE FAN SIMULATION ===\n");

    // Assemble the fan
    assembleFan(&assembly);

    // Simulate fan operation
    simulateFanRun();

    // Transfer queue to stack for disassembly
    char temp[64];
    while (dequeue(&assembly, temp)) {
        push(&disassembly, temp);
    }

    // Disassemble the fan
    disassembleFan(&disassembly);

    return 0;
}