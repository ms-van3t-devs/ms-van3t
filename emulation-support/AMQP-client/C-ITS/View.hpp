#ifndef ASN1CPP_VIEW_HEADER_FILE
#define ASN1CPP_VIEW_HEADER_FILE

#include <stdexcept>

#include "ASN1/asn_application.h"

#include "Utils.hpp"
#include "Seq.hpp"

namespace asn1cpp {
    namespace Impl {
        template <typename T, typename Check>
        struct Setter;
    }

    /**
     * @brief This class wraps, without owning, an ans1c non-primitive structure.
     *
     * This class is used in order to edit, copy or otherwise manipulate asn1c
     * non-primitive structures through a non-owning interface. This allows
     * manipulation of nested non-primitive objects without the need to copy
     * them into a Seq first.
     *
     * Once initialized, a View cannot change where it is pointed, and all
     * operations on it will work directly on its underlying structure.
     *
     * Note that View uses a mechanism similar to iterators to enforce
     * constness. This is because copying a View simply results into another
     * View pointing to the same object. So copying a `const` View would result
     * in a non-`const` View which would be able to edit the underlying data.
     * This is thus not allowed.
     *
     * In order to create a `const` View, you must wrap a particular type in a
     * `const` way; e.g. `View<const MyType>`. This can be done by simply
     * specifying the const during View creation:
     *
     * ```
     * View<const NestedType> const_view = getView(mystruct->nestedField, NestedType);
     * ```
     *
     * or by declaring functions as
     *
     * ```
     * void myFunction(View<const NestedType> view);
     * ```
     *
     * Views are very cheap to copy since they only store pointers and do not
     * do manage them.
     *
     * This class cannot be used to contain asn1c primitive types, such as
     * INTEGER_t, BOOLEAN_t, OCTET_STRING_t and so on. Those types can be read
     * and set through the setField() and getField() functions.
     */
    template <typename T>
    class View {
        public:
            /**
             * @brief Basic constructor.
             *
             * @param def The asn1c generated type descriptor for the input pointer.
             * @param p The asn1c struct.
             */
            View(asn_TYPE_descriptor_t * def, T * p);

            /**
             * @brief Null constructor.
             *
             * This constructor is used in order to produce a null View. This
             * can be useful to notify errors when encoding/decoding/etc.
             */
            View();

            /**
             * @brief Copy constructor.
             *
             * This constructor creates a new View which will access the same
             * underlying structure as the input View. No hard copy is performed.
             *
             * @param other The View to copy.
             */
            View(View & other);

            /**
             * @brief Const copy constructor.
             *
             * **This constructor is enabled only if our wrapped type is const.**
             *
             * We prevent construction of a View from a `const` View, or
             * otherwise it would be possible to modify data which is supposed
             * to be protected with a `const` interface. If you need to copy a
             * `const` View, you can copy it to a `View<const T>`, which will
             * prevent future modifications and allow further copying.
             *
             * @param other The View to copy.
             */
            View(const View& other);

            /**
             * @brief Move constructor.
             *
             * This constructor creates a new View which will access the same
             * underlying structure as the input View. No hard copy is performed.
             *
             * @param other The View to copy.
             */
            View(View && other);

            /**
             * @brief Templated copy constructor.
             *
             * **This constructor is enabled only if our wrapped type is at
             * least as const as the input one.**
             *
             * We use this constructor to both copy from Seq, and from View
             * which may wrap a type different from ours. The
             * is_convertible_asn1_wrapper checks that we have at least the
             * same constness in our wrapped type as the input.
             *
             * This constructor simply copies the pointers to the underlying
             * struct.
             *
             * @param other The other asn1c wrapped we want to copy.
             */
            template <template <typename> class S, typename Y>
            View(S<Y> & other);

            /**
             * @brief Templated copy constructor.
             *
             * **This constructor is enabled only if our wrapped type is const.**
             *
             * We use this constructor to both copy from Seq, and from View
             * which may wrap a type different from ours. The
             * is_convertible_asn1_wrapper checks that we have at least the
             * same constness in our wrapped type as the input.
             *
             * We prevent construction of a View from a `const` object, or
             * otherwise it would be possible to modify data which is supposed
             * to be protected with a `const` interface. If you need to copy a
             * `const` View, you can copy it to a `View<const T>`, which will
             * prevent future modifications and allow further copying.
             *
             * This constructor simply copies the pointers to the underlying
             * struct.
             *
             * @param other The other asn1c wrapped we want to copy.
             */
            template <template <typename> class S, typename Y>
            View(const S<Y> & other);

            /**
             * @brief Templated move constructor.
             *
             * **This constructor is enabled only if our wrapped type is at
             * least as const as the input one.**
             *
             * We use this constructor to both copy from Seq, and from View
             * which may wrap a type different from ours. The
             * is_convertible_asn1_wrapper checks that we have at least the
             * same constness in our wrapped type as the input.
             *
             * This constructor simply copies the pointers to the underlying
             * struct.
             *
             * @param other The other asn1c wrapped we want to copy.
             */
            template <template <typename> class S, typename Y>
            View(S<Y> && other);

            /**
             * @brief Operator =
             *
             * This operator acts on the wrapped structure, and not on the View
             * itself (which cannot be "repointed" somewhere else after
             * construction).
             *
             * The data passed by the input view is completely copied, and the
             * copy is set in place of our old data, which is automatically
             * deallocated and cleaned up.
             *
             * @param other The View we want to copy.
             *
             * @return A reference to this View.
             */
            View & operator=(const View & other);

