#ifndef ASN1CPP_SETOF_HEADER_FILE
#define ASN1CPP_SETOF_HEADER_FILE

#include "Setter.hpp"
#include "Getter.hpp"
#include "Seq.hpp"
#include "View.hpp"

namespace asn1cpp {
    namespace setof {
        namespace Impl {
            template <typename T>
            struct ArrayType {
                using type = typename std::remove_reference<decltype(**std::declval<T>().list.array)>::type;
            };
            template <typename T>
            struct ArrayType<T*> {
                using type = typename std::remove_reference<decltype(**std::declval<T>().list.array)>::type;
            };
        }

        template <typename T>
        int getSize(const T& field) {
            return field.list.count;
        }

        template <typename T>
        int getSize(const T * const & field) {
            if (!field) return -1;
            return getSize(*field);
        }

        template <typename T>
        int getSize(T * const & field) {
            return getSize(const_cast<const T*>(field));
        }

        template <typename R, typename T>
        R getterField(const T& field, int id, bool *ok = nullptr) {
            if (id < 0 || id >= getSize(field)) {
                if (ok) *ok = false;
                return R();
            }
            bool iok;
            return asn1cpp::getterField<R, typename Impl::ArrayType<T>::type>(*field.list.array[id], ok ? ok : &iok);
        }

        template <typename R, typename T>
        R getterField(const T* const & field, int id, bool *ok = nullptr) {
            if (!field) {
                if (ok) *ok = false;
                return R();
            }
            return getterField<R>(*field, id, ok);
        }

        template <typename R, typename T>
        R getterField(T* const & field, int id, bool *ok = nullptr) {
            return getterField<R>(const_cast<const T*>(field), id, ok);
        }

        // ### GETTER SEQ ###

        template <typename T>
        Seq<typename Impl::ArrayType<T>::type> getterSeq(const T& field, asn_TYPE_descriptor_t * def, int id, bool *ok = nullptr) {
            using R = typename Impl::ArrayType<T>::type;
            if (id < 0 || id >= getSize(field)) {
                if (ok) *ok = false;
                return Seq<R>();
            }
            bool iok;
            return asn1cpp::getterSeq(*field.list.array[id], def, ok ? ok : &iok);
        }

        template <typename T>
        Seq<typename Impl::ArrayType<T>::type> getterSeq(const T * const & field, asn_TYPE_descriptor_t * def, int id, bool *ok = nullptr) {
            using R = typename Impl::ArrayType<T>::type;
            if (!field) {
                if (ok) *ok = false;
                return Seq<R>();
            }
            return getterSeq(*field, def, id, ok);
        }

        template <typename T>
        Seq<typename Impl::ArrayType<T>::type> getterSeq(T * const & field, asn_TYPE_descriptor_t * def, int id, bool *ok = nullptr) {
            return getterSeq(const_cast<const T*>(field), def, id, ok);
        }

        // ### GETTER VIEW CONST ###

        template <typename T>
        View<const typename Impl::ArrayType<T>::type> getterView(const T& field, asn_TYPE_descriptor_t * def, int id, bool *ok = nullptr) {
            using R = const typename Impl::ArrayType<T>::type;
            if (id < 0 || id >= getSize(field)) {
                if (ok) *ok = false;
                return View<R>();
            }
            bool iok;
            return asn1cpp::getterView(*field.list.array[id], def, ok ? ok : &iok);
        }

        template <typename T>
        View<const typename Impl::ArrayType<T>::type> getterView(const T * const & field, asn_TYPE_descriptor_t * def, int id, bool *ok = nullptr) {
            using R = typename Impl::ArrayType<T>::type;
            if (!field) {
                if (ok) *ok = false;
                return View<R>();
            }
            return getterView(*field, def, id, ok);
        }

        // ### GETTER VIEW NON-CONST ###

