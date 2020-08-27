/* This file is part of PyCI.
 *
 * PyCI is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * PyCI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PyCI. If not, see <http://www.gnu.org/licenses/>. */

#pragma once

#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <fstream>
#include <future>
#include <ios>
#include <limits>
#include <new>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <SpookyV2.h>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <parallel_hashmap/phmap.h>

#include <sort_helper.h>

/* Macros to produce strings from literal macro parameters. */

#define LITERAL(S) #S
#define STRINGIZE(S) LITERAL(S)

/* PyCI version. */

#ifndef PYCI_VERSION
#define PYCI_VERSION 0.0.0
#endif

/* Seed for SpookyHash. */

#ifndef PYCI_SPOOKYHASH_SEED
#define PYCI_SPOOKYHASH_SEED 0xdeadbeefdeadbeefUL
#endif

/* Default number of threads. */

#ifndef PYCI_NUM_THREADS_DEFAULT
#define PYCI_NUM_THREADS_DEFAULT 4
#endif

namespace pyci {

/* Integer types, popcnt and ctz functions. */

typedef unsigned int uint;

typedef unsigned long ulong;

template<typename T>
inline constexpr int Size(void) {
    return std::numeric_limits<T>::digits;
}

template<typename T>
inline constexpr T Max(void) {
    return std::numeric_limits<T>::max();
}

template<typename T>
inline int Pop(const T);

template<>
inline int Pop(const unsigned t) {
    return __builtin_popcount(t);
}

template<>
inline int Pop(const unsigned long t) {
    return __builtin_popcountl(t);
}

template<typename T>
inline int Ctz(const T);

template<>
inline int Ctz(const unsigned t) {
    return __builtin_ctz(t);
}

template<>
inline int Ctz(const unsigned long t) {
    return __builtin_ctzl(t);
}

/* Hash map template type. */

template<class KeyType, class ValueType>
using HashMap = phmap::flat_hash_map<KeyType, ValueType>;

/* Pybind11 NumPy array type. */

template<typename Scalar>
using Array = pybind11::array_t<Scalar, pybind11::array::c_style | pybind11::array::forcecast>;

/* Forward-declare classes. */

struct Ham;
struct Wfn;
struct OneSpinWfn;
struct TwoSpinWfn;
struct DOCIWfn;
struct FullCIWfn;
struct GenCIWfn;
struct SparseOp;

/* Number of threads global variable. */

extern long g_number_threads;

/* PyCI routines. */

long get_num_threads(void);

void set_num_threads(const long);

long binomial(long, long);

void fill_hartreefock_det(long, ulong *);

void fill_det(const long, const long *, ulong *);

void fill_occs(const long, const ulong *, long *);

void fill_virs(const long, long, const ulong *, long *);

void next_colex(long *);

long rank_colex(const long, const long, const ulong *);

void unrank_colex(long, const long, long, long *);

long phase_single_det(const long, const long, const long, const ulong *);

long phase_double_det(const long, const long, const long, const long, const long, const ulong *);

long popcnt_det(const long, const ulong *);

long ctz_det(const long, const ulong *);

long nword_det(const long);

void excite_det(const long, const long, ulong *);

void setbit_det(const long, ulong *);

void clearbit_det(const long, ulong *);

long add_hci(const Ham &, DOCIWfn &, const double *, const double);

long add_hci(const Ham &, FullCIWfn &, const double *, const double);

long add_hci(const Ham &, GenCIWfn &, const double *, const double);

double compute_overlap(const OneSpinWfn &, const OneSpinWfn &, const double *, const double *);

double compute_overlap(const TwoSpinWfn &, const TwoSpinWfn &, const double *, const double *);

void compute_rdms(const DOCIWfn &, const double *, double *, double *);

void compute_rdms(const FullCIWfn &, const double *, double *, double *);

void compute_rdms(const GenCIWfn &, const double *, double *, double *);

double compute_enpt2(const Ham &, const DOCIWfn &, const double *, const double, const double);

double compute_enpt2(const Ham &, const FullCIWfn &, const double *, const double, const double);

double compute_enpt2(const Ham &, const GenCIWfn &, const double *, const double, const double);

/* Hamiltonian class. */

struct Ham final {
public:
    long nbasis;
    double ecore, *one_mo, *two_mo, *h, *v, *w;
    pybind11::object one_mo_array, two_mo_array, h_array, v_array, w_array;

