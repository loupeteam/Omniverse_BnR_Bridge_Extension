#ifndef _AR_STATE_H_
#define _AR_STATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define AR_OK (0)

typedef int AR_STATE; /* AR_OK or any error, test for != AR_OK */


#if defined(UT_BUILD)
/**
 * For UTs for std-libs both runtime.h and arState.h can be
 * included and we cant set the _AR_I386_AUTOMATION_STUDIO_ define
 * globally.
 */
#if !defined(ERR_OK)
#define ERR_OK AR_OK
#endif


#elif !defined(__cplusplus) && !defined(_AR_I386_AUTOMATION_STUDIO_)
/* For C++ ERR_OK is defined in Rtk.h as member of the RTK_ERROR enum.
 * For stdlibs ERR_OK is not defined here, but in runtime.h
 * ARTIG-Guidelines: Use AR_OK instead of ERR_OK */
#undef ERR_OK
#define ERR_OK AR_OK
#endif

#ifdef __cplusplus
}
#endif

#endif
