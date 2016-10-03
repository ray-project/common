#ifndef TYPES_H
#define TYPES_H

#include <Python.h>
#include "marshal.h"
#include "structmember.h"

#include "common.h"
#include "task.h"

extern PyObject *CommonError;

typedef struct {
  PyObject_HEAD
  object_id object_id;
} PyObjectID;

typedef struct {
  PyObject_HEAD
  task_spec *spec;
} PyTask;

extern PyTypeObject PyObjectIDType;

int PyObjectToUniqueID(PyObject* object, object_id *objectid);

PyObject* PyObjectID_make(object_id object_id);

#endif /* TYPES_H */
