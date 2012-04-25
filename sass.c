#include <Python.h>
#include <structmember.h>
#include "libsass/sass_interface.h"

PyDoc_STRVAR(sass_module_doc,
             "sass - document me\n");

// py_sass_options 

typedef struct {
    PyObject_HEAD;
    // The options are stored directly in the python object
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
    if (self->options.include_paths) {
        free(self->options.include_paths);
        self->options.include_paths = NULL;
    }

    if (value == NULL) {
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
	PyObject_HEAD_INIT(NULL)
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

// py_sass_context

typedef struct {
    PyObject_HEAD;
    // sass_context should be created by sass_new_context
    struct sass_context *context;
} py_sass_context;

static PyTypeObject py_sass_context_type;

static PyObject *
sass_context_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    py_sass_context *self;

    self = (py_sass_context*)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->context = sass_new_context();
        memset(self->context, 0, sizeof(struct sass_context));
    }

    return (PyObject *)self;
}

static int
sass_context_init(py_sass_context *self, PyObject *args, PyObject *kwds) 
{
    char* input_string = NULL;
    PyObject *options = NULL;

    static char *kwlist[] = { "input_string", "options", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|sO:options",
                                     kwlist, &input_string, &options))
        return -1;

    const size_t len = input_string ? strlen(input_string) : 0;
    if (len) { 
        self->context->input_string = malloc(len + 1);
        if (!self->context->input_string) {
            PyErr_NoMemory();
            return -1;
        }
        strcpy(self->context->input_string, input_string);
    }
    if (options) {
        if (options->ob_type != &py_sass_options_type) {
            PyErr_SetString(PyExc_TypeError, 
                            "options must be of type sass.options");
            return -1;
        }
        Py_INCREF(&options);
        py_sass_options *py_opt = (py_sass_options*)options;
        self->context->options = py_opt->options;
    }

	return 0;
}

static void
sass_context_dealloc(py_sass_context *obj)
{
    if (obj->context) {
        
        // If there is an input_string, we have set it
        if (obj->context->input_string) {
            free(obj->context->input_string);
        }

        sass_free_context(obj->context);
    }
    PyObject_Free(obj);
}

static PyObject *
sass_context_get_input_string(py_sass_context *self, void *closure)
{
    if (!self->context->input_string) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    return PyString_FromString(self->context->input_string);
}

static int
sass_context_set_input_string(py_sass_context *self, PyObject *value, 
                              void *closure)
{
    if (self->context->input_string) {
        free(self->context->input_string);
        self->context->input_string = NULL;
    }

    if (value == NULL) {
        return 0;
    }
    
    if (!PyString_Check(value)) {
        PyErr_SetString(PyExc_TypeError, 
                        "The include_paths attribute value must be a string");
        return -1;
    }
    
    const Py_ssize_t len = PyString_GET_SIZE(value);
    if (len) { 
        self->context->input_string = malloc(len + 1);
        if (!self->context->input_string) {
            PyErr_NoMemory();
            return -1;
        }

        strcpy(self->context->input_string, PyString_AsString(value));
    }

    return 0;
}

static PyObject *
sass_context_get_error_message(py_sass_context *self, void *closure)
{
    if (!self->context->error_message) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    return PyString_FromString(self->context->error_message);
}

static PyObject *
sass_context_get_error_status(py_sass_context *self, void *closure)
{
    return PyLong_FromLong(self->context->error_status);
}

static PyGetSetDef sass_context_getsetters[] = {
    {"input_string", 
     (getter)sass_context_get_input_string,
     (setter)sass_context_set_input_string,
     "input string", NULL},
    {"error_message", 
     (getter)sass_context_get_error_message, NULL,
     "error message", NULL},
    {"error_status", 
     (getter)sass_context_get_error_status, NULL,
     "error status", NULL},
    {NULL}  /* Sentinel */
};

static PyTypeObject py_sass_context_type = {
	PyObject_HEAD_INIT(NULL)
	0,                              /*ob_size*/
	"sass.context",                 /*tp_name*/
	sizeof(py_sass_context_type),   /*tp_basicsize*/
	0,                              /*tp_itemsize*/
	/* methods */    
	(destructor)sass_context_dealloc, /*tp_dealloc*/
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
	"sass context",  /*tp_doc*/
    0,		                        /* tp_traverse */
    0,		                        /* tp_clear */
    0,		                        /* tp_richcompare */
    0,		                        /* tp_weaklistoffset */
    0,		                        /* tp_iter */
    0,		                        /* tp_iternext */
    0,                              /* tp_methods */
    0,                              /* tp_members */
    sass_context_getsetters,        /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)sass_context_init,    /* tp_init */
    0,                              /* tp_alloc */
    sass_context_new,               /* tp_new */

};

static PyObject *
sass_compile_string(PyObject *self, PyObject *args)
{
    PyObject *input;
    PyObject *options = NULL;
    struct sass_context *ctx;
    struct sass_context *local_ctx = NULL;
    int retval;
    PyObject *rc = NULL;

    if (!PyArg_ParseTuple(args, "O|O", &input, &options))
        return NULL;

    if (PyString_Check(input)) {
        // Convenience: if the input is a string, create a context on demand
        local_ctx = sass_new_context();
        memset(local_ctx, 0, sizeof(struct sass_context));
        ctx = local_ctx;
        ctx->input_string = PyString_AsString(input);
    }
    else {
        if (input->ob_type != &py_sass_context_type) {
            PyErr_SetString(PyExc_TypeError, 
                            "input must be a string or a sass.context");
            return NULL;
        }
        py_sass_context *py_ctx = (py_sass_context*)input;
        ctx = py_ctx->context;
    }

    if (options) {
        if (options->ob_type != &py_sass_options_type) {
            PyErr_SetString(PyExc_TypeError, 
                            "options must be of type sass.options");
            return NULL;
        }

        py_sass_options *py_opts = (py_sass_options*)options;
        ctx->options = py_opts->options;
    }

    retval = sass_compile(ctx);

    if (ctx->error_status) {
        PyErr_SetString(PyExc_RuntimeError, ctx->error_message);
    } else {
        rc = Py_BuildValue("s", ctx->output_string);  
    }

    if (local_ctx)
        free(local_ctx);

    return rc;
}

static PyMethodDef sass_methods[] = {
    
    {"compile_string",  sass_compile_string, METH_VARARGS,
     "compile from string"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

#define _EXPORT_INT(mod, name, value)  \
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
    m = PyModule_Create(&sass_module);
#endif
    if (!m) 
        return;

    py_sass_options_type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&py_sass_options_type) < 0)
        return;

    Py_INCREF(&py_sass_options_type);
    PyModule_AddObject(m, "options", (PyObject *)&py_sass_options_type);

    if (PyType_Ready(&py_sass_context_type) < 0)
        return;

    Py_INCREF(&py_sass_context_type);
    PyModule_AddObject(m, "context", (PyObject *)&py_sass_context_type);

    _EXPORT_INT(m, "SASS_STYLE_NESTED", SASS_STYLE_NESTED);
    _EXPORT_INT(m, "SASS_STYLE_EXPANDED", SASS_STYLE_EXPANDED);
    _EXPORT_INT(m, "SASS_STYLE_COMPACT", SASS_STYLE_COMPACT);
    _EXPORT_INT(m, "SASS_STYLE_COMPRESSED", SASS_STYLE_COMPRESSED);
}
