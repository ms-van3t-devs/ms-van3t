/** @file */
#ifndef ASN1CPP_SEQ_HEADER_FILE
#define ASN1CPP_SEQ_HEADER_FILE

#include <stdexcept>
#include <type_traits>
#include <cstdlib>

#include "ASN1/asn_application.h"

#include "Utils.hpp"
#include "Encoding.hpp"

namespace asn1cpp {
    /**
     * @brief This class wraps and owns an asn1c non-primitive structure.
     *
     * This class is used to take ownership of asn1c structures so that they
     * can be manipulated without risking a memory leak. This class will
     * automatically clean up after itself upon destruction.
     *
     * Reading/writing the underlying asn1c structures should only be done
     * through the appropriate helper functions found in the Setter/Getter
     * files. Avoid bypassing them and directly read stuff unless you know what
     * you are doing.
     *
     * This class cannot be used to contain asn1c primitive types, such as
     * INTEGER_t, BOOLEAN_t, OCTET_STRING_t and so on. Those types can be read
     * and set through the setField() and getField() functions.
     */
    template <typename T>
    class Seq {
        public:
            /**
             * @brief Basic constructor.
             *
             * This constructor takes ownership of the input pointer, and uses
             * the input asn1c type descriptor to deallocate it upon
             * destruction. The type descriptor will also be used when encoding
             * and decoding this class.
             *
             * @param def The asn1c generated type descriptor for the input pointer.
             * @param p The asn1c struct.
             */
            Seq(asn_TYPE_descriptor_t * def, T * p);

            /**
             * @brief Null constructor.
             *
             * This constructor is used in order to produce a null Seq. This
             * can be useful to notify errors when encoding/decoding/etc.
             */
            Seq();

            /**
             * @brief New constructor.
             *
             * This constructor allocates the memory necessary to store the
             * underlying type, so that it can be used with the other library
             * functions.
             *
             * @param def The asn1c generated type descriptor for the stored type.
             */
            Seq(asn_TYPE_descriptor_t * def);

            /**
             * @brief Basic destructor.
             *
             * This destructor destroys, if present, the owned structure
             * pointer, using the type descriptor to correctly destroy it
             * through asn1c functions.
             */
            ~Seq();

            /**
             * @brief Copy constructor.
             *
             * This constructor creates a new copy of the input Seq, so that in
             * the end they will both own a separate, although equivalent,
             * copy.
             *
             * @param other The Seq to copy.
             */
            Seq(const Seq & other);

            /**
             * @brief Copy constructor from other classes.
             *
             * This constructor creates a new copy of the input so that the
             * input and this instance will compare equal afterwards.
             *
             * This constructor is able to work with any compatible asn1c
             * wrappers which fulfill the wrapper API.
             *
             * @param other The asn1c wrapper to copy.
             */
            template <template <typename> class S, typename Y,
                      typename = typename std::enable_if<are_compatible_asn1_wrappers<Seq<T>, S<Y>>::value>::type>
            Seq(const S<Y> & other);

            /**
             * @brief Assignment operator.
             *
             * This operator deallocates the currently owned asn1c structure
             * and creates a new one from scratch which copies the input.
             *
             * @param other The Seq to copy.
             *
             * @return A reference to this instance.
             */
            Seq & operator=(Seq other);

            /**
             * @brief Dereference operator.
             *
             * @return A reference to the underlying asn1c structure.
             */
            T & operator*();

            /**
             * @brief Const dereference operator.
             *
             * @return A const reference to the underlying asn1c structure.
             */
            const T & operator*() const;

            /**
             * @brief Arrow operator.
             *
             * @return A pointer to the underlying asn1c structure, can be nullptr.
             */
            T * operator->();

            /**
             * @brief Const arrow operator.
             *
             * @return A const pointer to the underlying asn1c structure, can be nullptr.
             */
            const T * operator->() const;

            /**
             * @brief Whether this instance is valid or not.
             *
             * This operator returns whether the currently held asn1c structure
             * is nullptr or not. Invalid structures will return false.
             *
             * @return True if the underlying structure is not null, false otherwise.
             */
            operator bool() const;

            /**
             * @brief Returns the underlying asn1c type descriptor.
             *
             * @return The underlying asn1c type descriptor, which can be null.
             */
            asn_TYPE_descriptor_t * getTypeDescriptor() const;

            /**
             * @brief Swaps two instances of Seq.
             *
             * @param lhs The left hand side.
             * @param rhs The right hand side.
             */
            static void swap(Seq & lhs, Seq & rhs);

        private:
            template <template <typename> class S, typename Y,
                      typename = typename std::enable_if<are_compatible_asn1_wrappers<Seq<T>, S<Y>>::value>::type>
            void deepCopy(const S<Y> & other);

            T * seq_;
            asn_TYPE_descriptor_t * def_;
    };

