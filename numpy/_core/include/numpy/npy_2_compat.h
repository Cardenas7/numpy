/*
 * This header file defines relevant features which:
 * - Require runtime inspection depending on the NumPy version.
 * - May be needed when compiling with an older version of NumPy to allow
 *   a smooth transition.
 *
 * As such, it is shipped with NumPy 2.0, but designed to be vendored in full
 * or parts by downstream projects.
 *
 * It must be included after any other includes.  `import_array()` must have
 * been called in the scope or version dependency will misbehave, even when
 * only `PyUFunc_` API is used.
 *
 * If required complicated defs (with inline functions) should be written as:
 *
 *     #if NPY_FEATURE_VERSION >= NPY_2_0_API_VERSION
 *         Simple definition when NumPy 2.0 API is guaranteed.
 *     #else
 *         static inline definition of a 1.x compatibility shim
 *         #if NPY_ABI_VERSION < 0x02000000
 *            Make 1.x compatibility shim the public API (1.x only branch)
 *         #else
 *             Runtime dispatched version (1.x or 2.x)
 *         #endif
 *     #endif
 *
 * An internal build always passes NPY_FEATURE_VERSION >= NPY_2_0_API_VERSION
 */

#ifndef NUMPY_CORE_INCLUDE_NUMPY_NPY_2_COMPAT_H_
#define NUMPY_CORE_INCLUDE_NUMPY_NPY_2_COMPAT_H_

/*
 * New macros for accessing real and complex part of a complex number can be
 * found in "npy_2_complexcompat.h".
 */


/*
 * This header is meant to be included by downstream directly for 1.x compat.
 * In that case we need to ensure that users first included the full headers
 * and not just `ndarraytypes.h`.
 */
#ifndef NPY_FEATURE_VERSION
  #error "The NumPy 2 compat header requires `import_array()` for which "  \
         "the `ndarraytypes.h` header include is not sufficient.  Please "  \
         "include it after `numpy/ndarrayobject.h` or similar.\n"  \
         "To simplify includsion, you may use `PyArray_ImportNumPy()` " \
         "which is defined in the compat header and is lightweight (can be)."
#endif

#if NPY_ABI_VERSION < 0x02000000
  /*
   * Define 2.0 feature version as it is needed below to decide whether we
   * compile for both 1.x and 2.x (defining it gaurantees 1.x only).
   */
  #define NPY_2_0_API_VERSION 0x00000012
  /*
   * If we are compiling with NumPy 1.x, PyArray_RUNTIME_VERSION so we
   * pretend the `PyArray_RUNTIME_VERSION` is `NPY_FEATURE_VERSION`.
   * This allows downstream to use `PyArray_RUNTIME_VERSION` if they need to.
   */
  #define PyArray_RUNTIME_VERSION NPY_FEATURE_VERSION
#endif


/*
 * Define a better way to call `_import_array()` to simplify backporting as
 * we now require imports more often (necessary to make ABI flexible).
 */
#ifdef import_array1

static inline int
PyArray_ImportNumPyAPI()
{
    if (NPY_UNLIKELY(PyArray_API == NULL)) {
        import_array1(-1);
    }
    return 0;
}

#endif  /* import_array1 */


/*
 * NPY_DEFAULT_INT
 *
 * The default integer has changed, `NPY_DEFAULT_INT` is available at runtime
 * for use as type number, e.g. `PyArray_DescrFromType(NPY_DEFAULT_INT)`.
 *
 * NPY_RAVEL_AXIS
 *
 * This was introduced in NumPy 2.0 to allow indicating that an axis should be
 * raveled in an operation. Before NumPy 2.0, NPY_MAXDIMS was used for this purpose.
 *
 * NPY_MAXDIMS
 *
 * A constant indicating the maximum number dimensions allowed when creating
 * an ndarray.
 *
 * NPY_NTYPES_LEGACY
 *
 * The number of built-in NumPy dtypes.
 */
#if NPY_FEATURE_VERSION >= NPY_2_0_API_VERSION
    #define NPY_DEFAULT_INT NPY_INTP
    #define NPY_RAVEL_AXIS NPY_MIN_INT
    #define NPY_MAXARGS 64

    static inline npy_uint64
    PyDataType_FLAGS(const PyArray_Descr *dtype)
    {
        return (unsigned char)dtype->flags;
    }
#elif NPY_ABI_VERSION < 0x02000000
    #define NPY_DEFAULT_INT NPY_LONG
    #define NPY_RAVEL_AXIS 32
    #define NPY_MAXARGS 32

    static inline npy_uint64
    PyDataType_FLAGS(const PyArray_Descr *dtype)
    {
        return (unsigned char)dtype->flags;
    }

    /* Aliases of 2.x names to 1.x only equivalent names */
    #define NPY_NTYPES NPY_NTYPES_LEGACY
    #define PyArray_DescrProto PyArray_Descr
#else
    #define NPY_DEFAULT_INT  \
        (PyArray_RUNTIME_VERSION >= NPY_2_0_API_VERSION ? NPY_INTP : NPY_LONG)
    #define NPY_RAVEL_AXIS  \
        (PyArray_RUNTIME_VERSION >= NPY_2_0_API_VERSION ? -1 : 32)
    #define NPY_MAXARGS  \
        (PyArray_RUNTIME_VERSION >= NPY_2_0_API_VERSION ? 64 : 32)

    static inline npy_uint64
    PyDataType_FLAGS(const PyArray_Descr *dtype)
    {
        if (PyArray_RUNTIME_VERSION >= NPY_2_0_API_VERSION) {
            // TODO: This will change to a semi-private 2.0 struct name
            return (unsigned char)((PyArray_Descr *)dtype)->flags;
        }
        else {
            return (unsigned char)((PyArray_DescrProto *)dtype)->flags;
        }
    }
#endif


#endif  /* NUMPY_CORE_INCLUDE_NUMPY_NPY_2_COMPAT_H_ */
