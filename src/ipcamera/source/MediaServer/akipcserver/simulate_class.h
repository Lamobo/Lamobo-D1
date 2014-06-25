/**
 * @FILENAME: simulate_class.h
 * @BRIEF simulate the c++ class.
 *        define some marco to simulate some c++ characteristic
 *        Copyright (C) 2012 Anyka (Guang zhou) Software Technology Co., LTD
 * @AUTHOR han kejia
 * @DATA 2012-7-15
 * @VERSION 1.0
 * @REF please refer to...
 **/

#ifndef AKDEMO_SIMULATE_CLASS
#define AKDEMO_SIMULATE_CLASS

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#include "anyka_types.h"

#define CLASS						typedef struct
#define CLASS_END( type )			type

//Declare a simple simulate constructor function for class
#define DECLARE_CONSTRUCTOR_POINT( type ) \
	struct type * (* type##_constructor)( T_pVOID pthis )

//Declare a simple simulate destructor function for class
#define DECLARE_DESTRUCTOR_POINT( type ) \
	T_VOID (* type##_destructor)( T_pVOID pthis )

//the simulate construct fuction achieve, add the achieve code
//between DEFINE_CONSTRUCTOR_BEGIN and DEFINE_CONSTRUCTOR_END
#define DEFINE_CONSTRUCTOR_BEGIN( type ) \
	static type * type##_constructor( T_pVOID pthis )
#define DEFINE_CONSTRUCTOR_END

//the simulate destructor fuction achieve, add the achieve code
//between DEFINE_DESTRUCTOR_BEGIN and DEFINE_DESTRUCTOR_END
#define DEFINE_DESTRUCTOR_BEGIN( type ) \
	static T_VOID type##_destructor( T_pVOID pthis )
#define DEFINE_DESTRUCTOR_END

//construct fuction
#define CONSTRUCTOR_FUN( type )	type##_constructor

//destruct fuction
#define DESTRUCTOR_FUN( type )	type##_destructor

//Register the function to the simulate class
#define REGISTER_FUN_BEGIN( type, constructor_fun, destructor_fun ) \
	type __this_Module_##type = { \
		.type##_constructor = constructor_fun, \
		.type##_destructor = destructor_fun,
#define REGISTER_FUN( module_fun, fun ) \
		.module_fun = fun,
#define REGISTER_FUN_END( type ) \
	};

//Register the simulate class, declare the simulate new function
//and simulate delete function, this macro use in h file
#define REGISTER_SIMULATE_CLASS_H( type ) \
	type * simulate_new_##type();\
	T_VOID simulate_delete_##type( type *pdelete ); \
	T_VOID load_##type##_module( type * pClass ); \
	T_VOID unload_##type##_module( type * pClass )

//Register the simulate class, define the simulate new function
//and simulate delete function, this macro use in c file
#define REGISTER_SIMULATE_CLASS_C( type ) \
	type * simulate_new_##type() { \
		type * pclass = ( type * )malloc( sizeof( type ) ); \
		if ( NULL == pclass ) { \
			return NULL; \
		} \
		memcpy( pclass, &__this_Module_##type, sizeof( type ) ); \
		\
		return pclass->type##_constructor( pclass ); \
	} \
	\
	T_VOID simulate_delete_##type( type *pdelete ) { \
		if ( NULL == pdelete ) \
			return; \
		\
		pdelete->type##_destructor( pdelete ); \
		free( pdelete ); \
	} \
	\
	T_VOID load_##type##_module( type * pClass ) { \
		if ( NULL == pClass ) \
			return; \
		memcpy( pClass, &__this_Module_##type, sizeof( type ) ); \
		pClass->type##_constructor( pClass ); \
	} \
	\
	T_VOID unload_##type##_module( type * pClass ) { \
		if ( NULL == pClass ) \
			return; \
		pClass->type##_destructor( pClass );\
	}
	
//simulate new
#define NEW_SIMULATE_CLASS( type ) \
	simulate_new_##type()

//simulate delete
#define DEL_SIMULATE_CLASS( type, DelPoint ) \
	simulate_delete_##type( DelPoint )

//define simulate class code
#define SIMULATE_CLASS_DEFINE( type, name ) \
	type name; \
	load_##type##_module( &name )

//pairing of SIMULATE_CLASS_DEFINE
#define SIMULATE_CLASS_UNDEFINE( type, v ) \
	unload_##type##_module( &v )

#ifdef __cplusplus
}
#endif

#endif