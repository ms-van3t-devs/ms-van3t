#ifndef ASN1CPP_SETTER_HEADER_FILE
#define ASN1CPP_SETTER_HEADER_FILE

#include <type_traits>

#include "BOOLEAN.h"
#include "INTEGER.h"
#include "OCTET_STRING.h"

#include "Utils.hpp"
#include "View.hpp"

namespace asn1cpp {
    namespace Impl {
        template <typename T, typename Check = void>
        struct Setter {
            template <template <typename> class S, typename = typename std::enable_if<is_asn1_wrapper<S<T>>::value>::type>
            bool operator()(T * field, const S<T> & v) {
                if (!field) return false;

                View<T> vf(field);
                vf = v;

                return true;
            }
        };

        template <typename T>
        struct Setter<T, typename std::enable_if<std::is_fundamental<T>::value || std::is_enum<T>::value>::type> {
            bool operator()(T * field, const T & value) {
                if (!field) return false;
                *field = value;
                return true;
            }
        };

        template <>
        struct Setter<BOOLEAN_t> {
            bool operator()(BOOLEAN_t * field, bool value) {
                *field = static_cast<BOOLEAN_t>(value);
                return true;
            }
        };

        template <>
        struct Setter<INTEGER_t> {
            bool operator()(INTEGER_t * field, long value) {
                return asn_long2INTEGER(field, value) == 0;
            }
            bool operator()(INTEGER_t * field, int value) {
                return operator()(field, static_cast<long>(value));
            }
            bool operator()(INTEGER_t * field, unsigned long value) {
                return asn_ulong2INTEGER(field, value) == 0;
            }
            bool operator()(INTEGER_t * field, unsigned int value) {
                return operator()(field, static_cast<unsigned long>(value));
            }
        };

        template <>
        struct Setter<OCTET_STRING_t> {
            bool operator()(OCTET_STRING_t * field, const std::string & value) {
                return OCTET_STRING_fromBuf(field, value.data(), value.size()) == 0;
            }
            bool operator()(OCTET_STRING_t * field, const char * value) {
                return operator()(field, std::string(value));
            }
            bool operator()(OCTET_STRING_t * field, const OCTET_STRING_t * value) {
                return OCTET_STRING_fromBuf(field, reinterpret_cast<const char *>(value->buf), value->size) == 0;
            }
            bool operator()(OCTET_STRING_t * field, const unsigned int value) {
                return operator()(field, std::to_string(value));
            }
        };
    }

    template <typename F, typename V>
    bool setterField(F & field, const V & value) {
        return Impl::Setter<F>()(&field, value);
    }

    template <typename F, typename V>
    bool setterField(F *& field, const V & value) {
        if (!field) field = static_cast<F*>(calloc(1, sizeof(F)));
        return Impl::Setter<F>()(field, value);
    }

    template <typename F>
    bool clearerField(F *& field, asn_TYPE_descriptor_t * def) {
        if (field) {
            def->op->free_struct(def, field, ASFM_FREE_EVERYTHING);
            field = nullptr;
        }
        return true;
    }
}

/**
 * @page SetGetPage Setting and Getting fields with asn1cpp
 *
 * All functions described here can be found in the \ref API group.
 *
 * Note that in asn1cpp some functions are macros due to the need to
 * automatically access the asn1c type descriptors from type names. This may
 * not play nice with your autocompletion features.
 *
 * Setting Fields
 * --------------
 *
 * Once a asn1cpp::Seq or asn1cpp::View is obtained, we can set the fields of
 * the underlying struct using the `setField` function. This function accepts
 * both C++ primitive/string values for fields containing asn1c primitive
 * values, or asn1cpp wrappers for fields which correspond to nested
 * structures.
 *
 * The `setField` function returns a `boolean` that represents whether the
 * value was set correctly (most of the times it will be). Errors may happen
 * due to string conversion errors, but not for mismatched types: those just
 * won't compile.
 *
 * ```
 * auto seq = asn1cpp::makeSeq(MyAsnType);
 *
 * asn1cpp::setField(seq->someIntegerField, 15);
 * asn1cpp::setField(seq->someStringField, "mystring");
 *
 * auto nested = asn1cpp::makeSeq(MyNestedType);
 *
 * asn1cpp::setField(seq->nestedField, nested);
 * ```
 *
 * This works for both normal and optional fields, with no difference.
 * Additionally, an optional field can be cleared with the `clrField` function:
 *
 * ```
 * asn1cpp::clrField(seq->optionalField, OptionalType);
 * ```
 *
 * Note that here `OptionalType` represents the underlying asn1c type, such as
 * `INTEGER`, `BOOLEAN`, `OCTET_STRING`. This is unavoidable as we need an
 * indication of the asn1c type in order to free memory correctly. The correct
 * types for a `clrField` call can be found in the generated headers of ans1c
 * for your specific type and field.
 *
 * Getting Fields
 * --------------
 *
 * Getting specific fields' contents has a similar interface, through the
 * `getField`, `getSeq` and `getView` functions.
 *
 * The `getField` function is used to obtain a C++ value from an asn1c
 * primitive type. The `getSeq` function is used to obtain a copy of a nested
 * structure. The `getView` function is used to obtain access to a nested
 * structure without performing a copy.
 *
 * All functions require the C++ type (or your custom type for nested
 * structures) that needs to be returned to you. All functions accept an
 * additional `bool*` parameter that returns whether the value was returned
 * successfully. This needs to be checked only for optional fields.
 *
 * ```
 * // build and fill a seq variable
 *
 * auto x = asn1cpp::getField(seq->someInteger,      int);
 * auto y = asn1cpp::getField(seq->someOtherInteger, unsigned);
 * auto z = asn1cpp::getField(seq->someOtherInt,     long);
 *
 * auto str = asn1cpp::getField(seq->someString, std::string);
 *
 * bool ok;
 * auto result = asn1cpp::getField(seq->optionalInt, int, &ok);
 * if (ok)
 *     do_work(result);
 *
 * auto copy = asn1cpp::getSeq(seq->nested, MyNestedType);
 * auto view = asn1cpp::getView(seq->nested, MyNestedType);
 * ```
 */

/**
 * @def setField(field, V, ...)
 * @ingroup API
 * @brief Sets a value inside a field of an asn1cpp wrapper or element in SET OF/SEQUENCE OF.
 *
 * This macro takes a reference to the field you want to set and the value you
 * want to set inside it. It returns a boolean indicating whether the setting
 * process was performed correctly.
 *
 * This macro is also used to set elements inside SET OF or SEQUENCE OF fields.
 * For these, the macro takes the input field, the value to set and the index
 * of the item to set the value (0 based).
 *
 * This macro must be prefixed with the asn1cpp namespace to work.
 */
#define setField(field, V, ...) \
    setterField(field, V, ## __VA_ARGS__)

/**
 * @def clrField(field, V, ...)
 * @ingroup API
 * @brief Clears the value inside an optional field of an asn1cpp wrapper or the list of a SET OF/SEQUENCE OF.
 *
 * If used on an optional field, this macro takes a reference to the field you
 * want to set and the name of its asn1c type. The optional field is then
 * cleared.
 *
 * If used on a SET OF or SEQUENCE OF field, it removes all items from their
 * list. If the SET OF/SEQUENCE OF are also optional, they are too cleared. The
 * asn1c type of the elements in the list must be specified as the second
 * input.
 *
 * This macro must be prefixed with the asn1cpp namespace to work.
 */
#define clrField(field, T, ...) \
    clearerField(field, &ASN1CPP_ASN1C_DEF(T), ## __VA_ARGS__)

#endif
