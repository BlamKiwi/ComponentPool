# Cache Coherent Component Pool

A C++11 implementation of cache coherent component pools done as a professional learning exercise. Works by moving components at runtime in an object pool to ensure that the update loop is as tight and cache friendly as possible. Provides a smart pointer implementation to simplify component access for users of the component system.

Dirty details can be found here: http://www.missingbox.co.nz/making-cache-coherency-easy/

Implementation is incomplete with only partial error handling and method qualifiers. I strongly recommend against using this code in production systems. Code has been tested with ICC v15 on Windows.

## License

The source code is licensed using the BSD 2-Clause license. A copy of the license can be found in the LICENSE file and in the source code. 