    Ham(void);

    Ham(const Ham &);

    Ham(Ham &&) noexcept;

    Ham(const std::string &);

    Ham(const double, const Array<double>, const Array<double>);

    void to_file(const std::string &, const long, const long, const double) const;

private:
    void init_ham(const pybind11::tuple &);
};

/* Wave function classes. */

struct Wfn {
public:
    long nbasis, nocc, nocc_up, nocc_dn, nvir, nvir_up, nvir_dn;
    long ndet, nword, nword2, maxrank_up, maxrank_dn;

protected:
    std::vector<ulong> dets;
    HashMap<ulong, long> dict;

public:
    Wfn(const Wfn &);

    Wfn(Wfn &&) noexcept;

    Wfn(const long, const long, const long);

    long length(void) const;

    void squeeze(void);

protected:
    Wfn(void);

    void init(const long, const long, const long);
};

struct OneSpinWfn : public Wfn {
public:
    using Wfn::maxrank_dn;
    using Wfn::maxrank_up;
    using Wfn::nbasis;
    using Wfn::ndet;
    using Wfn::nocc;
    using Wfn::nocc_dn;
    using Wfn::nocc_up;
    using Wfn::nvir;
    using Wfn::nvir_dn;
    using Wfn::nvir_up;
    using Wfn::nword;
    using Wfn::nword2;

protected:
    using Wfn::dets;
    using Wfn::dict;

public:
    OneSpinWfn(const OneSpinWfn &);

    OneSpinWfn(OneSpinWfn &&) noexcept;

    OneSpinWfn(const std::string &);

    OneSpinWfn(const long, const long, const long);

    OneSpinWfn(const long, const long, const long, const long, const ulong *);

    OneSpinWfn(const long, const long, const long, const long, const long *);

    OneSpinWfn(const long, const long, const long, const Array<ulong>);

    OneSpinWfn(const long, const long, const long, const Array<long>);

    const ulong *det_ptr(const long) const;

    void to_file(const std::string &) const;

    void to_det_array(const long, const long, ulong *) const;

    void to_occ_array(const long, const long, long *) const;

    long index_det(const ulong *) const;

    long index_det_from_rank(const ulong) const;

    void copy_det(const long, ulong *) const;

    ulong rank_det(const ulong *) const;

    long add_det(const ulong *);

    long add_det_with_rank(const ulong *, const ulong);

    long add_det_from_occs(const long *);

    void add_hartreefock_det(void);

    void add_all_dets(void);

    void add_excited_dets(const ulong *, const long);

    void add_dets_from_wfn(const OneSpinWfn &);

    void reserve(const long);

    Array<ulong> py_getitem(const long) const;

    Array<ulong> py_to_det_array(long, long) const;

    Array<long> py_to_occ_array(long, long) const;

    long py_index_det(const Array<ulong>) const;

    ulong py_rank_det(const Array<ulong>) const;

    long py_add_det(const Array<ulong>);

    long py_add_occs(const Array<long>);

    long py_add_excited_dets(const long, const pybind11::object);
};

struct TwoSpinWfn : public Wfn {
public:
    using Wfn::maxrank_dn;
    using Wfn::maxrank_up;
    using Wfn::nbasis;
    using Wfn::ndet;
    using Wfn::nocc;
    using Wfn::nocc_dn;
    using Wfn::nocc_up;
    using Wfn::nvir;
    using Wfn::nvir_dn;
    using Wfn::nvir_up;
    using Wfn::nword;
    using Wfn::nword2;

protected:
    using Wfn::dets;
    using Wfn::dict;

public:
    TwoSpinWfn(const TwoSpinWfn &);

    TwoSpinWfn(TwoSpinWfn &&) noexcept;

    TwoSpinWfn(const std::string &);

    TwoSpinWfn(const long, const long, const long);

