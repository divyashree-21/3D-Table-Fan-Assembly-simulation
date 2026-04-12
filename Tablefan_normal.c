#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 10

// ---------- QUEUE (Assembly Order) ----------
struct Queue {
    char parts[MAX][50];
    int front, rear;
};

// ---------- STACK (Undo) ----------
struct Stack {
    char parts[MAX][50];
    int top;
};

struct Queue q;
struct Stack s;

// ---------- QUEUE FUNCTIONS ----------
void initQueue() {
    q.front = q.rear = -1;
}

int isQueueEmpty() {
    return q.front == -1;
}

void enqueue(char part[]) {
    if (q.rear == MAX - 1) {
        printf("Queue Full!\n");
        return;
    }
    if (q.front == -1) q.front = 0;
    q.rear++;
    strcpy(q.parts[q.rear], part);
}

char* dequeue() {
    if (isQueueEmpty()) return NULL;

    char *part = q.parts[q.front];

    if (q.front == q.rear)
        q.front = q.rear = -1;
    else
        q.front++;

    return part;
}

// ---------- STACK FUNCTIONS ----------
void initStack() {
    s.top = -1;
}

int isStackEmpty() {
    return s.top == -1;
}

void push(char part[]) {
    if (s.top == MAX - 1) {
        printf("Stack Full!\n");
        return;
    }
    s.top++;
    strcpy(s.parts[s.top], part);
}

char* pop() {
    if (isStackEmpty()) return NULL;
    return s.parts[s.top--];
}

// ---------- DISPLAY ----------
void showAssembled() {
    printf("\n🔧 Assembled Parts:\n");
    if (isStackEmpty()) {
        printf("None\n");
        return;
    }
    for (int i = 0; i <= s.top; i++) {
        printf("%d. %s\n", i + 1, s.parts[i]);
    }
}

// ---------- MAIN ----------
int main() {
    int choice;

    initQueue();
    initStack();

    // Add fan parts in order
    enqueue("Base");
    enqueue("Stand");
    enqueue("Motor");
    enqueue("Blades");
    enqueue("Guard");

    printf("🌀 TABLE FAN ASSEMBLY (DSA - Queue + Stack)\n");

    while (1) {
        printf("\n===== MENU =====\n");
        printf("1. Assemble Next Part\n");
        printf("2. Undo Last Step\n");
        printf("3. Show Assembled Parts\n");
        printf("4. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {

        case 1: {
            char *part = dequeue();
            if (part == NULL) {
                printf("✅ All parts assembled!\n");
            } else {
                printf("🔧 Assembling: %s\n", part);
                push(part);
            }
            break;
        }

        case 2: {
            char *part = pop();
            if (part == NULL) {
                printf("⚠️ Nothing to undo!\n");
            } else {
                printf("↩️ Removed: %s\n", part);
                enqueue(part); // optional: add back to queue
            }
            break;
        }

        case 3:
            showAssembled();
            break;

        case 4:
            printf("👋 Exiting...\n");
            exit(0);

        default:
            printf("Invalid choice!\n");
        }
    }
}