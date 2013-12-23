/*------------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|-----------------------------------------------------------------------------*/
#include <kiwi/kiwi.h>
#include "pythonhelpers.h"
#include "symbolics.h"
#include "variable.h"


using namespace PythonHelpers;


static PyObject*
Variable_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
	static const char *kwlist[] = { "name", "context", 0 };
	PyObject* name;
	PyObject* context = 0;
	if( !PyArg_ParseTupleAndKeywords(
		args, kwargs, "S|O:__new__", const_cast<char**>( kwlist ), // grr
		&name, &context ) )
		return 0;
	PyObject* pyvar = PyType_GenericNew( type, args, kwargs );
	if( !pyvar )
		return 0;
	Variable* self = reinterpret_cast<Variable*>( pyvar );
	self->context = xnewref( context );
	new( &self->variable ) kiwi::Variable( PyString_AS_STRING( name ) );
	return pyvar;
}


static void
Variable_clear( Variable* self )
{
	Py_CLEAR( self->context );
}


static int
Variable_traverse( Variable* self, visitproc visit, void* arg )
{
	Py_VISIT( self->context );
	return 0;
}


static void
Variable_dealloc( Variable* self )
{
	PyObject_GC_UnTrack( self );
	Variable_clear( self );
	self->variable.~Variable();
	self->ob_type->tp_free( pyobject_cast( self ) );
}


static PyObject*
Variable_name( Variable* self )
{
	return PyString_FromString( self->variable.name().c_str() );
}


static PyObject*
Variable_setName( Variable* self, PyObject* pystr )
{
	if( !PyString_Check( pystr ) )
		return py_expected_type_fail( pystr, "str" );
	self->variable.setName( PyString_AS_STRING( pystr ) );
	Py_RETURN_NONE;
}


static PyObject*
Variable_context( Variable* self )
{
	if( self->context )
		return newref( self->context );
	Py_RETURN_NONE;
}


static PyObject*
Variable_setContext( Variable* self, PyObject* value )
{
	if( value != self->context )
	{
		PyObject* temp = self->context;
		self->context = newref( value );
		Py_XDECREF( temp );
	}
	Py_RETURN_NONE;
}


static PyObject*
Variable_value( Variable* self )
{
	return PyFloat_FromDouble( self->variable.value() );
}


static PyObject*
Variable_add( PyObject* first, PyObject* second )
{
	return BinaryInvoke<BinaryAdd, Variable>()( first, second );
}


static PyObject*
Variable_sub( PyObject* first, PyObject* second )
{
	return BinaryInvoke<BinarySub, Variable>()( first, second );
}


static PyObject*
Variable_mul( PyObject* first, PyObject* second )
{
	return BinaryInvoke<BinaryMul, Variable>()( first, second );
}


static PyObject*
Variable_div( PyObject* first, PyObject* second )
{
	return BinaryInvoke<BinaryDiv, Variable>()( first, second );
}


static PyObject*
Variable_neg( PyObject* value )
{
	return UnaryInvoke<UnaryNeg, Variable>()( value );
}


static PyMethodDef
Variable_methods[] = {
	{ "name", ( PyCFunction )Variable_name, METH_NOARGS,
	  "Get the name of the variable." },
	{ "setName", ( PyCFunction )Variable_setName, METH_O,
	  "Set the name of the variable." },
	{ "context", ( PyCFunction )Variable_context, METH_NOARGS,
	  "Get the context object associated with the variable." },
	{ "setContext", ( PyCFunction )Variable_setContext, METH_O,
	  "Set the context object associated with the variable." },
	{ "value", ( PyCFunction )Variable_value, METH_NOARGS,
	  "Get the current value of the variable." },
	{ 0 } // sentinel
};


