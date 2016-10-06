#include <Python.h>
#include "node.h"

#include "types.h"
#include "task.h"
#include "utarray.h"
#include "utstring.h"

PyObject *CommonError;

#define MARSHAL_VERSION 2

static int PyTask_init(PyTask *self, PyObject *args, PyObject *kwds) {
  function_id function_id;
  /* Arguments of the task (can be PyObjectIDs or Python values). */
  PyObject *arguments;
  /* Array of pointers to string representations of pass-by-value args. */
  UT_array *val_repr_ptrs;
  utarray_new(val_repr_ptrs, &ut_ptr_icd);
  int num_returns;
  if (!PyArg_ParseTuple(args, "O&Oi", &PyObjectToUniqueID, &function_id,
                        &arguments, &num_returns)) {
    return -1;
  }
  size_t size = PyList_Size(arguments);
  /* Determine the size of pass by value data in bytes. */
  size_t value_data_bytes = 0;
  for (size_t i = 0; i < size; ++i) {
    PyObject *arg = PyList_GetItem(arguments, i);
    if (!PyObject_IsInstance(arg, (PyObject *) &PyObjectIDType)) {
      PyObject *data = PyMarshal_WriteObjectToString(arg, MARSHAL_VERSION);
      value_data_bytes += PyString_Size(data);
      utarray_push_back(val_repr_ptrs, &data);
    }
  }
  /* Construct the task specification. */
  int val_repr_index = 0;
  self->spec =
      alloc_task_spec(function_id, size, num_returns, value_data_bytes);
  for (size_t i = 0; i < size; ++i) {
    PyObject *arg = PyList_GetItem(arguments, i);
    if (PyObject_IsInstance(arg, (PyObject *) &PyObjectIDType)) {
      task_args_add_ref(self->spec, ((PyObjectID *) arg)->object_id);
    } else {
      PyObject *data =
          *((PyObject **) utarray_eltptr(val_repr_ptrs, val_repr_index));
      task_args_add_val(self->spec, (uint8_t *) PyString_AS_STRING(data),
                        PyString_GET_SIZE(data));
      Py_DECREF(data);
      val_repr_index += 1;
    }
  }
  utarray_free(val_repr_ptrs);
  return 0;
}

static void PyTask_dealloc(PyTask *self) {
  free_task_spec(self->spec);
  Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *PyTask_function_id(PyObject *self) {
  function_id function_id = *task_function(((PyTask *) self)->spec);
  return PyObjectID_make(function_id);
}

static PyObject *PyTask_arguments(PyObject *self) {
  int64_t num_args = task_num_args(((PyTask *) self)->spec);
  PyObject *arg_list = PyList_New((Py_ssize_t) num_args);
  task_spec *task = ((PyTask *) self)->spec;
  for (int i = 0; i < num_args; ++i) {
    if (task_arg_type(task, i) == ARG_BY_REF) {
      object_id object_id = *task_arg_id(task, i);
      PyList_SetItem(arg_list, i, PyObjectID_make(object_id));
    } else {
      PyObject *s =
          PyMarshal_ReadObjectFromString((char *) task_arg_val(task, i),
                                         (Py_ssize_t) task_arg_length(task, i));
      PyList_SetItem(arg_list, i, s);
    }
  }
  return arg_list;
}

static PyObject *PyTask_returns(PyObject *self) {
  int64_t num_returns = task_num_returns(((PyTask *) self)->spec);
  PyObject *return_id_list = PyList_New((Py_ssize_t) num_returns);
  task_spec *task = ((PyTask *) self)->spec;
  for (int i = 0; i < num_returns; ++i) {
    object_id object_id = *task_return(task, i);
    PyList_SetItem(return_id_list, i, PyObjectID_make(object_id));
  }
  return return_id_list;
}

static PyMethodDef PyTask_methods[] = {
    {"function_id", (PyCFunction) PyTask_function_id, METH_NOARGS,
     "Return the function ID for this task."},
    {"arguments", (PyCFunction) PyTask_arguments, METH_NOARGS,
     "Return the arguments for the task."},
    {"returns", (PyCFunction) PyTask_returns, METH_NOARGS,
     "Return the object IDs for the return values of the task."},
    {NULL} /* Sentinel */
};

static PyTypeObject PyTaskType = {
    PyObject_HEAD_INIT(NULL) 0,  /* ob_size */
    "task.Task",                 /* tp_name */
    sizeof(PyTask),              /* tp_basicsize */
    0,                           /* tp_itemsize */
    (destructor) PyTask_dealloc, /* tp_dealloc */
    0,                           /* tp_print */
    0,                           /* tp_getattr */
    0,                           /* tp_setattr */
    0,                           /* tp_compare */
    0,                           /* tp_repr */
    0,                           /* tp_as_number */
    0,                           /* tp_as_sequence */
    0,                           /* tp_as_mapping */
    0,                           /* tp_hash */
    0,                           /* tp_call */
    0,                           /* tp_str */
    0,                           /* tp_getattro */
    0,                           /* tp_setattro */
    0,                           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,          /* tp_flags */
    "Task object",               /* tp_doc */
    0,                           /* tp_traverse */
    0,                           /* tp_clear */
    0,                           /* tp_richcompare */
    0,                           /* tp_weaklistoffset */
    0,                           /* tp_iter */
    0,                           /* tp_iternext */
    PyTask_methods,              /* tp_methods */
    0,                           /* tp_members */
    0,                           /* tp_getset */
    0,                           /* tp_base */
    0,                           /* tp_dict */
    0,                           /* tp_descr_get */
    0,                           /* tp_descr_set */
    0,                           /* tp_dictoffset */
    (initproc) PyTask_init,      /* tp_init */
    0,                           /* tp_alloc */
    PyType_GenericNew,           /* tp_new */
};

/* Create a PyTask from a C struct. The resulting PyTask takes ownership of the
 * task_spec and will deallocate the task_spec in the PyTask destructor. */
PyObject *PyTask_make(task_spec *task_spec) {
  PyTask *result = PyObject_New(PyTask, &PyTaskType);
  result = (PyTask *) PyObject_Init((PyObject *) result, &PyTaskType);
  result->spec = task_spec;
  return (PyObject *) result;
}

static PyMethodDef common_methods[] = {
    {"check_simple_value", check_simple_value, METH_VARARGS,
     "Should the object be passed by value?"},
    {NULL} /* Sentinel */
};

#ifndef PyMODINIT_FUNC /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

PyMODINIT_FUNC initcommon(void) {
  PyObject *m;

  if (PyType_Ready(&PyTaskType) < 0)
    return;

  if (PyType_Ready(&PyObjectIDType) < 0)
    return;

  m = Py_InitModule3("common", common_methods,
                     "Example module that creates an extension type.");

  Py_INCREF(&PyTaskType);
  PyModule_AddObject(m, "Task", (PyObject *) &PyTaskType);

  Py_INCREF(&PyObjectIDType);
  PyModule_AddObject(m, "ObjectID", (PyObject *) &PyObjectIDType);

  char common_error[] = "common.error";
  CommonError = PyErr_NewException(common_error, NULL, NULL);
  Py_INCREF(CommonError);
  PyModule_AddObject(m, "common_error", CommonError);
}