            /**
             * @brief Templated operator =
             *
             * This operator acts on the wrapped structure, and not on the View
             * itself (which cannot be "repointed" somewhere else after
             * construction).
             *
             * The data passed by the input is completely copied, and the copy
             * is set in place of our old data, which is automatically
             * deallocated and cleaned up.
             *
             * @param other The View we want to copy.
             *
             * @return A reference to this View.
             */
            template <template <typename> class S, typename Y>
            View & operator=(const S<Y> & other);

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
             * @brief Templated swap.
             *
             * This function swaps the contents of the input and this View's
             * structures, so that both them and all internal pointers are
             * swapped.
             *
             * @param lhs The left hand side.
             * @param rhs The right hand side.
             */
            template <template <typename> class S, typename Y>
            static void swap(View & lhs, S<Y> & rhs);

            /**
             * @brief Templated swap.
             *
             * \sa swap(View &, S<Y> &)
             */
            template <template <typename> class S, typename Y>
            static void swap(S<Y> & lhs, View & rhs);

        private:
            /**
             * @brief Private constructor.
             *
             * This constructor is used by the internals in order to keep
             * memory clean. It's not public since it does not get the asn1c
             * type descriptor, and this does not allow for encoding/decoding.
             *
             * @param p A pointer to the underlying asn1c struct.
             */
            View(T * p);

            template <typename TT, typename Check>
            friend struct Impl::Setter;

            T * seq_;
            asn_TYPE_descriptor_t * def_;
    };

    template <typename T>
    View<T>::View(asn_TYPE_descriptor_t * def, T * p) :
            seq_(p), def_(def)
    {
        if (!seq_)
            throw std::runtime_error("Cannot build empty View!");
        if (!def_)
            throw std::runtime_error("Cannot build View with no ASN descriptors!");
    }

    template <typename T>
    View<T>::View(View & other) : View(other.def_, other.seq_) {}

    template <typename T>
    View<T>::View(const View& other) : View(other.def_, other.seq_) {
        static_assert(std::is_const<T>::value, "Cannot copy const View if our wrapped type is not const!");
    }

    template <typename T>
    View<T>::View(View && other) : View(other.def_, other.seq_) {}

    template <typename T>
    template <template <typename> class S, typename Y>
    View<T>::View(S<Y> & other) : View(other.getTypeDescriptor(), &(*other)) {
        static_assert(is_convertible_asn1_wrapper<View<T>, S<Y>>::value, "Cannot copy unconvertible asn1 wrapper");
    }

    template <typename T>
    template <template <typename> class S, typename Y>
    View<T>::View(const S<Y> & other) : View(other.getTypeDescriptor(), &(*other)) {
        static_assert(is_convertible_asn1_wrapper<View<T>, S<Y>>::value, "Cannot copy unconvertible asn1 wrapper");
        static_assert(std::is_const<T>::value, "Cannot copy const asn1 wrapper if our wrapped type is not const!");
    }

    template <typename T>
    template <template <typename> class S, typename Y>
    View<T>::View(S<Y> && other) : View(other.getTypeDescriptor(), &(*other)) {
        static_assert(is_convertible_asn1_wrapper<View<T>, S<Y>>::value, "Cannot copy unconvertible asn1 wrapper");
    }

    template <typename T>
    View<T>::View() : seq_(nullptr), def_(nullptr) {}

    template <typename T>
    View<T>::View(T * seq) : seq_(seq), def_(nullptr) {}

    template <typename T>
    asn_TYPE_descriptor_t * View<T>::getTypeDescriptor() const {
        return def_;
    }

    template <typename T>
    View<T>::operator bool() const {
        return seq_;
    }

    template <typename T>
    T & View<T>::operator*() {
        if (!seq_)
            throw std::runtime_error("Cannot dereference null pointer");
        return *seq_;
    }

    template <typename T>
    const T & View<T>::operator*() const {
        if (!seq_)
            throw std::runtime_error("Cannot dereference null pointer");
        return *seq_;
    }

    template <typename T>
    T * View<T>::operator->() {
        return seq_;
    }

    template <typename T>
    const T * View<T>::operator->() const {
        return seq_;
    }

    template <typename T>
    View<T> & View<T>::operator=(const View & other) {
        Seq<T> copy = other;
        swap(*this, copy);
        return *this;
    }

    template <typename T>
    template <template <typename> class S, typename Y>
    View<T> & View<T>::operator=(const S<Y> & other) {
        static_assert(are_compatible_asn1_wrappers<View<T>, S<Y>>::value, "Cannot assign from an uncompatible asn1 wrapper");
        Seq<T> copy = other;
        swap(*this, copy);
        return *this;
    }

    template <typename T>
    template <template <typename> class S, typename Y>
    void View<T>::swap(View & lhs, S<Y> & rhs) {
        static_assert(are_compatible_asn1_wrappers<View<T>, S<Y>>::value, "Cannot swap from an uncompatible asn1 wrapper");
        if (!lhs && !rhs) return;
        if (!(lhs && rhs))
            throw std::runtime_error("Cannot swap null pointers!");
        T tmp = *rhs;
        *rhs = *lhs;
        *lhs = tmp;
    }

    template <typename T>
    template <template <typename> class S, typename Y>
    void View<T>::swap(S<Y> & lhs, View & rhs) {
        swap(rhs, lhs);
    }
}

#endif
