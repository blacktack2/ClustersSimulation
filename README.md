# ClustersSimulation

Based on Jeffrey Ventrella's Clusters simulation (see:
https://www.ventrella.com/Clusters/)

Inspired by this YouTube video on the topic (see:
https://www.youtube.com/watch?v=0Kx4Y9TVMGg)

## Installation

### Dependencies

This project makes use of the following dependencies:

- SDL2 - (https://www.libsdl.org/)
- Dear ImGui - (included in project) (https://github.com/ocornut/imgui)
- OpenGL (glad) - (download | https://glad.dav1d.de/) (OpenGL docs |
https://www.khronos.org/)

### SDL2

#### Linux

Follow the instructions at
(https://lazyfoo.net/tutorials/SDL/01_hello_SDL/linux/index.php)
to download SDL2. If you download using `apt-get` SDL2 should be placed in the
expected directory by default. If you download manually you will have to ensure
SDL2 is located somewhere CMake can find it, otherwise you may want to edit the
`CMakeLists.txt` file to look in the correct directory.

#### Windows

Follow the instructions at
(https://lazyfoo.net/tutorials/SDL/01_hello_SDL/linux/index.php)
to download SDL2. CMake expects to find SDL2 in the `c:/programs/sdl/`
directory.

### OpenGL (glad)

glad can be downloaded from (https://glad.dav1d.de/) and should be saved to
(`~/programs/glad/` on Linux, `c:/programs/glad/` on Windows), otherwise you
can edit `CMakeLists.txt` to search the directory you placed it in. The
language should beset to C/C++, the specification set to OpenGL, and the
profile set to Compatibility. The API should have gl set to version 4.6+,
and everything else set to None. To be safe you should enable all of the
extensions that appear, though it should work without (not tested!).

## Usage

Compiling using cmake should create two executables (ClustersSimulation
and ClustersSimulation_GPU). The GPU version uses Compute Shaders to execute
and render the simulation faster (the exact improvement depends on your
hardware but I've seen an improvement of ~10x).

The application will expect to find the `resources` directory and the `shaders`
directory in the same working directory as the executable. Some features may
not work correctly if the `resources` directory is missing (but the application
should still run) and the application will break if the `shaders` directory is
missing.

The application also expects to find the `SimConfigs` directory in its working
directory (but will create it if not found). If you are missing any config files
make sure you didn't accidentally move/delete this directory, or move the
executable to a separate directory without this.

### Running

The program should start with a default simulation layed out for you with
6 atom types and a total of 1200 atoms. You can run the simulation by either
pressing **space bar** or the **Play**/**Pause** button in the parameters panel.

### General Parameters

At the top of the parameters panel you will find buttons to apply general
operations to all atom types as well as numeric inputs for the simulation.

- **Play**/**Pause** - Start/stop the simulation (can also be done using the
**space bar**)
- **Single Iteration** - Perform a single iteration on the simulation
- **Re-generate Atoms** - Re-initialize the simulation with new atoms and
randomize their positions. You will have to press this if you change the
number of atoms in the simulation, or if you add an atom type, in order to
update the simulation (this may be changed).
- **Clear Atoms** - Remove all atoms from the simulation
- **Clear Atom Types** - Remove all atom types and atoms from the simulation
- **Randomize Positions** - Randomly set the positions of *all* loaded atoms
- **Zero Interactions** - Set *all* atom type interactions to 0
- **Shuffle Interactions** - Set *all* atom type interactions to a random value

---

- **Simulation Scale** - How large the simulation is (both directions)
- **Time Delta (dt)** - Lower values will make the simulation more technically
accurate, but may result in the simulation appearing to slow down
- **Drag Force** - How fast the atoms will lose velocity over time (0 will
stop them entirely, 1 will be no drag force)
- **Max Interaction Range** - How far (radius) the atoms must be from each other
in order to interact (same units as **Simulation Scale**)
- **Collision Force** - How strongly atoms will repel from each other when
overlapping (0 will result in no collisions)

### Atom Parameters

- **Name** - What the atom will be called (shown above each interaction)
- **Colour** - The colour the atom will be displayed as
- **Quantity** - How many atoms of this type should be created
- ***a*->*b*** - How strongly does atom ***a*** attract (negative) or repel
(positive) atom ***b*** (assymetric: ***a*->*b*** != ***b*->*a***). Setting
to 0 will result in ***a*** ignoring ***b*** (except for collisions, if
enabled)
    + **Button** - Set interaction to 0
    + **Slider** - Set interaction to value between -1 and 1

### Limits

I am yet to fully implement hard-coded limits so be careful of simulating with
more than 1500 atoms on the CPU version, and more than 5000 on the GPU version.
The CPU version currently also has poorly implemented atom types, so avoid
using much more than 8.

No proper limits are not yet implemented on the CPU version. Be careful using
large values on the CPU version (the application may hang).

| Entity | Min | Max | Notes |
| ------ |:---:|:-----:| ----- |
| Atoms (total) | 0 | 10000 | CPU version should not go above 2000<br/>Not hard implemented in CPU version |
| Atom Types | 0 | 50 | CPU version should not go above 8 |
| Interactions | 0 | 2500 | Hard defined as the square of the atom types |
| Scale | 1.0 | 1000000.0 | Small values may not render correctly |
| dt | 0.01 | 10.00 | Small values may be imperceptibly slow |
| Drag | 0.0 | 1.0 |  |
| I-Range | 1.0 | **Scaled** | Max scales with **Simulation Scale** parameter |
| Collision | 0.00 | 10.00 |  |

## License

MIT License

Copyright (c) 2022 Stuart Manfred Lewis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## TODO

### Additions

- Improve parameters UI (make cleaner and more intuitive, and add colours to
help distinguish buttons)
- Add VSync toggle to Debug panel (should be saved across executions)
- Make simulation scale able to handle different width and height (and correctly
display)
- Possibly try to improve speed of simulation's Compute Shader (was expecting
it to perform significantly faster)
- Add atom size parameter
- Atom type name parameter should be set to it's default if empty
- Remove trivial getters/setters in SimulationHandler
- Add proper error checking and edge-case handling for file reading/writing
(should allow for edge cases such as files/directories being deleted at runtime)
- See if shader files can be included in the executable itself, rather than
copied into a local directory
    + Also implement error checking to make sure the program correctly exits
in their absence
- Create mesh class for handling and drawing vertices (instead of embedding
this functionality in the Shader class)
- Improve handling of uniforms to avoid constantly looking up the same name,
and prevent assigning values to programs which don't need them.
- Change quad to use a triangle strip/fan instead of two separate triangles
- Replace header definition checks with `#pragma once`
- Better organise log outputs
    + Implement printing to a log file
- Use typedefs to improve type clarity

### Fixes

- **Max Interaction Range** parameter should have a maximum of the distance
from simulation center to corner (currently set from center to edge)
- Add hard coded limits to number of atoms and atom types to the UI (and a
warning if the number of atoms exceeds the limit)
- Remove unwanted padding on the simulation display
- GPU version does not instantiate default values of new atom types correctly
- Fix interaction sliders to fill the remaining panel width
- CPU version uses incredibly slow method of querying atom types
- Remove Herobrine?

### 'Maybe' additions

- Use threading to separate simulation execution from window handling (to
prevent a low fps from hanging the application)
- Include different shapes/textures to render different atom types with
- Add a render mode which renders the atoms as blobs which 'blend' together
when close
- Add sound effects to buttons
- Implement compatibility with MacOS
