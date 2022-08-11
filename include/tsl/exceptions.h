/**
 * MIT License
 *
 * Copyright (c) 2022 Thibaut Goetghebuer-Planchon <tessil@gmx.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef TSL_EXCEPTIONS_H
#define TSL_EXCEPTIONS_H

#ifndef TSL_NO_EXCEPTIONS
#if (defined(__cpp_exceptions) || defined(__EXCEPTIONS) || \
     (defined(_MSC_VER) && defined(_CPPUNWIND)))
#define TSL_NO_EXCEPTIONS 0
#else
#define TSL_NO_EXCEPTIONS 1
#endif
#endif

#if TSL_NO_EXCEPTIONS
#include <cstdio>
#include <cstdlib>

#define TSL_SM_THROW_OR_TERMINATE(ex, msg)    \
  do {                                        \
    std::fprintf(stderr, "error: %s\n", msg); \
    std::abort();                             \
  } while (0)
#define TSL_SM_TRY if (true)
#define TSL_SM_CATCH(x) if (false)
#define TSL_SM_RETRHOW
#else
#include <stdexcept>  // IWYU pragma: export

#define TSL_SM_THROW_OR_TERMINATE(ex, msg) throw ex(msg)
#define TSL_SM_TRY try
#define TSL_SM_CATCH(x) catch (x)
#define TSL_SM_RETRHOW throw
#endif

#endif
