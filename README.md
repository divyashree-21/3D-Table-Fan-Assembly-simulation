# 🌀 3D Table Fan Assembly Simulation

## 📌 Project Overview
This project simulates the assembly of a table fan using Data Structures and basic 3D visualization.

It demonstrates how different parts of a table fan are assembled step-by-step using programming concepts.

---

## 🎯 Objectives
- To understand real-world assembly using programming
- To implement Data Structures like Stack and Queue
- To visualize fan assembly in 3D using OpenGL

---

## 🧠 Concepts Used
- Queue (for assembly order)
- Stack (for undo operation)
- OpenGL (for 3D visualization)
- C Programming

---

## ⚙️ Features
- Step-by-step fan assembly
- Undo last assembled part
- Automatic animation of fan
- Keyboard and mouse controls
- Screenshot saving feature

---

## 🎮 Controls

- Mouse Drag → Rotate fan  
- Scroll → Zoom in/out  
- A → Auto assembly  
- 1–6 → Jump to steps  
- R → Reset  
- S → Save screenshot  
- ESC → Exit  

---

## 🛠️ Technologies Used
- C Language
- OpenGL
- GLFW
- GLU

---

## ▶️ How to Run

1. Open **MSYS2 MinGW64 terminal**
2. Navigate to project folder:

```bash
cd /c/Users/YourPath/3D-Table-Fan-Assembly-simulation
```

3. Compile:

```bash
gcc tablefan.c -o tablefan.exe -lglfw3 -lglu32 -lopengl32 -lgdi32 -lm
```

4. Run:

```bash
./tablefan.exe
```

---

## 📁 Project Structure

```
Tablefan_normal.c   → DSA logic
tablefan.c          → 3D simulation
table.html          → basic visualization
stb_image_write.h   → image saving
```

---

## 👩‍💻 Author

**Divya Shree**
B.E CSE Student

---

## 🌟 Conclusion

This project successfully combines Data Structures with real-time visualization, helping in understanding both programming and real-world system design.
