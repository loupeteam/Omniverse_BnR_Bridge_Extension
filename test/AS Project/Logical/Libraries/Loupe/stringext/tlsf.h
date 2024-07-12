/** @file
 *******************************************************************************
 **
 ** @brief
 ** Two Levels Segregate Fit memory allocator (TLSF)
 ** Written by Miguel Masmano Tello <mimastel@doctor.upv.es>
 **
 ** This code is released using a dual license strategy: GPL/LGPL
 **
 *******************************************************************************
 ** $Header: https://brateggevsvn1.br-automation.co.at/svn/motion_gmc/trunk/GMC/Core/sys/tlsf.h 2314 2013-11-26 09:33:41Z eisenmannm $
 *******************************************************************************
 **
 ** @remark
 ** This file is part of the common AR C++ System environment.
 **
 ** @copyright
 ** <a href="http://www.br-automation.com/">
 ** Bernecker + Rainer Industrie-Elektronik Ges.m.b.H.</a>
 **
 ** @date 2012-06-21	File has been adopted by Markus Eisenmann
 **
 ******************************************************************************/

#ifndef _ARSYS_TLSF_H_INCLUDED_
#define _ARSYS_TLSF_H_INCLUDED_

#ifndef _SIZE_T_DEFINED
#include <stddef.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/** @cond COMPILER_SPECIFICS */
#ifndef DECLSPEC_NOTHROW
#if defined(__GNUC__) && ((__GNUC__ > 3) || \
						  ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 3)))
#define DECLSPEC_NOTHROW __attribute__((__nothrow__))
#elif (_MSC_VER >= 1200) && defined(__cplusplus)
#define DECLSPEC_NOTHROW __declspec(nothrow)
#else
#define DECLSPEC_NOTHROW
#endif
#endif

#if defined(__ELF__) && (__GNUC__ >= 4)
#pragma GCC visibility push(hidden)
#define BURTLSF_DLLAPI __attribute__((visibility("default"), __nothrow__))
#else
#if defined(_WIN32) || defined(__WINDOWS__) || defined(__MINGW32__) || defined(__CYGWIN__)
#if defined(_ARSVCREG_EXPORT) || defined(BURTLSF_EXPORTS)
#define BURTLSF_DLLAPI __declspec(dllexport) DECLSPEC_NOTHROW
#else
#define BURTLSF_DLLAPI __declspec(dllimport) DECLSPEC_NOTHROW
#endif
#else /* not Windows */
#define BURTLSF_DLLAPI extern DECLSPEC_NOTHROW
#endif /* end Windows */
#endif
	/** @endcond */

	DECLSPEC_NOTHROW size_t _tlsf_init_memory_pool(size_t mem_pool_size, void *mem_pool);
	DECLSPEC_NOTHROW size_t _tlsf_add_new_area(void *area, size_t area_size, void *mem_pool);
#if !defined(NDEBUG) && (defined(DEBUG) || defined(_DEBUG))
	DECLSPEC_NOTHROW size_t _tlsf_get_used_size(void *mem_pool);
	DECLSPEC_NOTHROW size_t _tlsf_get_max_size(void *mem_pool);
#endif
	DECLSPEC_NOTHROW void _tlsf_destroy_memory_pool(void *mem_pool);
	DECLSPEC_NOTHROW void *_tlsf_malloc_ex(size_t size, void *mem_pool);
	DECLSPEC_NOTHROW void *_tlsf_free_ex(void *ptr, void *mem_pool);
	DECLSPEC_NOTHROW void *_tlsf_realloc_ex(void *ptr, size_t new_size, void *mem_pool);
	DECLSPEC_NOTHROW void *_tlsf_calloc_ex(size_t nelem, size_t elem_size, void *mem_pool);

#if defined(__ELF__) && (__GNUC__ >= 4)
#pragma GCC visibility pop
#endif

	/*lint -esym(526, tlsf_*) prevent warning about undefined functions*/
	/*exos BURTLSF_DLLAPI*/ void *tlsf_malloc(size_t size);
	/*exos BURTLSF_DLLAPI*/ void tlsf_free(void *ptr);
	/*exos BURTLSF_DLLAPI*/ void *tlsf_realloc(void *ptr, size_t size);
	/*exos BURTLSF_DLLAPI*/ void *tlsf_calloc(size_t nelem, size_t elem_size);

	/** @cond HIDDEN_INTERNALS */
	extern int const _force_tlfs_malloc; /*link symbol to force use of TLSF*/

#if defined(_ENFORCE_TLFS_REDIRECT) && !(defined(_CODECHECK_PASS) || defined(_CODEPARSE_PASS) || defined(__CDT_PARSER__) || defined(_lint) || defined(__DOXYGEN) || defined(_doxygen))
#if (__GNUC__ >= 3) && !defined(WIN32)
	__extension__ void *malloc(size_t) __asm__("tlsf_malloc");
	__extension__ void free(void *) __asm__("tlsf_free");
	__extension__ void *realloc(void *, size_t) __asm__("tlsf_realloc");
	__extension__ void *calloc(size_t, size_t) __asm__("tlsf_calloc");
#else
#error "Forcing TSLF-redirection of DSA not supported!"
#endif
#endif
	/** @endcond */

#ifdef __cplusplus
} // extern "C"
#endif
#endif /*_ARSYS_TLSF_H_INCLUDED_*/