    /**
     * @relates Seq
     * @brief Equality operator between Seq and an asn1 wrapper.
     *
     * This function uses encoding in order to perform equality comparisons.
     * This allows us to perform equality checks independently of the wrapped
     * type.
     *
     * @param lhs The left hand side.
     * @param rhs The right hand side.
     *
     * @return True if the two instances encode to the same string, false otherwise.
     */
    template <typename T, template <typename> class S, typename Y,
              typename = typename std::enable_if<are_compatible_asn1_wrappers<Seq<T>, S<Y>>::value>::type>
    bool operator==(const Seq<T> &lhs, const S<Y> &rhs) {
        return ber::encode(lhs) == ber::encode(rhs);
    }

    /**
     * @relates Seq
     * @brief Inequality operator between Seq and an asn1 wrapper.
     *
     * \sa operator==
     *
     * @param lhs The left hand side.
     * @param rhs The right hand side.
     *
     * @return False if the two instances encode to the same string, true otherwise.
     */
    template <typename T, template <typename> class S, typename Y,
              typename = typename std::enable_if<are_compatible_asn1_wrappers<Seq<T>, S<Y>>::value>::type>
    bool operator!=(const Seq<T> &lhs, const S<Y> &rhs) {
        return !(lhs == rhs);
    }

    // Implementations...

    template <typename T>
    Seq<T>::Seq(asn_TYPE_descriptor_t * def, T * p) :
            seq_(p), def_(def)
    {
        if (seq_ && !def_)
            throw std::runtime_error("Cannot build non-empty Seq with no ASN descriptors!");
    }

    template <typename T>
    Seq<T>::Seq() : Seq(nullptr, nullptr) {}

    template <typename T>
    Seq<T>::Seq(asn_TYPE_descriptor_t * def) :
            seq_(static_cast<T*>(calloc(1, sizeof(T)))), def_(def)
    {
        if (!seq_)
            throw std::runtime_error("Allocation for Seq failed!");
    }

    template <typename T>
    Seq<T>::~Seq() {
        if (seq_)
            def_->op->free_struct(def_, seq_, ASFM_FREE_EVERYTHING);
    }

    template <typename T>
    Seq<T>::Seq(const Seq & other) {
        deepCopy(other);
    }

    template <typename T>
    template <template <typename> class S, typename Y, typename>
    Seq<T>::Seq(const S<Y> & other) {
        deepCopy(other);
    }

    template <typename T>
    Seq<T> & Seq<T>::operator=(Seq other) {
        swap(*this, other);
        return *this;
    }

    template <typename T>
    asn_TYPE_descriptor_t * Seq<T>::getTypeDescriptor() const {
        return def_;
    }

    template <typename T>
    Seq<T>::operator bool() const {
        return seq_;
    }

    template <typename T>
    T & Seq<T>::operator*() {
        if (!seq_)
            throw std::runtime_error("Cannot dereference null pointer");
        return *seq_;
    }

    template <typename T>
    const T & Seq<T>::operator*() const {
        if (!seq_)
            throw std::runtime_error("Cannot dereference null pointer");
        return *seq_;
    }

    template <typename T>
    T * Seq<T>::operator->() {
        return seq_;
    }

    template <typename T>
    const T * Seq<T>::operator->() const {
        return seq_;
    }

    template <typename T>
    void Seq<T>::swap(Seq & lhs, Seq & rhs) {
        std::swap(lhs.seq_, rhs.seq_);
        std::swap(lhs.def_, rhs.def_);
    }

    template <typename T>
    template <template <typename> class S, typename Y, typename>
    void Seq<T>::deepCopy(const S<Y> & other) {
        // Note: in here we haven't been built yet so we can't use our own
        // data. We also have to set seq_ to nullptr so that the swap happening
        // during the assignment won't cause a Seq<T> to try to delete an
        // uninitialized pointer.
        seq_ = nullptr;
        if (other && other.getTypeDescriptor()) {
            *this = ber::decode<T>(other.getTypeDescriptor(), ber::encode(other));
        } else {
            def_ = other.getTypeDescriptor();
        }
    }

    template <typename T>
    Seq<T> makeSeq(asn_TYPE_descriptor_t * def) {
        return Seq<T>(def);
    }
}

/**
 * @defgroup API
 * @brief Library public API
 *
 * This group contains all free functions and macros which should be used by
 * the user in order to correctly use the library. Note that all macros should
 * still be prefaced by the correct namespace indicator, as they still refer to
 * namespaced functions.
 *
 * In addition to the functions and macros referenced in this page, you should
 * look at the documentation for asn1cpp::Seq and asn1cpp::View, which are the
 * main ways that asn1cpp uses in order to manage memory and provide an easy
 * way to access asn1c fields.
 *
 * @def makeSeq(T)
 * @ingroup API
 * @brief Creates a Seq instance for the specified type.
 *
 * This macro allows specifying the name of the underlying asn1c type, and it
 * will automatically find the appropriate asn1c type descriptor for you.
 */
#define makeSeq(T) \
    makeSeq<T>(&ASN1CPP_ASN1C_DEF(T))

#define makeSeqV1(T) \
    makeSeq<T##V1>(&ASN1CPP_ASN1C_DEF(T##V1))
#endif