        template <typename T>
        View<typename Impl::ArrayType<T>::type> getterView(T& field, asn_TYPE_descriptor_t * def, int id, bool *ok = nullptr) {
            using R = typename Impl::ArrayType<T>::type;
            if (id < 0 || id >= getSize(field)) {
                if (ok) *ok = false;
                return View<R>();
            }
            bool iok;
            return asn1cpp::getterView(*field.list.array[id], def, ok ? ok : &iok);
        }

        template <typename T>
        View<typename Impl::ArrayType<T>::type> getterView(T *& field, asn_TYPE_descriptor_t * def, int id, bool *ok = nullptr) {
            using R = typename Impl::ArrayType<T>::type;
            if (!field) {
                if (ok) *ok = false;
                return View<R>();
            }
            return getterView(*field, def, id, ok);
        }

        template <typename T, typename V>
        bool setterField(T & field, const V & value, int id) {
            if (id < 0 || id >= getSize(field))
                return false;
            return asn1cpp::setterField(field.list.array[id], value);
        }

        template <typename T, typename V>
        bool setterField(T *& field, const V & value, int id) {
            if (!field)
                return false;
            return setterField(*field, value, id);
        }

        template <typename T, typename V>
        bool adderElement(T & field, const V & value) {
            typename Impl::ArrayType<T>::type * ptr = nullptr;
            if (!asn1cpp::setterField(ptr, value))
                return false;
            return asn_set_add(&field, ptr) == 0;
        }

        template <typename T, typename V>
        bool adderElement(T *& field, const V & value) {
            if (!field) field = static_cast<T*>(calloc(1, sizeof(T)));
            return adderElement(*field, value);
        }

        template <typename T>
        bool removerElement(T & field, int id, asn_TYPE_descriptor_t * def) {
            if (id < 0 || id >= getSize(field))
                return false;

            auto p = field.list.array[id];
            asn_set_del(&field, id, 0);
            def->op->free_struct(def, p, ASFM_FREE_EVERYTHING);
            return true;
        }

        template <typename T, typename V>
        bool removerElement(T *& field, const V & value) {
            if (!field) return false;
            return removerElement(*field, value);
        }

        template <typename T>
        void clearerField(T & field, asn_TYPE_descriptor_t * def) {
            for (int i = 0; i < getSize(field); ++i)
                def->op->free_struct(def, field.list.array[i], ASFM_FREE_EVERYTHING);
            field.list.count = 0;
        }

        template <typename T>
        void clearerField(T *& field, asn_TYPE_descriptor_t * def) {
            if (!field) return;
            for (int i = 0; i < getSize(field); ++i)
                def->op->free_struct(def, field->list.array[i], ASFM_FREE_EVERYTHING);

            asn_set_empty(field);
            free(field);

            field = nullptr;
        }
    }
}

/**
 * @def pushList(field, V, ...)
 * @ingroup API
 * @brief Inserts an element at the end of the input list for SET OF/SEQUENCE OF.
 *
 * Inserts at the end of the specified field a new element. The second input is
 * the value of the new element. A boolean is returned to report whether the
 * operation was successful.
 *
 * This macro must be prefixed with both the asn1cpp namespace and the
 * namespace of the underlying list type (setof, sequenceof).
 */
#define pushList(field, V, ...) \
    adderElement(field, V, ## __VA_ARGS__)

/**
 * @def popList(field, T, id, ...)
 * @ingroup API
 * @brief Removes an element from the input list for SET OF/SEQUENCE OF.
 *
 * This macro takes an input SET OF or SEQUENCE OF field, the asn1c type of the
 * elements of the list and an integer id. The specified element is removed
 * from the list. A boolean value is returned to report whether the operation
 * was successful.
 *
 * This macro must be prefixed with both the asn1cpp namespace and the
 * namespace of the underlying list type (setof, sequenceof).
 */
#define popList(field, T, id, ...) \
    removerElement(field, id, &ASN1CPP_ASN1C_DEF(T), ## __VA_ARGS__)

#endif
