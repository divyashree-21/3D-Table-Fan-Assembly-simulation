#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// ...existing code...
// ...existing code...
#define DB_USER "root"
#define DB_PASS "0000"  // <-- replace "0000" with your real password (remove stray comment characters)
// ...existing code...   // MySQL header

#define MAX 10

// Database credentials
#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASS "0000"  // <-- replace "0000" with your real password (remove stray comment characters)
#define DB_NAME "table_fan_db"

// STRUCTURE — Queue (Assembly)
struct Queue {
    char parts[MAX][50];
    int front, rear;
};

// STRUCTURE — Stack (Disassembly)
struct Stack {
    char parts[MAX][50];
    int top;
};

// GLOBAL MySQL Variable
MYSQL *conn;

// ---------- DBMS FUNCTIONS ----------
void connectDB() {
    conn = mysql_init(NULL);
    if (conn == NULL) {
        printf("❌ MySQL initialization failed!\n");
        exit(1);
    }

    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, NULL, 0)) {
        printf("❌ Database Connection Failed: %s\n", mysql_error(conn));
        exit(1);
    }
    printf("✅ Connected to Database Successfully!\n");
}

void savePartToDB(char *partName) {
    char query[200];
    sprintf(query, "INSERT INTO parts (name) VALUES ('%s')", partName);

    if (mysql_query(conn, query)) {
        printf("❌ Failed to insert into DB: %s\n", mysql_error(conn));
    } else {
        printf("📌 Saved to DB: %s\n", partName);
    }
}

// ---------- QUEUE FUNCTIONS ----------
void initQueue(struct Queue *q) {
    q->front = -1;
    q->rear = -1;
}

int isFullQ(struct Queue *q) {
    return q->rear == MAX - 1;
}

int isEmptyQ(struct Queue *q) {
    return q->front == -1;
}

void enqueue(struct Queue *q, char *part) {
    if (isFullQ(q)) {
        printf("\nQueue Full! Cannot add more parts.\n");
        return;
    }
    if (q->front == -1)
        q->front = 0;

    strcpy(q->parts[++q->rear], part);
    printf("🧩 Part added to Assembly Queue: %s\n", part);

    // SAVE IN DATABASE
    savePartToDB(part);
}

// ---------- STACK FUNCTIONS ----------
void initStack(struct Stack *s) {
    s->top = -1;
}

int isFullS(struct Stack *s) {
    return s->top == MAX - 1;
}

int isEmptyS(struct Stack *s) {
    return s->top == -1;
}

void push(struct Stack *s, char *part) {
    if (isFullS(s)) {
        printf("\nStack is Full. Cannot push.\n");
        return;
    }
    strcpy(s->parts[++s->top], part);
}

// ---------- FAN SIMULATION ----------
void simulateFanRun() {
    printf("\n🌀 Fan is running...\n");
    for (int i = 0; i < 3; i++) {
        printf("  🔄 Spinning at Speed Level %d...\n", i + 1);
    }
    printf("🛑 Fan stopped successfully.\n");
}

// ---------- MAIN ----------
int main() {

    // DB Connect
    connectDB();

    struct Queue assembly;
    struct Stack disassembly;

    initQueue(&assembly);
    initStack(&disassembly);

    printf("\n===== TABLE FAN PROJECT (DSA + DBMS) =====\n");

    // Assembly using Queue
    enqueue(&assembly, "Base");
    enqueue(&assembly, "Stand");
    enqueue(&assembly, "Motor");
    enqueue(&assembly, "Blades");
    enqueue(&assembly, "Front Cover");

    printf("\n✅ All parts assembled successfully!\n");

    // Run Simulation
    simulateFanRun();

    // Disassembly using Stack
    printf("\n🔧 Starting Disassembly...\n");
    while (!isEmptyQ(&assembly)) {
        strcpy(disassembly.parts[++disassembly.top], assembly.parts[assembly.rear]);
        printf("❌ Removed Part: %s\n", disassembly.parts[disassembly.top]);

        assembly.rear--;
        if (assembly.rear < assembly.front)
            assembly.front = -1;
    }

    printf("\n✅ Disassembly Complete.\n");

    // Close DB
    mysql_close(conn);

    return 0;
}
