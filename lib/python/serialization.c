#include "types.h"

/* TODO(pcm): Add limit on total number of elements. */

#define SIZE_LIMIT 100

int is_simple_value(PyObject *value) {
  if (PyInt_Check(value) || PyLong_Check(value) || value == Py_False ||
      value == Py_True || PyFloat_Check(value) || value == Py_None) {
    return 1;
  }
  if (PyList_Check(value) && PyList_Size(value) < SIZE_LIMIT) {
    for (size_t i = 0; i < PyList_Size(value); ++i) {
      if (!is_simple_value(PyList_GetItem(value, i))) {
        return 0;
      }
    }
  }
  if (PyDict_Check(value) && PyDict_Size(value) < SIZE_LIMIT) {
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    while (PyDict_Next(index, &pos, &key, &value)) {
      if (!is_simple_value(key) || !is_simple_value(value)) {
        return 0;
      }
    }
  }
  if (PyTuple_Check(value) && PyTuple_Size(value) < SIZE_LIMIT) {
    for (size_t i = 0; i < PyTuple_Size(value); ++i) {
      if (!is_simple_value(PyTuple_GetItem(value, i))) {
        return 0;
      }
    }
  }
  return 1;
}

PyObject *check_simple_value(PyObject *self, PyObject *args) {
  PyObject *value;
  if (!PyArg_ParseTuple(args, "O", &value)) {
    return NULL;
  }
  if (is_simple_value(value)) {
    Py_RETURN_TRUE;
  }
  Py_RETURN_FALSE;
}
