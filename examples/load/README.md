# load

![preview](img/preview.png)

Loads the Stanford bunny from Wavefront OBJ (+ MTL diffuse to `CMesh::faceColors`), presents with rigRender3D.

Mesh: `data/bunny.obj` - [Stanford 3D Scanning Repository](https://graphics.stanford.edu/data/3Dscanrep/)

Mouse: LMB orbit, Shift+LMB or MMB pan, wheel or RMB dolly. F frames. Ctrl+R reloads `bunny.obj`. Host `GraphicsSettings::showFps` turns on the corner readout.

```bash
cmake -S . -B build
cmake --build build --target load
./build/bin/load/load
```
