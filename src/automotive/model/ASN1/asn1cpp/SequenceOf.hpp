#ifndef ASN1CPP_SEQUENCEOF_HEADER_FILE
#define ASN1CPP_SEQUENCEOF_HEADER_FILE

#include "SetOf.hpp"

namespace asn1cpp {
    namespace sequenceof {
        template <typename T>
        int getSize(const T& field) {
            return setof::getSize(field);
        }

        template <typename R, typename T>
        R getterField(const T& field, int id, bool *ok = nullptr) {
            return setof::getterField<R>(field, id, ok);
        }

        template <typename T>
        Seq<typename setof::Impl::ArrayType<T>::type> getterSeq(const T& field, asn_TYPE_descriptor_t * def, int id, bool *ok = nullptr) {
            return setof::getterSeq(field, def, id, ok);
        }

        template <typename T>
        View<const typename setof::Impl::ArrayType<T>::type> getterView(const T& field, asn_TYPE_descriptor_t * def, int id, bool *ok = nullptr) {
            return setof::getterView(field, def, id, ok);
        }

        template <typename T>
        View<typename setof::Impl::ArrayType<T>::type> getterView(T& field, asn_TYPE_descriptor_t * def, int id, bool *ok = nullptr) {
            return setof::getterView(field, def, id, ok);
        }

        template <typename T, typename V>
        bool setterField(T & field, const V & value, int id) {
            return setof::setterField(field, value, id);
        }

        template <typename T, typename V>
        bool adderElement(T & field, const V & value) {
            return setof::adderElement(field, value);
        }

        template <typename T>
        bool removerElement(T & field, int id, asn_TYPE_descriptor_t * def) {
            if (id < 0 || id >= getSize(field))
                return false;

            auto p = field.list.array[id];
            // Note that here we use the sequence del, not the set
            asn_sequence_del(&field, id, 0);
            def->op->free_struct(def, p, ASFM_FREE_EVERYTHING);
            return true;
        }

        template <typename T>
        bool removerElement(T *& field, int id, asn_TYPE_descriptor_t * def) {
            if (!field) return false;

            return removerElement(*field, id, def);
        }

        template <typename T>
        void clearerField(T & field, asn_TYPE_descriptor_t * def) {
            setof::clearerField(field, def);
        }
    }
}

#endif
