// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// This test ensures that all public headers fully include all their dependancies, as well compile cleanly at maximum warning level

#ifdef _MSC_VER
#pragma warning(disable : 4619 4616 4061 4365 4668 4820 5031 5032 5039)
// C4619/4616 #pragma warning warnings
// C4061 enumerator 'X' in switch of enum 'X' is not explicitly handled by a case label
// C4365 signed/unsigned mismatch
// C4668 not defined as a preprocessor macro
// C4820 padding added after data member
// C5031/5032 push/pop mismatches in windows headers
// C5039 pointer or reference to potentially throwing function passed to extern C function under - EHc

// C5219 implicit conversion from 'int' to 'float', possible loss of data
#endif

#ifdef __clang__
#pragma clang diagnostic ignored "-Wc++98-compat"
#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#pragma clang diagnostic ignored "-Wno-language-extension-token"
#endif

#include "DirectXTex.h"

int main()
{
    return 0;
}
