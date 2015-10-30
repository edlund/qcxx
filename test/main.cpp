
/* qcxx - A test utility library
 * Copyright(c) 2015, Erik Edlund <erik.edlund@32767.se>
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 *  * Neither the name of Erik Edlund, nor the names of its contributors may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <qcxx.hpp>

#define PROPERTY_TYPE_GEN_IN_INTERVAL(_Name, _Type)                         \
    BEGIN_PROPERTY_TYPE(                                                    \
        _Name,                                                              \
        _Type,                                                              \
        _Type                                                               \
    ) PROPERTY_METHOD(                                                      \
        _Type x,                                                            \
        _Type y                                                             \
    ) {                                                                     \
        auto mn = std::min(x, y);                                           \
        auto mx = std::max(x, y);                                           \
        auto z = qcxx::get_generator<_Type>(this->engine())(mn, mx);        \
        return mn <= z && z <= mx;                                          \
    }                                                                       \
    END_PROPERTY_TYPE

PROPERTY_TYPE_GEN_IN_INTERVAL(prop_GenSignedIntInInterval, signed int)
PROPERTY_TYPE_GEN_IN_INTERVAL(prop_GenUnsignedIntInInterval, unsigned int)

PROPERTY_TYPE_GEN_IN_INTERVAL(prop_GenFloatInInterval, float)
PROPERTY_TYPE_GEN_IN_INTERVAL(prop_GenDoubleInInterval, double)

#define PROPERTY_TYPE_SHRINK_TO_ZERO(_Name, _Type)                          \
    BEGIN_PROPERTY_TYPE(                                                    \
        _Name,                                                              \
        _Type                                                               \
    ) PROPERTY_METHOD(                                                      \
        _Type x                                                             \
    ) {                                                                     \
        if (x == 0)                                                         \
            return qcxx::TEST_DISCARD;                                      \
        auto y = x;                                                         \
        auto ys = qcxx::get_minimizer<_Type>(this->engine())(x);            \
        if (ys.empty() || *ys.begin() != x || *ys.rbegin() != 0)            \
            return qcxx::TEST_FAILURE;                                      \
        for (auto i = ++ys.begin(); i != ys.end(); ++i) {                   \
            if (std::abs(*i) > std::abs(y))                                 \
                return qcxx::TEST_FAILURE;                                  \
            y = *i;                                                         \
        }                                                                   \
        return qcxx::TEST_SUCCESS;                                          \
    }                                                                       \
    END_PROPERTY_TYPE

PROPERTY_TYPE_SHRINK_TO_ZERO(prop_ShrinkSignedInt, signed int)
PROPERTY_TYPE_SHRINK_TO_ZERO(prop_ShrinkUnsignedInt, unsigned int)

PROPERTY_TYPE_SHRINK_TO_ZERO(prop_ShrinkFloat, float)
PROPERTY_TYPE_SHRINK_TO_ZERO(prop_ShrinkDouble, double)

BEGIN_PROPERTY_TYPE(
    prop_GenAndShrinkList,
    std::list<int>
) PROPERTY_METHOD(
    std::list<int> xs
) {
    if (xs.empty())
        return qcxx::TEST_DISCARD;
    
    auto ys = qcxx::get_minimizer<std::list<int>>(this->engine())(xs);
    auto y = ys.begin();
    
    if (ys.empty())
        return qcxx::TEST_FAILURE;
    
    auto n = y->size();
    
    for (++y; y != ys.end(); ++y) {
        if (y->size() > n)
            return qcxx::TEST_FAILURE;
        n = y->size();
    }
    
    return (
        ys.begin()->size() == xs.size() &&
        ys.rbegin()->size() == 0
    );
}
END_PROPERTY_TYPE

BEGIN_PROPERTY_TYPE(
    prop_OneofList,
    std::list<int>
) PROPERTY_METHOD(
    std::list<int> xs
) {
    if (xs.empty())
        return qcxx::TEST_DISCARD;
    
    auto x = std::find(
        xs.begin(),
        xs.end(),
        *qcxx::oneof(this->engine(), xs)
    );
    
    return x != xs.end();
}
END_PROPERTY_TYPE

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    
    qcxx::quickCheck<prop_GenSignedIntInInterval>();
    qcxx::quickCheck<prop_GenUnsignedIntInInterval>();
    
    qcxx::quickCheck<prop_GenFloatInInterval>();
    qcxx::quickCheck<prop_GenDoubleInInterval>();
    
    qcxx::quickCheck<prop_ShrinkSignedInt>();
    qcxx::quickCheck<prop_ShrinkUnsignedInt>();
    
    qcxx::quickCheck<prop_ShrinkFloat>();
    qcxx::quickCheck<prop_ShrinkDouble>();
    
    qcxx::quickCheck<prop_GenAndShrinkList>();
    
    qcxx::quickCheck<prop_OneofList>();
    
    return 0;
}