    TwoSpinWfn(const long, const long, const long, const long, const ulong *);

    TwoSpinWfn(const long, const long, const long, const long, const long *);

    TwoSpinWfn(const long, const long, const long, const Array<ulong>);

    TwoSpinWfn(const long, const long, const long, const Array<long>);

    const ulong *det_ptr(const long) const;

    void to_file(const std::string &) const;

    void to_det_array(const long, const long, ulong *) const;

    void to_occ_array(const long, const long, long *) const;

    long index_det(const ulong *) const;

    long index_det_from_rank(const ulong) const;

    void copy_det(const long, ulong *) const;

    ulong rank_det(const ulong *) const;

    long add_det(const ulong *);

    long add_det_with_rank(const ulong *, const ulong);

    long add_det_from_occs(const long *);

    void add_hartreefock_det(void);

    void add_all_dets(void);

    void add_excited_dets(const ulong *, const long, const long);

    void add_dets_from_wfn(const TwoSpinWfn &);

    void reserve(const long);

    Array<ulong> py_getitem(const long) const;

    Array<ulong> py_to_det_array(long, long) const;

    Array<long> py_to_occ_array(long, long) const;

    long py_index_det(const Array<ulong>) const;

    ulong py_rank_det(const Array<ulong>) const;

    long py_add_det(const Array<ulong>);

    long py_add_occs(const Array<long>);

    long py_add_excited_dets(const long, const pybind11::object);
};

struct DOCIWfn final : public OneSpinWfn {
public:
    using Wfn::maxrank_dn;
    using Wfn::maxrank_up;
    using Wfn::nbasis;
    using Wfn::ndet;
    using Wfn::nocc;
    using Wfn::nocc_dn;
    using Wfn::nocc_up;
    using Wfn::nvir;
    using Wfn::nvir_dn;
    using Wfn::nvir_up;
    using Wfn::nword;
    using Wfn::nword2;

protected:
    using Wfn::dets;
    using Wfn::dict;

public:
    DOCIWfn(const DOCIWfn &);

    DOCIWfn(DOCIWfn &&) noexcept;

    DOCIWfn(const std::string &);

    DOCIWfn(const long, const long, const long);

    DOCIWfn(const long, const long, const long, const long, const ulong *);

    DOCIWfn(const long, const long, const long, const long, const long *);

    DOCIWfn(const long, const long, const long, const Array<ulong>);

    DOCIWfn(const long, const long, const long, const Array<long>);
};

struct FullCIWfn final : public TwoSpinWfn {
public:
    using Wfn::maxrank_dn;
    using Wfn::maxrank_up;
    using Wfn::nbasis;
    using Wfn::ndet;
    using Wfn::nocc;
    using Wfn::nocc_dn;
    using Wfn::nocc_up;
    using Wfn::nvir;
    using Wfn::nvir_dn;
    using Wfn::nvir_up;
    using Wfn::nword;
    using Wfn::nword2;

protected:
    using Wfn::dets;
    using Wfn::dict;

public:
    FullCIWfn(const FullCIWfn &);

    FullCIWfn(FullCIWfn &&) noexcept;

    FullCIWfn(const DOCIWfn &);

    FullCIWfn(const std::string &);

    FullCIWfn(const long, const long, const long);

    FullCIWfn(const long, const long, const long, const long, const ulong *);

    FullCIWfn(const long, const long, const long, const long, const long *);

    FullCIWfn(const long, const long, const long, const Array<ulong>);

    FullCIWfn(const long, const long, const long, const Array<long>);
};

struct GenCIWfn final : public OneSpinWfn {
public:
    using Wfn::maxrank_dn;
    using Wfn::maxrank_up;
    using Wfn::nbasis;
    using Wfn::ndet;
    using Wfn::nocc;
    using Wfn::nocc_dn;
    using Wfn::nocc_up;
    using Wfn::nvir;
    using Wfn::nvir_dn;
    using Wfn::nvir_up;
    using Wfn::nword;
    using Wfn::nword2;

protected:
    using Wfn::dets;
    using Wfn::dict;

public:
    GenCIWfn(const GenCIWfn &);

