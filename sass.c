#include <Python.h>
#include "libsass/sass_interface.h"

static PyObject *
sass_compile_string(PyObject *self, PyObject *args)
{

    const char *input;
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


static PyMethodDef SassMethods[] = {
    
    {"compile_string",  sass_compile_string, METH_VARARGS,
     "compile from string"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initsass(void) {
    (void) Py_InitModule("sass", SassMethods);
}
