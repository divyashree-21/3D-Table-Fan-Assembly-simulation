# Table Fan 3D Simulation

## Overview
This project simulates a table fan in 3D, allowing users to visualize the assembly and disassembly of its parts, as well as the operation of the fan itself. The application utilizes OpenGL for rendering the 3D model and GLSL for shader programming.

## Project Structure
```
tablefan-3d
├── src
│   ├── main.c            # Entry point of the application
│   ├── tablefan.c        # Implementation of the table fan simulation
│   ├── renderer.c        # Handles rendering of the 3D model
│   ├── renderer.h        # Function declarations for rendering
│   ├── math_utils.c      # Mathematical utility functions
│   └── math_utils.h      # Declarations for math utilities
├── include
│   └── tablefan.h        # Declarations for table fan simulation functions
├── shaders
│   ├── vertex.glsl       # Vertex shader code
│   └── fragment.glsl     # Fragment shader code
├── models
│   └── tablefan.obj      # 3D model of the table fan
├── CMakeLists.txt        # Build configuration for CMake
├── Makefile              # Build rules for make
├── .gitignore            # Files to ignore in Git
└── README.md             # Project documentation
```

## Setup Instructions
1. **Clone the repository**:
   ```
   git clone <repository-url>
   cd tablefan-3d
   ```

2. **Build the project**:
   - Using CMake:
     ```
     mkdir build
     cd build
     cmake ..
     make
     ```
   - Or using Makefile:
     ```
     make
     ```

3. **Run the application**:
   ```
   ./tablefan-3d
   ```

## Usage
- The application will display a 3D model of the table fan.
- Users can interact with the model to assemble and disassemble parts.
- The fan's operation can be simulated, showcasing different speed levels.

## Dependencies
- OpenGL
- GLSL
- CMake (for building)
- Make (optional, for building)

## Contributing
Contributions are welcome! Please submit a pull request or open an issue for any suggestions or improvements.

## License
This project is licensed under the MIT License. See the LICENSE file for details.