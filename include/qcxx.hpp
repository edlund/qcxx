
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

#ifndef QCXX_HPP
#define QCXX_HPP

#include <algorithm>
#include <cmath>
#include <exception>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <random>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <vector>

#define QCXX_THROW_(_E, _F, _L, _S)                                         \
    throw _E(_F "[" #_L "]:" _S)

#define QCXX_THROW(_Exception, _String)                                     \
    QCXX_THROW_(_Exception, __FILE__, __LINE__, _String)

namespace qcxx {

typedef unsigned int size_type;

/* Default minimizer for non-specialized types. Every minimizer
 * must produce a list of at least one value, if it can not
 * shrink it, it can contain the original value.
 */
template
<
    typename Type,
    typename Engine
>
class minimizer
{
public:
    typedef Type value_type;
    typedef Engine engine_type;
    typedef std::list<
        value_type
    > list_type;
    
    explicit
    minimizer(
        Engine& engine
    ) :
        engine_(engine)
    {}
    
#define MINIMIZER_CTOR(_Class)                                              \
    explicit                                                                \
    _Class(                                                                 \
        Engine& engine                                                      \
    ) :                                                                     \
        minimizer<                                                          \
            Type,                                                           \
            Engine                                                          \
        >(engine)                                                           \
    {}
    
    virtual
    ~minimizer(
        void
    )
    {}
    
    virtual std::list<Type>
    operator()(
        const Type& x
    ) {
        std::list<Type> xs;
        xs.push_front(x);
        return xs;
    }
    
    Engine&
    engine(
        void
    ) {
        return this->engine_;
    }
    
private:
    Engine& engine_;
};

/* Specialize this template if it is possible (somewhat
 * easy) to implement shrinking for the type in question,
 * preferably using %SHRINK_TYPE.
 */
template
<
    typename Type,
    typename Engine
>
class shrink
{
public:
    typedef minimizer<
        Type,
        Engine
    > minimizer_type;
};

/* Create a specialization for the shrink template using the
 * given %Minimizer and %Type.
 */
#define SHRINK_TYPE(_Minimizer, _Type)                                      \
    template                                                                \
    <                                                                       \
        typename Engine                                                     \
    >                                                                       \
    class shrink<                                                           \
        _Type,                                                              \
        Engine                                                              \
    > {                                                                     \
    public:                                                                 \
        typedef _Minimizer<                                                 \
            _Type,                                                          \
            Engine                                                          \
        > minimizer_type;                                                   \
    }

/* Get a minimizer for the given %Type, use the minimizer
 * to get a list of values.
 */
template
<
    typename Type,
    typename Engine
>
typename shrink<
    Type,
    Engine
>::minimizer_type
get_minimizer(
    Engine& engine
) {
    return typename shrink<
        Type,
        Engine
    >::minimizer_type(engine);
}

/* Specialize this template, preferably using %ARBITRARY_TYPE.
 */
template
<
    typename Type,
    typename Engine
>
class arbitrary;

/* Create a specialization for the arbitrary template using the
 * given %Generator and %Type.
 */
#define ARBITRARY_TYPE(_Generator, _Type)                                   \
    template                                                                \
    <                                                                       \
        typename Engine                                                     \
    >                                                                       \
    class arbitrary<                                                        \
        _Type,                                                              \
        Engine                                                              \
    > {                                                                     \
    public:                                                                 \
        typedef _Generator<                                                 \
            _Type,                                                          \
            Engine                                                          \
        > generator_type;                                                   \
    }

/* Get a generator for the given %Type, use the generator
 * to get values.
 */
template
<
    typename Type,
    typename Engine
>
typename arbitrary<
    Type,
    Engine
>::generator_type
get_generator(
    Engine& engine
) {
    return typename arbitrary<
        Type,
        Engine
    >::generator_type(engine);
}

/* Base class template for value generators.
 */
template
<
    typename Type,
    typename Engine
>
class generator
{
public:
    typedef Type value_type;
    typedef Engine engine_type;
    
    explicit
    generator(
        Engine& engine
    ) :
        engine_(engine)
    {}
    
#define GENERATOR_CTOR(_Class)                                              \
    explicit                                                                \
    _Class(                                                                 \
        Engine& engine                                                      \
    ) :                                                                     \
        generator<                                                          \
            Type,                                                           \
            Engine                                                          \
        >(engine)                                                           \
    {}
    
    virtual
    ~generator(
        void
    ) {
    }
    
    virtual Type
    operator()(
        void
    ) = 0;
    
    Engine&
    engine(
        void
    ) {
        return this->engine_;
    }
    
private:
    Engine& engine_;
};

/* Generate uniform distributions of integral or floating point
 * values, possibly within a given range, otherwise within the
 * limits of the given type.
 */
template
<
    typename Type,
    typename Engine,
    template
    <
        typename
    >
    class Uniform
>
class uniform_numeric_generator :
    public generator<
        Type,
        Engine
    >
{
public:
    
    GENERATOR_CTOR(uniform_numeric_generator)
    
    virtual Type
    operator()(
        const Type& min,
        const Type& max
    ) {
        static_assert(
            (std::is_integral<Type>::value &&
                std::is_same<
                    Uniform<Type>,
                    std::uniform_int_distribution<Type>
                >::value) ||
            (std::is_floating_point<Type>::value &&
                std::is_same<
                    Uniform<Type>,
                    std::uniform_real_distribution<Type>
                >::value),
            
            "Type/Uniform mismatch"
        );
        
        Uniform<
            Type
        > distribution(min, max);
        
        return distribution(this->engine());
    }
    
    virtual Type
    operator()(
        const std::pair<Type, Type>& pair
    ) {
        return (*this)(
            pair.first,
            pair.second
        );
    }
    
    virtual Type
    operator()(
        void
    ) {
        return (*this)(
            std::numeric_limits<Type>::min(),
            std::numeric_limits<Type>::max()
        );
    }
    
};

/* Generate uniform integral values.
 */
template
<
    typename Integral,
    typename Engine
>
using uniform_integral_generator = uniform_numeric_generator
<
    Integral,
    Engine,
    std::uniform_int_distribution
>;

/* Generate uniform real values.
 */
template
<
    typename Real,
    typename Engine
>
using uniform_real_generator = uniform_numeric_generator
<
    Real,
    Engine,
    std::uniform_real_distribution
>;

/* Generate containers filled with arbitrary values. The given
 * container must have a %push_back() method.
 */
template
<
    typename Type,
    typename Engine
>
class back_container_generator :
    public generator<
        Type,
        Engine
    >
{
public:
    
    GENERATOR_CTOR(back_container_generator)
    
    virtual Type
    operator()(
        const typename Type::size_type& n
    ) {
        auto xs = Type();
        auto gen = get_generator<
            typename Type::value_type
        >(this->engine());
        
        for (typename Type::size_type i = 0; i < n; ++i) {
            xs.push_back(gen());
        }
        return xs;
    }
    
    virtual Type
    operator()(
        void
    ) {
        auto gen = get_generator<
            typename Type::size_type
        >(this->engine());
        
        return (*this)(gen(0, 128));
    }
    
};

/* Shrink an %Integral value to zero.
 */
template
<
    typename Type,
    typename Engine
>
class integral_minimizer :
    public minimizer<
        Type,
        Engine
    >
{
public:
    
    MINIMIZER_CTOR(integral_minimizer)
    
    virtual std::list<Type>
    operator()(
        const Type& x
    ) {
        static_assert(std::is_integral<Type>::value,
            "given Type is not Integral");
        
        auto gen = get_generator<Type>(this->engine());
        
        std::list<Type> xs;
        for (Type i = x; i != 0; i /= gen(2, 3)) {
            xs.push_back(i);
        }
        xs.push_back(0);
        
        return xs;
    }
    
};

/* Shrink a %Real value to zero.
 */
template
<
    typename Type,
    typename Engine
>
class real_minimizer :
    public minimizer<
        Type,
        Engine
    >
{
public:
    
    MINIMIZER_CTOR(real_minimizer)
    
    virtual std::list<Type>
    operator()(
        const Type& x
    ) {
        static_assert(std::is_floating_point<Type>::value,
            "given Type is not Real");
        
        std::list<Type> xs;
        if (std::isfinite(x)) {
            Type a;
            Type b = x;
            
            auto gen = get_generator<Type>(this->engine());
            auto vel = [](const auto& d)
            {
                return d < 2e64?
                    std::make_pair(2e1, 3e1):
                    (d < 2e128?
                        std::make_pair(2e10, 2e20):
                        std::make_pair(2e75, 2e100));
            };
            
            do {
                a = std::abs(b);
                b = b / gen(vel(a));
                xs.push_back(b);
            } while (a > 1e-20);
        }
        xs.push_front(x);
        xs.push_back(0);
        
        return xs;
    }
    
};

/* Shrink a %Container to empty. The given container must
 * support construction from an iterator range and must have
 * a %size() method.
 */
template
<
    typename Type,
    typename Engine
>
class container_minimizer :
    public minimizer<
        Type,
        Engine
    >
{
public:
    
    MINIMIZER_CTOR(container_minimizer)
    
    virtual std::list<Type>
    operator()(
        const Type& x
    ) {
        typedef typename std::list<Type>::size_type size_type;
        
        auto min = get_minimizer<size_type>(this->engine());
        auto ns = min(x.size());
        
        std::list<Type> xs;
        for (auto n : ns) {
            auto i = x.begin();
            std::advance(i, n);
            xs.push_back(Type(x.begin(), i));
        }
        
        return xs;
    }
    
};

template
<
    typename Container
>
void
show_container(
    std::ostream& out,
    const Container& xs
) {
    std::ostringstream osstr;
    
    if (!xs.empty()) {
        auto beg = xs.begin();
        auto it = beg;
        auto n = xs.size() - 1;
        std::advance(it, n);
        std::copy(
            beg,
            it,
            std::ostream_iterator<
                typename Container::value_type
            >(osstr, ", ")
        );
        
        osstr << *xs.rbegin();
    }
    
    out << "["
        << osstr.str()
        << "]"
        << std::endl;
}

template
<
    typename Type
>
void
show(
    std::ostream& out,
    const Type& x
) {
    out << x
        << std::endl;
}

#define SHOWABLE_TYPE(_Type, _Show)                                         \
    template                                                                \
    <                                                                       \
    >                                                                       \
    void                                                                    \
    show<                                                                   \
        _Type                                                               \
    >(                                                                      \
        std::ostream& out,                                                  \
        const _Type& t                                                      \
    ) {                                                                     \
        _Show(out, t);                                                      \
    }

/**
 * %show_all()
 * @{
 */
template
<
    typename Type
>
void
show_all(
    std::ostream& out,
    const Type& x
) {
    show(out, x);
}
template
<
    typename Type,
    typename... Container
>
void
show_all(
    std::ostream& out,
    const Type& x,
    const Container&... xs
) {
    show_all(out, x);
    show_all(out, xs...);
}
/**
 * @}
 */

enum state {
    TEST_FAILURE = 0,
    TEST_SUCCESS = 1,
    TEST_DISCARD = 2,
    TEST_NOTHING = 3
};

class result
{
public:
    result(
        void
    ) :
        s_(TEST_NOTHING)
    {}
    
    result(
        const bool& s
    ) :
        s_(static_cast<state>(s))
    {}
    
    result(
        const state& s
    ) :
        s_(s)
    {}
    
    operator state(
        void
    ) const {
        return this->s_;
    }
    
private:
    state s_;
};

struct qc_config
{
    qc_config(
        void
    ) :
        n_tests(0),
        max_tests(128),
        n_discards(0),
        max_discards(1024)
    {}
    
    bool
    again(
        void
    ) const {
        return (
            n_tests < max_tests &&
            n_discards < max_discards
        );
    }
    
    size_type n_tests;
    size_type max_tests;
    size_type n_discards;
    size_type max_discards;
    
};

/* Base class for properties.
 */
template
<
    typename Engine,
    typename... Params
>
class property
{
public:
    typedef Engine engine_type;
    
    explicit
    property(
        engine_type& engine,
        qc_config& conf
    ) :
        engine_(engine),
        conf_(conf)
    {}
    
    virtual
    ~property(
        void
    ) {
    }
    
    virtual result
    test(
        Params...
    ) = 0;
    
    virtual result
    step(
        std::ostream& out,
        const std::list<Params>&... xs
    ) {
        auto r0 = this->test(this->data(xs)...);
        auto wr = false;
        
        if (r0 == TEST_FAILURE) {
            if (this->reducible(xs...)) {
                auto r1 = this->step(
                    out,
                    this->reduce(xs)...
                );
                wr = (
                    TEST_SUCCESS == r1 ||
                    TEST_DISCARD == r1 ||
                    TEST_NOTHING == r1
                );
            } else
                wr = true;
        }
        if (wr)
            this->failure(out, this->data(xs)...);
        
        return r0;
    }
    
    virtual result
    go(
        std::ostream& out
    ) {
        result r;
        
        while (this->config().again() && r != TEST_FAILURE) {
            try {
                r = this->step(
                    out,
                    get_minimizer<Params>(this->engine())(
                        get_generator<Params>(
                            this->engine()
                        )()
                    )...
                );
            } catch(...) {
                out << "Failed, caught exception after "
                    << this->config().n_tests + 1
                    << " tests"
                    << std::endl;
                
                r = TEST_FAILURE;
            }
            
            switch (r) {
            case TEST_SUCCESS:
                this->config().n_tests++;
                break;
            case TEST_DISCARD:
                this->config().n_discards++;
                break;
            default:
                break;
            }
        }
        
        if (r == TEST_SUCCESS) {
            out << "OK, "
                << this->config().n_tests
                << " tests passed, "
                << this->config().n_discards
                << " tests discarded"
                << std::endl;
        }
        
        return r;
    }
    
    virtual void
    failure(
        std::ostream& out,
        Params... xs
    ) {
        out << "Falsifiable, after "
            << this->config().n_tests + 1
            << " tests: "
            << std::endl;
        
        show_all(out, xs...);
    }
    
    engine_type&
    engine(
        void
    ) {
        return this->engine_;
    }
    
    qc_config&
    config(
        void
    ) {
        return this->conf_;
    }
    
private:
    engine_type& engine_;
    qc_config& conf_;
    
protected:
    template
    <
        typename Container
    >
    typename Container::value_type
    data(
        const Container& c
    ) {
        return *c.begin();
    }
    
    template
    <
        typename Container
    >
    Container
    reduce(
        const Container& c
    ) {
        return Container(++c.begin(), c.end());
    }
    
    /**
     * %reducible()
     * @{
     */
    template
    <
        typename Container
    >
    bool
    reducible(
        const Container& c
    ) {
        return c.size() > 1;
    }
    template
    <
        typename Container,
        typename... Args
    >
    bool
    reducible(
        const Container& c,
        const Args&... cs
    ) {
        return this->reducible(c) && this->reducible(cs...);
    }
    /**
     * @}
     */
    
};

#define BEGIN_PROPERTY_TYPE(_Name, ...)                                     \
    template                                                                \
    <                                                                       \
        typename Engine                                                     \
    >                                                                       \
    class _Name :                                                           \
        public qcxx::property<                                              \
            Engine,                                                         \
            __VA_ARGS__                                                     \
        >                                                                   \
    {                                                                       \
    public:                                                                 \
        explicit                                                            \
        _Name(                                                              \
            Engine& engine,                                                 \
            qcxx::qc_config& conf                                           \
        ) :                                                                 \
            qcxx::property<                                                 \
                Engine,                                                     \
                __VA_ARGS__                                                 \
            >(engine, conf)                                                 \
        {}

#define END_PROPERTY_TYPE                                                   \
    };

#define PROPERTY_METHOD(...)                                                \
    virtual qcxx::result                                                    \
    test(                                                                   \
        __VA_ARGS__                                                         \
    )

template
<
    template
    <
        typename
    >
    class Property,
    typename RandomEngine = std::mt19937,
    typename RandomDevice = std::random_device
>
result
quickCheckWith(
    qc_config& conf,
    std::ostream& out
) {
    typedef RandomEngine engine_type;
    typedef RandomDevice device_type;
    
    typedef Property<
        engine_type
    > property_type;
    
    device_type device;
    engine_type engine(device());
    
    property_type prop(engine, conf);
    
    return prop.go(out);
}

template
<
    template
    <
        typename
    >
    class Property,
    typename RandomEngine = std::mt19937,
    typename RandomDevice = std::random_device
>
result
quickCheck(
    void
) {
    qc_config conf;
    
    return quickCheckWith<
        Property,
        RandomEngine,
        RandomDevice
    >(conf, std::cout);
}

/* Get a random element.
 */
/**
 * %oneof()
 * @{
 */
template
<
    typename Engine,
    typename Iterator
>
Iterator
oneof(
    Engine& engine,
    Iterator begin,
    Iterator end
) {
    auto gen = get_generator<typename std::iterator_traits<
        Iterator
    >::difference_type>(engine);
    
    std::advance(begin, gen(0, std::distance(begin, end) - 1));
    
    return begin;
}
template
<
    typename Engine,
    typename Container
>
typename Container::iterator
oneof(
    Engine& engine,
    Container& container
) {
    return oneof(engine, container.begin(), container.end());
}
/**
 * @}
 */

/* Get a random weighted value.
 */
template
<
    typename Type,
    typename Engine,
    template
    <
        typename
    >
    class Container
>
Type
frequency(
    Engine& engine,
    const Container<std::pair<
        size_type,
        Type
    >>& xs
) {
    auto gen = get_generator<size_type>(engine);
    auto sum = std::accumulate(
        xs.begin(),
        xs.end(),
        static_cast<size_type>(0),
        [](
            const auto& a,
            const auto& b
        ) {
            return a + b.first;
        }
    );
    
    auto n = gen(0, sum);
    
    for (auto x : xs) {
        if (n <= x.first)
            return x.second;
    }
    
    QCXX_THROW(
        std::logic_error,
        "qcxx::frequency: failed to to pick something"
    );
}

/* Note: Unless you know exactly what you are doing it is
 * probably wise to not use QCXX_SKIP_*; they can cause large
 * amounts of headache.
 */

#ifndef QCXX_SKIP_DEFAULT_ARBITRARY_TYPES
#ifndef QCXX_SKIP_INTEGRAL_ARBITRARY_TYPES
ARBITRARY_TYPE(uniform_integral_generator, char);
ARBITRARY_TYPE(uniform_integral_generator, signed char);
ARBITRARY_TYPE(uniform_integral_generator, unsigned char);
ARBITRARY_TYPE(uniform_integral_generator, signed short int);
ARBITRARY_TYPE(uniform_integral_generator, unsigned short int);
ARBITRARY_TYPE(uniform_integral_generator, signed int);
ARBITRARY_TYPE(uniform_integral_generator, unsigned int);
ARBITRARY_TYPE(uniform_integral_generator, signed long int);
ARBITRARY_TYPE(uniform_integral_generator, unsigned long int);
ARBITRARY_TYPE(uniform_integral_generator, signed long long int);
ARBITRARY_TYPE(uniform_integral_generator, unsigned long long int);
SHRINK_TYPE(integral_minimizer, char);
SHRINK_TYPE(integral_minimizer, signed char);
SHRINK_TYPE(integral_minimizer, unsigned char);
SHRINK_TYPE(integral_minimizer, signed short int);
SHRINK_TYPE(integral_minimizer, unsigned short int);
SHRINK_TYPE(integral_minimizer, signed int);
SHRINK_TYPE(integral_minimizer, unsigned int);
SHRINK_TYPE(integral_minimizer, signed long int);
SHRINK_TYPE(integral_minimizer, unsigned long int);
SHRINK_TYPE(integral_minimizer, signed long long int);
SHRINK_TYPE(integral_minimizer, unsigned long long int);
#endif
#ifndef QCXX_SKIP_REAL_ARBITRARY_TYPES
ARBITRARY_TYPE(uniform_real_generator, float);
ARBITRARY_TYPE(uniform_real_generator, double);
SHRINK_TYPE(real_minimizer, float);
SHRINK_TYPE(real_minimizer, double);
#endif
#ifndef QCXX_SKIP_CONTAINER_ARBITRARY_TYPES
ARBITRARY_TYPE(back_container_generator, std::list<signed int>);
ARBITRARY_TYPE(back_container_generator, std::list<unsigned int>);
ARBITRARY_TYPE(back_container_generator, std::vector<signed int>);
ARBITRARY_TYPE(back_container_generator, std::vector<unsigned int>);
ARBITRARY_TYPE(back_container_generator, std::list<float>);
ARBITRARY_TYPE(back_container_generator, std::list<double>);
ARBITRARY_TYPE(back_container_generator, std::vector<float>);
ARBITRARY_TYPE(back_container_generator, std::vector<double>);
SHRINK_TYPE(container_minimizer, std::list<signed int>);
SHRINK_TYPE(container_minimizer, std::list<unsigned int>);
SHRINK_TYPE(container_minimizer, std::vector<signed int>);
SHRINK_TYPE(container_minimizer, std::vector<unsigned int>);
SHRINK_TYPE(container_minimizer, std::list<float>);
SHRINK_TYPE(container_minimizer, std::list<double>);
SHRINK_TYPE(container_minimizer, std::vector<float>);
SHRINK_TYPE(container_minimizer, std::vector<double>);
#endif
#endif

#ifndef QCXX_SKIP_DEFAULT_SHOWABLE_TYPES
SHOWABLE_TYPE(std::list<signed int>, show_container);
SHOWABLE_TYPE(std::list<unsigned int>, show_container);
SHOWABLE_TYPE(std::vector<signed int>, show_container);
SHOWABLE_TYPE(std::vector<unsigned int>, show_container);
#endif

} // qcxx

#endif

