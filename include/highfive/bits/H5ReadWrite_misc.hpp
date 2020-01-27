/*
 *  Copyright (c) 2020 Blue Brain Project
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include "H5Utils.hpp"

namespace HighFive {

namespace details {


template <typename T>
struct BufferInfo {
    typedef typename std::remove_const<T>::type type_no_const;
    typedef typename details::type_of_array<type_no_const>::type elem_type;
    typedef typename details::type_char_array<type_no_const>::type char_array_t;
    static constexpr bool is_char_array = !std::is_same<char_array_t, void>::value;

    BufferInfo(const DataType& dtype);

    // member data which depends on the destination dataset type
    const bool is_fixed_len_string;
    const size_t n_dimensions;
    const DataType data_type;
};



// details implementation
template <typename SrcStrT>
struct string_type_checker {
    static DataType getDataType(const DataType&, bool);
};

template<>
struct string_type_checker<void> {
inline static DataType getDataType(const DataType& element_type, bool) {
    return element_type;
}};

template<std::size_t FixedLen>
struct string_type_checker<char[FixedLen]> {
inline static DataType getDataType(const DataType& element_type, bool ds_fixed_str) {
    return ds_fixed_str ? AtomicType<char[FixedLen]>() : element_type;
}};

template<>
struct string_type_checker<char*> {
inline static DataType getDataType(const DataType&, bool ds_fixed_str) {
    if (ds_fixed_str) throw DataSetException(
        "Can't output variable-length to fixed-length strings");
    return AtomicType<std::string>();
}};


template <typename T>
BufferInfo<T>::BufferInfo(const DataType& dtype)
    : is_fixed_len_string(dtype.isFixedLengthString())
    , n_dimensions(details::array_dims<type_no_const>::value
                   - ((is_fixed_len_string && is_char_array)? 1 : 0))
    , data_type(string_type_checker<char_array_t>
                ::getDataType(AtomicType<elem_type>(), is_fixed_len_string))
{
    if (is_fixed_len_string && std::is_same<elem_type, std::string>::value) {
        throw DataSetException("Can't output std::string as fixed-length.");
    }
    if (dtype.getClass() != data_type.getClass()) {
        std::cerr << "WARNING: src data and hdf5 dataset have different types: "
                  << data_type.string() << " -> " << dtype.string() << std::endl;
    }
}


}  // namespace details

}  // namespace HighFive