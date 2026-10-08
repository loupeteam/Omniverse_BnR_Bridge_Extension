/* This headerfile contains makros used to mark exported functions as deprecated or as not recommended */

#ifndef _AR_EXPORT_H_
#define _AR_EXPORT_H_

/* Functions marked with AR_DEPRECATED should not be used in AR-external code at all.
 * AR still supports these functions, but cannot provide guarantees for the compatibility
 * of these functions.
 * Any AR-external code using AR_DEPRECATED functions should replace these functions with
 * the equivalent exported function which is not marked as AR_DEPRECATED. If you cannot
 * find such a function in the exported AR headerfiles, then talk to the AR team. */
#if !defined(AR_CORE) && defined(__GNUC__)
#define AR_DEPRECATED __attribute__((deprecated))
#else
#define AR_DEPRECATED
#endif

/* Functions marked with AR_NOT_RECOMMENDED should not be used when writing new AR-external code.
 * However, for these functions there is no functionally equivalent alternative available and thus
 * they cannot be replaced easily in existing code (that is why they are not AR_DEPRECATED).
 * If you think that you need an AR_NOT_RECOMMENDED function when writing new code, then talk
 * to the AR team - they'll try to help you finding a better solution to your problem. */
#define AR_NOT_RECOMMENDED

#endif