    GenCIWfn(GenCIWfn &&) noexcept;

    GenCIWfn(const DOCIWfn &);

    GenCIWfn(const FullCIWfn &);

    GenCIWfn(const std::string &);

    GenCIWfn(const long, const long, const long);

    GenCIWfn(const long, const long, const long, const long, const ulong *);

    GenCIWfn(const long, const long, const long, const long, const long *);

    GenCIWfn(const long, const long, const long, const Array<ulong>);

    GenCIWfn(const long, const long, const long, const Array<long>);
};

/* Sparse matrix operator class. */

struct SparseOp final {
public:
    long nrow, ncol, size;
    double ecore;
    pybind11::object shape;

private:
    std::vector<double> data;
    std::vector<long> indices;
    std::vector<long> indptr;

public:
    SparseOp(const SparseOp &);

    SparseOp(SparseOp &&) noexcept;

    SparseOp(const long, const long);

    SparseOp(const Ham &, const DOCIWfn &, const long, const long);

    SparseOp(const Ham &, const FullCIWfn &, const long, const long);

    SparseOp(const Ham &, const GenCIWfn &, const long, const long);

    const double *data_ptr(const long) const;

    const long *indices_ptr(const long) const;

    const long *indptr_ptr(const long) const;

    double get_element(const long, const long) const;

    void perform_op(const double *, double *) const;

    void perform_op_cepa0(const double *, double *, const long) const;

    void perform_op_transpose_cepa0(const double *, double *, const long) const;

    void rhs_cepa0(double *, const long) const;

    Array<double> py_matvec(const Array<double>) const;

    Array<double> py_matvec_out(const Array<double>, Array<double>) const;

    Array<double> py_matvec_cepa0(const Array<double>, const long) const;

    Array<double> py_rmatvec_cepa0(const Array<double>, const long) const;

    Array<double> py_rhs_cepa0(const long) const;

private:
    template<class WfnType>
    static void init_thread(SparseOp &, const Ham &, const WfnType &, const long, const long);

    template<class WfnType>
    void init(const Ham &, const WfnType &, const long, const long);

    void init_thread_add_row(const Ham &, const DOCIWfn &, const long, ulong *, long *, long *);

    void init_thread_add_row(const Ham &, const FullCIWfn &, const long, ulong *, long *, long *);

    void init_thread_add_row(const Ham &, const GenCIWfn &, const long, ulong *, long *, long *);

    void init_thread_sort_row(const long);

    void init_thread_condense(SparseOp &, const long);
};

/* Free Python interface functions. */

long py_popcnt(const Array<ulong>);

long py_ctz(const Array<ulong>);

long py_dociwfn_add_hci(const Ham &, DOCIWfn &, const Array<double>, const double);

long py_fullciwfn_add_hci(const Ham &, FullCIWfn &, const Array<double>, const double);

long py_genciwfn_add_hci(const Ham &, GenCIWfn &, const Array<double>, const double);

double py_dociwfn_compute_overlap(const DOCIWfn &, const DOCIWfn &, const Array<double>,
                                  const Array<double>);

double py_fullciwfn_compute_overlap(const FullCIWfn &, const FullCIWfn &, const Array<double>,
                                    const Array<double>);

double py_genciwfn_compute_overlap(const GenCIWfn &, const GenCIWfn &, const Array<double>,
                                   const Array<double>);

pybind11::tuple py_dociwfn_compute_rdms(const DOCIWfn &, const Array<double>);

pybind11::tuple py_fullciwfn_compute_rdms(const FullCIWfn &, const Array<double>);

pybind11::tuple py_genciwfn_compute_rdms(const GenCIWfn &, const Array<double>);

double py_dociwfn_compute_enpt2(const Ham &, const DOCIWfn &, const Array<double>, const double,
                                const double);

double py_fullciwfn_compute_enpt2(const Ham &, const FullCIWfn &, const Array<double>, const double,
                                  const double);

double py_genciwfn_compute_enpt2(const Ham &, const GenCIWfn &, const Array<double>, const double,
                                 const double);

} // namespace pyci
