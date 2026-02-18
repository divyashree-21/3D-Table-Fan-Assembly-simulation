// ...existing code...
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 20

// Structures
struct Queue { char parts[MAX][64]; int front, rear; };
struct Stack { char parts[MAX][64]; int top; };

// Queue
void initQueue(struct Queue *q){ q->front = -1; q->rear = -1; }
int isEmptyQ(struct Queue *q){ return q->front == -1; }
int isFullQ(struct Queue *q){ return q->rear == MAX-1; }
int enqueue(struct Queue *q, const char *part){
    if(isFullQ(q)) return 0;
    if(q->front == -1) q->front = 0;
    strncpy(q->parts[++q->rear], part, sizeof(q->parts[0])-1);
    q->parts[q->rear][sizeof(q->parts[0])-1] = '\0';
    return 1;
}
int dequeue(struct Queue *q, char *out){
    if(isEmptyQ(q)) return 0;
    strcpy(out, q->parts[q->front]);
    if(q->front == q->rear) q->front = q->rear = -1;
    else q->front++;
    return 1;
}

// Stack
void initStack(struct Stack *s){ s->top = -1; }
int isEmptyS(struct Stack *s){ return s->top == -1; }
int isFullS(struct Stack *s){ return s->top == MAX-1; }
int push(struct Stack *s, const char *part){
    if(isFullS(s)) return 0;
    strncpy(s->parts[++s->top], part, sizeof(s->parts[0])-1);
    s->parts[s->top][sizeof(s->parts[0])-1] = '\0';
    return 1;
}
int pop(struct Stack *s, char *out){
    if(isEmptyS(s)) return 0;
    strcpy(out, s->parts[s->top--]);
    return 1;
}

// Simulate fan
void simulateFanRun(){
    printf("\n🌀 Fan is running...\n");
    for(int i=0;i<3;i++) printf("  🔄 Speed Level %d...\n", i+1);
    printf("🛑 Fan stopped.\n");
}

int main(void){
    struct Queue assembly;
    struct Stack disassembly;
    initQueue(&assembly);
    initStack(&disassembly);

    // SQL output file (no DB client needed)
    const char *sql_path = "c:\\Users\\Divya gowda\\OneDrive\\Documents\\project.c\\tablefan.sql";
    FILE *sql = fopen(sql_path, "w");
    if(!sql){
        fprintf(stderr, "Error: cannot open SQL output file: %s\n", sql_path);
        return 1;
    }

    // Create DB schema statements
    fprintf(sql, "-- Generated SQL for table_fan_db\n");
    fprintf(sql, "CREATE DATABASE IF NOT EXISTS table_fan_db;\nUSE table_fan_db;\n\n");
    fprintf(sql, "CREATE TABLE IF NOT EXISTS parts (\n  id INT AUTO_INCREMENT PRIMARY KEY,\n  name VARCHAR(100),\n  assembly_order INT,\n  disassembly_order INT\n);\n\n");
    fprintf(sql, "CREATE TABLE IF NOT EXISTS assembly_summary (\n  id INT AUTO_INCREMENT PRIMARY KEY,\n  joined_parts TEXT\n);\n\n");

    printf("=== TABLE FAN (assembly -> run -> disassembly) ===\n");

    // Enqueue parts (assembly) and write INSERTs
    const char *parts[] = {"Base", "Stand", "Motor", "Blades", "Front Cover"};
    int nparts = sizeof(parts)/sizeof(parts[0]);
    for(int i=0;i<nparts;i++){
        enqueue(&assembly, parts[i]);
        // escape single quotes if present (basic)
        char esc[128]; int p=0;
        for(const char *s=parts[i]; *s && p < (int)sizeof(esc)-2; s++){
            esc[p++] = (*s == '\'') ? '\'' : *s;
            if(*s == '\'') esc[p++] = '\'';
        }
        esc[p] = '\0';
        fprintf(sql, "INSERT INTO parts (name, assembly_order) VALUES ('%s', %d);\n", esc, i+1);
        printf("🧩 Part assembled: %s (order %d)\n", parts[i], i+1);
    }

    // Write joined assembly_summary
    char joined[512] = "";
    for(int i=0;i<nparts;i++){
        if(i) strcat(joined, ", ");
        strcat(joined, parts[i]);
    }
    // escape single quotes in joined if any
    char joined_esc[512]; int j=0;
    for(char *s = joined; *s && j < (int)sizeof(joined_esc)-2; s++){
        if(*s == '\'') { joined_esc[j++] = '\''; joined_esc[j++] = '\''; }
        else joined_esc[j++] = *s;
    }
    joined_esc[j] = '\0';
    fprintf(sql, "INSERT INTO assembly_summary (joined_parts) VALUES ('%s');\n\n", joined_esc);

    printf("\n✅ Assembly complete. Joined: %s\n", joined);

    // Simulate fan
    simulateFanRun();

    // Transfer queue -> stack (prepares LIFO disassembly)
    char temp[64];
    while(dequeue(&assembly, temp)){
        push(&disassembly, temp);
    }

    // Pop stack to disassemble, write UPDATEs for disassembly_order
    printf("\n🔧 Disassembly steps:\n");
    int dorder = 1;
    while(pop(&disassembly, temp)){
        printf("  🧭 Removed: %s (step %d)\n", temp, dorder);
        // escape simple single quotes
        char esc2[128]; int k=0;
        for(char *s=temp; *s && k < (int)sizeof(esc2)-2; s++){
            if(*s == '\'') { esc2[k++] = '\''; esc2[k++] = '\''; }
            else esc2[k++] = *s;
        }
        esc2[k] = '\0';
        fprintf(sql, "UPDATE parts SET disassembly_order = %d WHERE name = '%s' AND disassembly_order IS NULL;\n", dorder, esc2);
        dorder++;
    }

    fprintf(sql, "\n-- End of generated SQL\n");
    fclose(sql);

    printf("\n✅ Disassembly complete.\nSQL script saved to:\n  %s\n", sql_path);
    printf("Import into MariaDB: mysql -u root -p table_fan_db < \"%s\"\n", sql_path);
    return 0;
}
// ...existing code...