# tinyobjloader

Vendored single-header [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) (`tiny_obj_loader.h`, MIT).

Used only by **rigObj**. Do not include from host `src/`.

## Local modifications

Tracked in the RigKit fork: https://github.com/RigKit/tinyobjloader

### `constexpr` on the `tinyobj_ff` helpers

**Symptom** - GCC warns (hard-errors before GCC 13) inside the embedded fast_float:

```
tiny_obj_loader.h:3100:31: warning: call to non-'constexpr' function
  'tinyobj_ff::distance(It, It) [with It = const char*]' [-Winvalid-constexpr]
```

**Cause** - `tinyobj_ff` supplies freestanding stand-ins for `<algorithm>`/`<iterator>`
so the header needs no libstdc++ algorithms. fast_float calls them from functions
marked `FASTFLOAT_CONSTEXPR14` / `FASTFLOAT_CONSTEXPR20`, and the `std::` originals
it was written against *are* `constexpr`. Ours were not, which makes those callers
`constexpr` functions that can never be constant-evaluated for any arguments -
ill-formed NDR, which GCC diagnoses.

**Fix** - mark the affected helpers `constexpr`:

| Helper | Marking | Why |
| --- | --- | --- |
| `min_val`, `distance` | `constexpr` | single-return bodies, valid since C++11 |
| `copy_n`, `copy_backward`, `fill` | `TINYOBJ_FF_CONSTEXPR14` | loop bodies need relaxed constexpr ([N3652](https://wg21.link/N3652)) |

`TINYOBJ_FF_CONSTEXPR14` expands to `constexpr` when `__cpp_constexpr >= 201304`
and to nothing otherwise, mirroring fast_float's own `FASTFLOAT_CONSTEXPR14`.
The header's C++11 floor is preserved.

Single hunk, around the helper definitions in `namespace tinyobj_ff`. Verified
warning-free on GCC 11 with `-Wall -Wextra` under `-std=c++11/14/17/20/23`, with
`static_assert`s confirming the helpers are now genuinely constant-evaluable.

**Upstream status** - PR pending. Drop this patch once it lands.

### Notes for the next sync

Re-apply the above after pulling upstream. Two things make this diff easy to lose:
the block sits in `namespace tinyobj_ff` (RigKit-specific, several thousand lines
above the fast_float section that consumes it), and `-Winvalid-constexpr` is *off
by default* under `-std=c++23` (P2448 relaxed the rule), so a C++23 build will not
tell you the patch went missing.
