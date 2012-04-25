#include <Python.h>
#include <structmember.h>
#include "libsass/sass_interface.h"

PyDoc_STRVAR(sass_module_doc,
             "sass - document me\n");

typedef struct {
    PyObject_HEAD;
    struct sass_options options;
} py_sass_options;

static PyTypeObject py_sass_options_type;

static int
sass_options_init(py_sass_options *self, PyObject *args, PyObject *kwds) 
{
    int output_style = 0;
    char *include_paths = NULL;

    static char *kwlist[] = { "output_style", "include_paths", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|is:options",
                                     kwlist, &output_style, &include_paths))
        return -1;

	self->options.output_style = output_style;
    const size_t len = include_paths ? strlen(include_paths) : 0;
    if (len) { 
        self->options.include_paths = malloc(len + 1);
        if (!self->options.include_paths) {
            PyErr_NoMemory();
            return -1;
        }
        strcpy(self->options.include_paths, include_paths);
    }

	return 0;
}

static void
sass_options_dealloc(py_sass_options *obj)
{
    if (obj->options.include_paths)
        free(obj->options.include_paths);
    PyObject_Free(obj);
}

static PyObject *
sass_options_get_include_paths(py_sass_options *self, void *closure)
{
    if (!self->options.include_paths) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    return PyString_FromString(self->options.include_paths);
}

static int
sass_options_set_include_paths(py_sass_options *self, PyObject *value, 
                               void *closure)
{
    if (value == NULL) {
        if (self->options.include_paths) {
            free(self->options.include_paths);
        }
        self->options.include_paths = NULL;
        return 0;
    }
    
    if (!PyString_Check(value)) {
        PyErr_SetString(PyExc_TypeError, 
                        "The include_paths attribute value must be a string");
        return -1;
    }
    
    const Py_ssize_t len = PyString_GET_SIZE(value);
    if (len) { 
        self->options.include_paths = malloc(len + 1);
        if (!self->options.include_paths) {
            PyErr_NoMemory();
            return -1;
        }
        if (self->options.include_paths) 
            free(self->options.include_paths);

        strcpy(self->options.include_paths, PyString_AsString(value));
    }

    return 0;
}

static PyMemberDef sass_options_members[] = {
    {"output_style", T_INT, 
	 offsetof(py_sass_options, options.output_style), 0, ""},
    {NULL}  /* Sentinel */
};

static PyGetSetDef sass_options_getsetters[] = {
    {"include_paths", 
     (getter)sass_options_get_include_paths, 
     (setter)sass_options_set_include_paths,
     "include paths", NULL},
    {NULL}  /* Sentinel */
};

static PyTypeObject py_sass_options_type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,                              /*ob_size*/
	"sass.options",                 /*tp_name*/
	sizeof(py_sass_options_type),   /*tp_basicsize*/
	0,                              /*tp_itemsize*/
	/* methods */    
	(destructor)sass_options_dealloc, /*tp_dealloc*/
	0,                              /*print*/
    0,                              /*tp_getattr*/
	0,                              /*tp_setattr*/
	0,                              /*tp_compare*/ 
	0,                              /*tp_repr*/
	0,                              /*tp_as_number*/
	0,                              /*tp_as_sequence*/
	0,                              /*tp_as_mapping*/
	0,                              /*tp_hash*/
	0,                              /*tp_call*/
	0,                              /*tp_str*/
	PyObject_GenericGetAttr,        /*tp_getattro*/
	PyObject_GenericSetAttr,        /*tp_setattro*/
	0,                              /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  /*tp_flags*/
	"sass option flags and include path",  /*tp_doc*/
    0,		                        /* tp_traverse */
    0,		                        /* tp_clear */
    0,		                        /* tp_richcompare */
    0,		                        /* tp_weaklistoffset */
    0,		                        /* tp_iter */
    0,		                        /* tp_iternext */
    0,                              /* tp_methods */
    sass_options_members,           /* tp_members */
    sass_options_getsetters,        /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)sass_options_init,    /* tp_init */
    0,                              /* tp_alloc */
    0,                              /* tp_new */

};

static PyObject *
sass_compile_string(PyObject *self, PyObject *args)
{

    char *input;
    static struct sass_context *ctx;
    int retval;

    if (!PyArg_ParseTuple(args, "s", &input))
        return NULL;

    ctx = sass_new_context();
    ctx->input_string = input;
    ctx->options.include_paths = "";
    ctx->options.output_style = 0;
    retval = sass_compile(ctx);
    if (ctx->error_status) {
      return NULL;
    } else {
      return Py_BuildValue("s", ctx->output_string);  
    }
    
}

static PyMethodDef sass_methods[] = {
    
    {"compile",  sass_compile_string, METH_VARARGS,
     "compile from a sass context"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

#define _EXPORT_INT(mod, name, value)                                   \
    if (PyModule_AddIntConstant(mod, name, (long) value) == -1) return;

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef sass_module = {
    PyModuleDef_HEAD_INIT,
    "sass",
    sass_module_doc,
    -1,
    sass_methods,
    0,  /* m_reload */
    0,  /* m_traverse */
    0,  /* m_clear */
    0,  /* m_free */
};
#endif
	
PyMODINIT_FUNC
initsass(void) 
{
    PyObject *m;

#if PY_MAJOR_VERSION < 3
    m = Py_InitModule3("sass", sass_methods, sass_module_doc);
#else
    m = PyModule_Create(&alsaaudio_module);
#endif
    if (!m) 
        return;

    py_sass_options_type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&py_sass_options_type) < 0)
        return;

    Py_INCREF(&py_sass_options_type);
    PyModule_AddObject(m, "options", (PyObject *)&py_sass_options_type);
}