static PyNumberMethods
Variable_as_number = {
	(binaryfunc)Variable_add,   /* nb_add */
	(binaryfunc)Variable_sub,   /* nb_subtract */
	(binaryfunc)Variable_mul,   /* nb_multiply */
	(binaryfunc)Variable_div,   /* nb_divide */
	0,                          /* nb_remainder */
	0,                          /* nb_divmod */
	0,                          /* nb_power */
	(unaryfunc)Variable_neg,    /* nb_negative */
	0,                          /* nb_positive */
	0,                          /* nb_absolute */
	0,                          /* nb_nonzero */
	0,                          /* nb_invert */
	0,                          /* nb_lshift */
	0,                          /* nb_rshift */
	0,                          /* nb_and */
	0,                          /* nb_xor */
	0,                          /* nb_or */
	0,                          /* nb_coerce */
	0,                          /* nb_int */
	0,                          /* nb_long */
	0,                          /* nb_float */
	0,                          /* nb_oct */
	0,                          /* nb_hex */
	0,                          /* nb_inplace_add */
	0,                          /* nb_inplace_subtract */
	0,                          /* nb_inplace_multiply */
	0,                          /* nb_inplace_divide */
	0,                          /* nb_inplace_remainder */
	0,                          /* nb_inplace_power */
	0,                          /* nb_inplace_lshift */
	0,                          /* nb_inplace_rshift */
	0,                          /* nb_inplace_and */
	0,                          /* nb_inplace_xor */
	0,                          /* nb_inplace_or */
	0,                          /* nb_floor_divide */
	0,                          /* nb_true_divide */
	0,                          /* nb_inplace_floor_divide */
	0,                          /* nb_inplace_true_divide */
	0,                          /* nb_index */
};


PyTypeObject Variable_Type = {
	PyObject_HEAD_INIT( 0 )
	0,                                      /* ob_size */
	"pykiwi.Variable",                      /* tp_name */
	sizeof( Variable ),                     /* tp_basicsize */
	0,                                      /* tp_itemsize */
	(destructor)Variable_dealloc,           /* tp_dealloc */
	(printfunc)0,                           /* tp_print */
	(getattrfunc)0,                         /* tp_getattr */
	(setattrfunc)0,                         /* tp_setattr */
	(cmpfunc)0,                             /* tp_compare */
	(reprfunc)0,                            /* tp_repr */
	(PyNumberMethods*)&Variable_as_number,  /* tp_as_number */
	(PySequenceMethods*)0,                  /* tp_as_sequence */
	(PyMappingMethods*)0,                   /* tp_as_mapping */
	(hashfunc)0,                            /* tp_hash */
	(ternaryfunc)0,                         /* tp_call */
	(reprfunc)0,                            /* tp_str */
	(getattrofunc)0,                        /* tp_getattro */
	(setattrofunc)0,                        /* tp_setattro */
	(PyBufferProcs*)0,                      /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_HAVE_GC|Py_TPFLAGS_BASETYPE|Py_TPFLAGS_CHECKTYPES, /* tp_flags */
	0,                                      /* Documentation string */
	(traverseproc)Variable_traverse,        /* tp_traverse */
	(inquiry)Variable_clear,                /* tp_clear */
	(richcmpfunc)0,                         /* tp_richcompare */
	0,                                      /* tp_weaklistoffset */
	(getiterfunc)0,                         /* tp_iter */
	(iternextfunc)0,                        /* tp_iternext */
	(struct PyMethodDef*)Variable_methods,  /* tp_methods */
	(struct PyMemberDef*)0,                 /* tp_members */
	0,                                      /* tp_getset */
	0,                                      /* tp_base */
	0,                                      /* tp_dict */
	(descrgetfunc)0,                        /* tp_descr_get */
	(descrsetfunc)0,                        /* tp_descr_set */
	0,                                      /* tp_dictoffset */
	(initproc)0,                            /* tp_init */
	(allocfunc)PyType_GenericAlloc,         /* tp_alloc */
	(newfunc)Variable_new,                  /* tp_new */
	(freefunc)PyObject_GC_Del,              /* tp_free */
	(inquiry)0,                             /* tp_is_gc */
	0,                                      /* tp_bases */
	0,                                      /* tp_mro */
	0,                                      /* tp_cache */
	0,                                      /* tp_subclasses */
	0,                                      /* tp_weaklist */
	(destructor)0                           /* tp_del */
};


int import_variable()
{
	return PyType_Ready( &Variable_Type );
}