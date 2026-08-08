# rigObj

![preview](examples/load/img/preview.png)

Wavefront **OBJ** load/save for [rigComponent](https://github.com/rigkid/rigComponent) `CMesh`, via [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader). Code / IO only — no systems.

## API

```cpp
#include "ObjIO.h"

rigkit::ecs::CMesh mesh;
auto r = rig::obj::load("model.obj", mesh);
auto e = rig::obj::makeMeshFromFile(ecs, "model.obj", rig::fill(1,1,1), "thing");
rig::obj::save("out.obj", mesh);
```

Triangulates on load. Fills `texcoords` and material `faceColors` (MTL diffuse) when present.

## Example

[examples/load](examples/load/) — low-poly landing pad: `rocket.obj`, `pad.obj`, and `crate.obj` (with MTL) load into `CMesh`, then present under rigRender3D with an orbiting camera and banded light.

```bash
cmake -S examples/load -B examples/load/build
cmake --build examples/load/build --target load
```

[API/docs](https://rigkid.github.io/rigObj/)
