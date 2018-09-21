# cppLab
C++ stuff

## Visual Studio Code
How to set up VSC.
* Install extension `CMake Tools Helper`. It will install `C/C++` and `CMake Tools` extensions.

### VSC with Visual Studio 15 2017
* `ctr + shift + p` + `Preference: Open Workspace Settings` and add those lines in (Workspace settings) settings.json:

```javascript
    {
        "cmake.generator": "Visual Studio 15 2017"
    }
```
* `ctr + shift + p` + `CMake: Select active build environments` and chose `Visual Studio Professional 2017 -amd64`.

* `C/Cpp: Edit Configurations`
