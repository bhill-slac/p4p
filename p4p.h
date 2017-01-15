#ifndef P4P_H
#define P4P_H

#include <sstream>
#include <algorithm>
#include <stdexcept>

#include <Python.h>

#include <pv/pvIntrospect.h>
#include <pv/pvData.h>

struct SB {
    std::ostringstream strm;
    operator std::string() { return strm.str(); }
    template<typename T>
    SB& operator<<(const T& v) {
        strm<<v;
        return *this;
    }
};

struct borrow {};
struct allownull {};
struct PyRef {
    PyObject *obj;
    PyRef() :obj(0) {}
    explicit PyRef(PyObject *o, const allownull&) :obj(o) {}
    explicit PyRef(PyObject *o, const borrow&) :obj(o) {
        if(!o)
            throw std::runtime_error("Can't borrow NULL");
        Py_INCREF(obj);
    }
    explicit PyRef(PyObject *o) :obj(o) {
        if(!o)
            throw std::runtime_error("Alloc failed");
    }
    ~PyRef() {
        Py_CLEAR(obj);
    }
    void reset(PyObject *o=0) {
        std::swap(obj, o);
        Py_XDECREF(o);
    }
    void reset(PyObject *o, const borrow&) {
        if(!o)
            throw std::runtime_error("Can't borrow NULL");
        Py_INCREF(o);
        std::swap(obj, o);
        Py_XDECREF(o);
    }
    PyObject *release() {
        PyObject *o=0;
        std::swap(obj, o);
        return o;
    }
    PyObject* get() const { return obj; }
};

struct PyString
{
    PyObject *base;
    PyRef temp;
    explicit PyString(PyObject *b) :base(b) {
        if(PyUnicode_Check(b)) {
            temp.reset(PyUnicode_AsUTF8String(b));
            if(!temp.get())
                throw std::runtime_error("PyString Unicode Error");
        } else if(!PyBytes_Check(b))
            throw std::runtime_error("Not bytes or unicode");
    }
    std::string str() {
        PyObject *X = temp.get() ? temp.get() : base;
        return std::string(PyBytes_AS_STRING(X),
                           PyBytes_GET_SIZE(X));
    }
};

#define CATCH() catch(std::exception& e) { if(!PyErr_Occurred()) { PyErr_SetString(PyExc_RuntimeError, e.what()); } }

#if 1
#define TRACE(ARG) do{ std::cerr<<"TRACE "<<__FUNCTION__<<" "<<ARG<<"\n";} while(0)
#else
#define TRACE(ARG) do{ } while(0)
#endif

#if PY_MAJOR_VERSION >= 3
# define MODINIT_RET(VAL) return (VAL)
#else
# define MODINIT_RET(VAL) do {(void)(VAL); return; }while(0)
#endif

#if PY_MAJOR_VERSION >= 3
#define PyMOD(NAME) PyMODINIT_FUNC PyInit_##NAME (void)
#else
#define PyMOD(NAME) PyMODINIT_FUNC init##NAME (void)
#endif

void p4p_type_register(PyObject *mod);
void p4p_value_register(PyObject *mod);


extern PyTypeObject P4PType_type;
// Extract Structure from P4PType
epics::pvData::Structure::const_shared_pointer P4PType_unwrap(PyObject *);
// Wrap Structure in P4PType
PyObject *P4PType_wrap(const epics::pvData::Structure::const_shared_pointer&);
// Find a Field capable of storing the provided value
epics::pvData::Field::const_shared_pointer P4PType_guess(PyObject *);


extern PyTypeObject P4PValue_type;
epics::pvData::PVStructure::shared_pointer P4PValue_unwrap(PyObject *);
PyObject *P4PValue_wrap(PyTypeObject *type, const epics::pvData::PVStructure::shared_pointer&);

#endif // P4P_H
