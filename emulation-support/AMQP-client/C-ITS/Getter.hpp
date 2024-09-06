#ifndef ASN1CPP_GETTER_HEADER_FILE
#define ASN1CPP_GETTER_HEADER_FILE

#include <cerrno>
#include <string>
#include <type_traits>

#include "ASN1/BOOLEAN.h"
#include "ASN1/INTEGER.h"
#include "ASN1/OCTET_STRING.h"

#include "Seq.hpp"
#include "View.hpp"

namespace asn1cpp {
    namespace Impl {
        template <typename R, typename T, typename Check = void>
        struct Getter {
            static_assert(!std::is_same<T, T>::value, "Getter not implemented for this type!");
        };

        template <>
        struct Getter<bool, BOOLEAN_t> {
            bool operator()(const BOOLEAN_t * field, bool & ok) {
                ok = true;
                return !!*field;
            }
        };

        template <>
        struct Getter<long, INTEGER_t> {
            long operator()(const INTEGER_t * field, bool & ok) {
                long retval;
                if (asn_INTEGER2long(field, &retval) == 0) {
                    ok = true;
                    return retval;
                } else {
                    ok = (errno == EINVAL);
                    return 0;
                }
            }
        };

        template <>
        struct Getter<unsigned long, INTEGER_t> {
            unsigned long operator()(const INTEGER_t * field, bool & ok) {
                unsigned long retval;
                if (asn_INTEGER2ulong(field, &retval) == 0) {
                    ok = true;
                    return retval;
                } else {
                    ok = (errno == EINVAL);
                    return 0;
                }
            }
        };

        template <typename R>
        struct Getter<R, INTEGER_t, typename std::enable_if<std::is_integral<R>::value && std::is_signed<R>::value>::type> {
            R operator()(const INTEGER_t * field, bool & ok) {
                return static_cast<R>(Getter<long, INTEGER_t>()(field, ok));
            }
        };

        template <typename R>
        struct Getter<R, INTEGER_t, typename std::enable_if<std::is_integral<R>::value && std::is_unsigned<R>::value>::type> {
            R operator()(const INTEGER_t * field, bool & ok) {
                return static_cast<R>(Getter<unsigned long, INTEGER_t>()(field, ok));
            }
        };

        template <typename R>
        struct Getter<R, long, typename std::enable_if<std::is_arithmetic<R>::value>::type> {
            R operator()(const long * field, bool & ok) {
                ok = true;
                return static_cast<R>(*field);
            }
        };

        template <typename R>
        struct Getter<R, unsigned long, typename std::enable_if<std::is_arithmetic<R>::value>::type> {
            R operator()(const unsigned long * field, bool & ok) {
                ok = true;
                return static_cast<R>(*field);
            }
        };

        template <>
        struct Getter<std::string, OCTET_STRING_t> {
            std::string operator()(const OCTET_STRING_t * field, bool & ok) {
                ok = true;
                return std::string((const char *)field->buf, field->size);
            }
        };

        template <typename R, typename T>
        struct Getter<R, T, typename std::enable_if<std::is_enum<R>::value && std::is_enum<T>::value>::type> {
            R operator()(const T * field, bool & ok) {
                ok = true;
                return static_cast<R>(*field);
            }
        };

        template <typename R, typename T>
        struct Getter<R, T, typename std::enable_if<std::is_enum<R>::value && !std::is_enum<T>::value>::type> {
            R operator()(const T * field, bool & ok) {
                return static_cast<R>(Getter<int, T>()(field, ok));
            }
        };

        template <typename T>
        struct Getter<Seq<T>, T> {
            Seq<T> operator()(const T * field, asn_TYPE_descriptor_t * def, bool & ok) {
                ok = true;
                return Seq<T>(View<const T>(def, field));
            }
        };

        template <typename T>
        struct Getter<View<T>, T> {
            View<T> operator()(T * field, asn_TYPE_descriptor_t * def, bool & ok) {
                ok = true;
                return View<T>(def, field);
            }
        };
    }

