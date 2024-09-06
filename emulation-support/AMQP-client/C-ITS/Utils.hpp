#ifndef ASN1CPP_UTILS_HEADER_FILE
#define ASN1CPP_UTILS_HEADER_FILE

#include <type_traits>

#include "ASN1/asn_application.h"

// This macro is used in order to be able to access an asn1c struct definition
// just by the type name.
#define ASN1CPP_ASN1C_DEF(T) asn_DEF_##T

namespace asn1cpp {
    /**
     * @brief This struct represents whether a particular class fulfills our asn1c wrapped interface.
     *
     * This is used in order to enable template constructors from classes that
     * fulfill the asn1c wrapped interface we use in this library.
     *
     * Note that simply implementing the interface is not a guarantee that
     * everything will work right: if your interface breaks any invariants we
     * assume it will break (for example, we assume that getTypeDescriptor()
     * never returns nullptr).
     */
    template <typename W>
    struct is_asn1_wrapper {
        enum { value = false };
    };

    template <template <typename> class W, typename T>
    struct is_asn1_wrapper<W<T>>{
        private:
            template <typename Z> static constexpr auto test(int) -> decltype(

                    static_cast<asn_TYPE_descriptor_t * (Z::*)() const>                     (&Z::getTypeDescriptor),
                    static_cast<T & (Z::*)()>                                               (&Z::operator*),
                    static_cast<const T & (Z::*)() const>                                   (&Z::operator*),
                    static_cast<T * (Z::*)()>                                               (&Z::operator->),
                    static_cast<const T * (Z::*)() const>                                   (&Z::operator->),

                    bool()
            ) { return true; }

            template <typename Z> static constexpr auto test(...) -> bool
            { return false; }

        public:
            enum { value = test<W<T>>(0) };
    };

    /**
     * @brief This struct represents whether two asn1 wrappers contain the same type.
     */
    template <typename A, typename B>
    struct are_compatible_asn1_wrappers {
        enum { value = false };
    };

    template <template <typename> class W, typename T,
              template <typename> class Z, typename Y>
    struct are_compatible_asn1_wrappers<W<T>, Z<Y>> {
        public:
            enum {
                value = is_asn1_wrapper<W<T>>::value &&
                        is_asn1_wrapper<Z<Y>>::value &&
                        std::is_same<
                            typename std::remove_cv<T>::type,
                            typename std::remove_cv<Y>::type
                        >::value
            };
    };

    /**
     * @brief This struct represents whether an asn1 wrapper can be converted to another.
     *
     * This checks whether the first input both has the same type as the second
     * input, and that the first input is at least as const as the second.
     *
     * This avoids bypassing const and having non-const wrappers from const
     * wrappers.
     */
    template <typename A, typename B>
    struct is_convertible_asn1_wrapper {
        enum { value = false };
    };

    template <template <typename> class W, typename T,
              template <typename> class Z, typename Y>
    struct is_convertible_asn1_wrapper<W<T>, Z<Y>> {
        public:
            enum {
                value = is_asn1_wrapper<W<T>>::value &&
                        is_asn1_wrapper<Z<Y>>::value &&
                        (
                         std::is_same<T, Y>::value ||
                         std::is_same<
                            typename std::remove_cv<T>::type,
                            Y
                         >::value
                        )
            };
    };
}

#endif
