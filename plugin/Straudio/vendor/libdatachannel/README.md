These static libs are not trivial to generate. The easier way to do this is as follows:

- download + install vcpkg if you haven't already
- copy arm64-osx and x64-osx triplets into the community folder
- rename to arm64-osx-11 and x64-osx-11
- add the following code to each triplet:

```
set(VCPKG_C_FLAGS -mmacosx-version-min=11.0)
set(VCPKG_CXX_FLAGS -mmacosx-version-min=11.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
```

- `vcpkg install libdatachannel:arm64-osx-11`
- `vcpkg install libdatachannel:x64-osx-11`
- `cd installed`
- `mkdir lipo`
- `lipo -create -output lipo/libcrypto.a arm64-osx-11/lib/libcrypto.a x64-osx-11/lib/libcrypto.a`
- Repeat for every library
- Copy header files and libs to respective dirs in Straudio

Creating those additional triplets is required because vcpkg will by default build targeting the current platform. iPlug2 lists 11.0 as its target.

We need to use lipo to combine the x64 and arm64 architectures because iPlug2 supports both architectures out-of-the-box (and we just should).