    template <typename R, typename T>
    R getterField(const T & field, bool * ok = nullptr) {
        bool iok;
        return Impl::Getter<R, T>()(&field, ok ? *ok : iok);
    }

    template <typename R, typename T>
    R getterField(const T * const & field, bool * ok = nullptr) {
        if (field) {
            bool iok;
            return Impl::Getter<R, T>()(field, ok ? *ok : iok);
        } else {
            if (ok) *ok = false;
            return R();
        }
    }

    template <typename R, typename T>
    R getterField(T * const & field, bool * ok = nullptr) {
        return getterField<R>(const_cast<const T *>(field), ok);
    }

    template <typename T>
    Seq<T> getterSeq(const T & field, asn_TYPE_descriptor_t * def, bool * ok = nullptr) {
        bool iok;
        return Impl::Getter<Seq<T>, T>()(&field, def, ok ? *ok : iok);
    }

    template <typename T>
    Seq<T> getterSeqOpt(const T * field, asn_TYPE_descriptor_t * def, bool * ok = nullptr) {
        if(field==NULL && ok!=NULL) {
            *ok=false;
            return Seq<T>();
        } else {
            return getterSeq(*field,def,ok);
        }
    }

    template <typename T>
    View<T> getterView(T & field, asn_TYPE_descriptor_t * def, bool * ok = nullptr) {
        bool iok;
        return Impl::Getter<View<T>, T>()(&field, def, ok ? *ok : iok);
    }

    template <typename T>
    View<const T> getterView(const T & field, asn_TYPE_descriptor_t * def, bool * ok = nullptr) {
        bool iok;
        return Impl::Getter<View<const T>, const T>()(&field, def, ok ? *ok : iok);
    }
}

// For user documentation about the getter function, see the Setter.hpp file.

/**
 * @def getField(field, R, ...)
 * @ingroup API
 * @brief Gets a value from a field of an asn1cpp wrapper or element in SET OF/SEQUENCE OF.
 *
 * This macro must only be used for asn1c primitive types, not types you have
 * defined.
 *
 * This macro takes a reference to the field you want to set and the C++ type
 * name you want to extract from it. It accepts an optional boolean pointer
 * parameter indicating whether the extraction process was successful, which is
 * useful for optional fields.
 *
 * This macro is also used to get elements inside SET OF or SEQUENCE OF fields.
 * For these, the macro takes the input field, the C++ type name you want to
 * extract and the index of the item to get the value from (0 based). Once
 * again a boolean pointer as last parameter is available although it is not
 * that useful.
 *
 * This macro must be prefixed with the asn1cpp namespace to work.
 */
#define getField(field, R, ...) \
    getterField<R>(field, ## __VA_ARGS__)

/**
 * @def getSeq(field, R, ...)
 * @ingroup API
 * @brief Copies and wraps a non-primitive value from a field of an asn1cpp wrapper or element in SET OF/SEQUENCE OF.
 *
 * This macro must only be used for asn1c types you have defined, and not on
 * asn1c primitive types.
 *
 * This macro copies the underlying field into a separate, memory-managed
 * asn1cpp::Seq.
 *
 * This macro API is the same as the #getField macro.
 */
#define getSeq(field, R, ...) \
    getterSeq(field, &ASN1CPP_ASN1C_DEF(R), ## __VA_ARGS__)

#define getSeqOpt(field, R, ...) \
    getterSeqOpt(field, &ASN1CPP_ASN1C_DEF(R), ## __VA_ARGS__)

/**
 * @def getView(field, R, ...)
 * @ingroup API
 * @brief Wraps a non-primitive value from a field of an asn1cpp wrapper or element in SET OF/SEQUENCE OF.
 *
 * This macro must only be used for asn1c types you have defined, and not on
 * asn1c primitive types.
 *
 * This macro provides a asn1cpp::View on the underlying field.
 *
 * This macro API is the same as the #getField macro.
 */
#define getView(field, R, ...) \
    getterView(field, &ASN1CPP_ASN1C_DEF(R), ## __VA_ARGS__)

#endif
