/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/crypto.h>
#include <openssl/opensslconf.h>

# include <openssl/opensslv.h>

/* The implementation is in ../md32_common.h */

# include "sha_local.h"

#ifdef __KERNEL__
#include <linux/module.h>
EXPORT_SYMBOL(SHA1_Init);
EXPORT_SYMBOL(SHA1_Update);
EXPORT_SYMBOL(SHA1_Final);
EXPORT_SYMBOL(SHA1_Transform);
#endif
