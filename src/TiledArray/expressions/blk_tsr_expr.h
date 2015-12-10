/*
 *  This file is a part of TiledArray.
 *  Copyright (C) 2015  Virginia Tech
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Justus Calvin
 *  Department of Chemistry, Virginia Tech
 *
 *  blk_tsr_expr.h
 *  May 20, 2015
 *
 */

#ifndef TILEDARRAY_EXPRESSIONS_BLK_TSR_EXPR_H__INCLUDED
#define TILEDARRAY_EXPRESSIONS_BLK_TSR_EXPR_H__INCLUDED

#include <TiledArray/expressions/unary_expr.h>
#include "blk_tsr_engine.h"

namespace TiledArray {
  namespace expressions {

    // Forward declaration
    template <typename> class TsrExpr;
    template <typename> class BlkTsrExpr;
    template <typename, typename> class ScalBlkTsrExpr;

    template <typename A>
    using ConjBlkTsrExpr = ScalBlkTsrExpr<A, TiledArray::detail::ComplexConjugate<void> >;

    template <typename A, typename S>
    using ScalConjBlkTsrExpr = ScalBlkTsrExpr<A, TiledArray::detail::ComplexConjugate<S> >;

    using TiledArray::detail::conj_op;


    template <typename Array>
    struct ExprTrait<BlkTsrExpr<Array> > {
      typedef Array array_type; ///< The \c Array type
      typedef Array& reference; ///< \c Array reference type
      typedef BlkTsrEngine<Array> engine_type; ///< Expression engine type
      typedef typename TiledArray::detail::scalar_type<Array>::type scalar_type;
                                                          ///< Tile scalar type
    };

    template <typename Array>
    struct ExprTrait<BlkTsrExpr<const Array> > {
      typedef Array array_type; ///< The \c Array type
      typedef const Array& reference; ///< \c Array reference type
      typedef BlkTsrEngine<Array> engine_type; ///< Expression engine type
      typedef typename TiledArray::detail::scalar_type<Array>::type scalar_type;
                                                          ///< Tile scalar type
    };

    template <typename Array, typename Scalar>
    struct ExprTrait<ScalBlkTsrExpr<Array, Scalar> > {
      typedef Array array_type; ///< The \c Array type
      typedef const Array& reference; ///< \c Array reference type
      typedef ScalBlkTsrEngine<Array, Scalar> engine_type; ///< Expression engine type
      typedef Scalar scalar_type; ///< Tile scalar type
    };

    template <typename Array, typename Scalar>
    struct ExprTrait<ScalBlkTsrExpr<const Array, Scalar> > {
      typedef Array array_type; ///< The \c Array type
      typedef const Array& reference; ///< \c Array reference type
      typedef ScalBlkTsrEngine<Array, Scalar> engine_type; ///< Expression engine type
      typedef Scalar scalar_type; ///< Tile scalar type
    };

    /// Block expression

    /// \tparam Derived The derived class type
    template <typename Derived>
    class BlkTsrExprBase : public Expr<Derived> {
    public:
      typedef BlkTsrExprBase<Derived> BlkTsrExprBase_; ///< This class type
      typedef Expr<Derived> Expr_; ///< Unary base class type
      typedef typename ExprTrait<Derived>::array_type array_type;
                                                            ///< The array type
      typedef typename ExprTrait<Derived>::reference reference;
                                                  ///< The array reference type

    protected:

      reference array_; ///< The array that this expression
      std::string vars_; ///< The tensor variable list
      std::vector<std::size_t> lower_bound_; ///< Lower bound of the tile block
      std::vector<std::size_t> upper_bound_; ///< Upper bound of the tile block

      void check_valid() const {
        const unsigned int rank = array_.trange().tiles().rank();
        // Check the dimension of the lower block bound
        if(TiledArray::detail::size(lower_bound_) != rank) {
          if(World::get_default().rank() == 0) {
            TA_USER_ERROR_MESSAGE( \
                "The size lower bound of the block is not equal to rank of " \
                "the array: " \
                << "\n    array rank  = " << array_.trange().tiles().rank() \
                << "\n    lower bound = " << lower_bound_ );

            TA_EXCEPTION("The size lower bound of the block is not equal to " \
                "rank of the array.");
          }
        }

        // Check the dimension of the upper block bound
        if(TiledArray::detail::size(upper_bound_) != rank) {
          if(World::get_default().rank() == 0) {
            TA_USER_ERROR_MESSAGE( \
                "The size upper bound of the block is not equal to rank of " \
                "the array: " \
                << "\n    array rank  = " << rank \
                << "\n    upper bound = " << upper_bound_ );

            TA_EXCEPTION("The size upper bound of the block is not equal to " \
                "rank of the array.");
          }
        }

        const bool lower_bound_check =
            std::equal(std::begin(lower_bound_), std::end(lower_bound_),
                    array_.trange().tiles().lobound_data(),
                    [] (std::size_t l, std::size_t r) { return l >= r; });
        const bool upper_bound_check =
            std::equal(std::begin(upper_bound_), std::end(upper_bound_),
                    array_.trange().tiles().upbound_data(),
                    [] (std::size_t l, std::size_t r) { return l <= r; });
        if(! (lower_bound_check && upper_bound_check)) {
          if(World::get_default().rank() == 0) {
            TA_USER_ERROR_MESSAGE( \
                "The block range is not a sub-block of the array range: " \
                << "\n    array range = " << array_.trange().tiles() \
                << "\n    block range = [ " << lower_bound_  << " , " << upper_bound_ << " )");
          }

          TA_EXCEPTION("The block range is not a sub-block of the array range.");
        }

        const bool lower_upper_bound_check =
            std::equal(std::begin(lower_bound_), std::end(lower_bound_),
                    std::begin(upper_bound_),
                    [] (std::size_t l, std::size_t r) { return l < r; });
        if(! lower_upper_bound_check) {
          if(World::get_default().rank() == 0) {
            TA_USER_ERROR_MESSAGE( \
                "The block lower bound is not less than the upper bound: " \
                << "\n    lower bound = " << lower_bound_ \
                << "\n    upper bound = " << upper_bound_);
          }

          TA_EXCEPTION("The block lower bound is not less than the upper bound.");
        }
      }

    public:

      // Compiler generated functions
      BlkTsrExprBase() = delete;
      BlkTsrExprBase(const BlkTsrExprBase_&) = default;
      BlkTsrExprBase(BlkTsrExprBase_&&) = default;
      ~BlkTsrExprBase() = default;
      BlkTsrExprBase_& operator=(const BlkTsrExprBase_&) = delete;
      BlkTsrExprBase_& operator=(BlkTsrExprBase_&&) = delete;

      /// Block expression constructor

      /// \tparam Index A coordinate index type
      /// \param array The array object
      /// \param vars The array annotation variables
      /// \param lower_bound The lower bound of the tile block
      /// \param upper_bound The upper bound of the tile block
      template <typename Index>
      BlkTsrExprBase(reference array, const std::string& vars,
          const Index& lower_bound, const Index& upper_bound) :
        Expr_(), array_(array), vars_(vars),
        lower_bound_(std::begin(lower_bound), std::end(lower_bound)),
        upper_bound_(std::begin(upper_bound), std::end(upper_bound))
      {
#ifndef NDEBUG
        check_valid();
#endif // NDEBUG
      }

      /// Array accessor

      /// \return a const reference to this array
      reference array() const { return array_; }

      /// Tensor variable string accessor

      /// \return A const reference to the variable string for this tensor
      const std::string& vars() const { return vars_; }

      /// Lower bound accessor

      /// \return The block lower bound
      const std::vector<std::size_t>& lower_bound() const { return lower_bound_; }

      /// Upper bound accessor

      /// \return The block upper bound
      const std::vector<std::size_t>& upper_bound() const { return upper_bound_; }

    }; // class BlkTsrExprBase

    /// Block expression

    /// \tparam A The array type
    template <typename Array>
    class BlkTsrExpr : public BlkTsrExprBase<BlkTsrExpr<Array> > {
    public:
      typedef BlkTsrExpr<Array> BlkTsrExpr_; ///< This class type
      typedef BlkTsrExprBase<BlkTsrExpr<Array> > BlkTsrExprBase_;
                                                 ///< Block expresion base type
      typedef typename ExprTrait<BlkTsrExpr_>::engine_type engine_type;
                                                    ///< Expression engine type
      typedef typename ExprTrait<BlkTsrExpr_>::array_type array_type;
                                                            ///< The array type
      typedef typename ExprTrait<BlkTsrExpr_>::reference reference;
                                                  ///< The array reference type

      // Compiler generated functions
      BlkTsrExpr() = delete;
      BlkTsrExpr(const BlkTsrExpr_&) = default;
      BlkTsrExpr(BlkTsrExpr_&&) = default;
      ~BlkTsrExpr() = default;

      /// Block expression constructor

      /// \tparam Index A coordinate index type
      /// \param array The array object
      /// \param vars The array annotation variables
      /// \param lower_bound The lower bound of the tile block
      /// \param upper_bound The upper bound of the tile block
      template <typename Index>
      BlkTsrExpr(reference array, const std::string& vars,
          const Index& lower_bound, const Index& upper_bound) :
        BlkTsrExprBase_(array, vars, lower_bound, upper_bound)
      { }

      /// Expression assignment operator

      /// \param other The expression that will be assigned to this array
      BlkTsrExpr_& operator=(const BlkTsrExpr_& other) {
        other.eval_to(*this);
        return *this;
      }

      /// Expression assignment operator

      /// \param other The expression that will be assigned to this array
      BlkTsrExpr_& operator=(BlkTsrExpr_&& other) {
        other.eval_to(*this);
        return *this;
      }

      /// Expression assignment operator

      /// \tparam D The derived expression type
      /// \param other The expression that will be assigned to this array
      template <typename D>
      BlkTsrExpr_& operator=(const Expr<D>& other) {
        other.derived().eval_to(*this);
        return *this;
      }

      /// Expression plus-assignment operator

      /// \tparam D The derived expression type
      /// \param other The expression that will be added to this array
      template <typename D>
      BlkTsrExpr_& operator+=(const Expr<D>& other) {
        return operator=(AddExpr<BlkTsrExpr_, D>(*this, other.derived()));
      }

      /// Expression minus-assignment operator

      /// \tparam D The derived expression type
      /// \param other The expression that will be subtracted from this array
      template <typename D>
      BlkTsrExpr_& operator-=(const Expr<D>& other) {
        return operator=(SubtExpr<BlkTsrExpr_, D>(*this, other.derived()));
      }

      /// Expression multiply-assignment operator

      /// \tparam D The derived expression type
      /// \param other The expression that will scale this array
      template <typename D>
      BlkTsrExpr_& operator*=(const Expr<D>& other) {
        return operator=(MultExpr<BlkTsrExpr_, D>(*this, other.derived()));
      }


      /// Conjugated block tensor expression factory

      /// \return A conjugated block expression object
      ConjBlkTsrExpr<array_type> conj() const {
        return ConjBlkTsrExpr<array_type>(BlkTsrExprBase_::array(),
            BlkTsrExprBase_::vars(), conj_op(),
            BlkTsrExprBase_::lower_bound(), BlkTsrExprBase_::upper_bound());
      }

    }; // class BlkTsrExpr

    /// Block expression

    /// \tparam Array The array type
    template <typename Array>
    class BlkTsrExpr<const Array> : public BlkTsrExprBase<BlkTsrExpr<const Array> > {
    public:
      typedef BlkTsrExpr<const Array> BlkTsrExpr_; ///< This class type
      typedef BlkTsrExprBase<BlkTsrExpr<const Array> > BlkTsrExprBase_;
                                                 ///< Block expresion base type
      typedef typename ExprTrait<BlkTsrExpr_>::engine_type engine_type;
                                                    ///< Expression engine type
      typedef typename ExprTrait<BlkTsrExpr_>::array_type array_type;
                                                            ///< The array type
      typedef typename ExprTrait<BlkTsrExpr_>::reference reference;
                                                  ///< The array reference type

      // Compiler generated functions
      BlkTsrExpr() = delete;
      BlkTsrExpr(const BlkTsrExpr_&) = default;
      BlkTsrExpr(BlkTsrExpr_&&) = default;
      ~BlkTsrExpr() = default;
      BlkTsrExpr_& operator=(const BlkTsrExpr_&) = delete;
      BlkTsrExpr_& operator=(BlkTsrExpr_&&) = delete;


      /// Block expression constructor

      /// \tparam Index A coordinate index type
      /// \param array The array object
      /// \param vars The array annotation variables
      /// \param lower_bound The lower bound of the tile block
      /// \param upper_bound The upper bound of the tile block
      template <typename Index>
      BlkTsrExpr(reference array, const std::string& vars,
          const Index& lower_bound, const Index& upper_bound) :
        BlkTsrExprBase_(array, vars, lower_bound, upper_bound)
      { }


      /// Conjugated block tensor expression factory

      /// \return A conjugated block expression object
      ConjBlkTsrExpr<array_type> conj() const {
        return ConjBlkTsrExpr<array_type>(BlkTsrExprBase_::array(),
            BlkTsrExprBase_::vars(), conj_op(),
            BlkTsrExprBase_::lower_bound(), BlkTsrExprBase_::upper_bound());
      }

    }; // class BlkTsrExpr<const Array>

    /// Block expression

    /// \tparam Array The array type
    template <typename Array, typename Scalar>
    class ScalBlkTsrExpr : public BlkTsrExprBase<ScalBlkTsrExpr<Array, Scalar> > {
    public:
      typedef ScalBlkTsrExpr<Array, Scalar> ScalBlkTsrExpr_; ///< This class type
      typedef BlkTsrExprBase<ScalBlkTsrExpr_> BlkTsrExprBase_;
                                                 ///< Block expresion base type
      typedef typename ExprTrait<ScalBlkTsrExpr_>::engine_type engine_type;
                                                    ///< Expression engine type
      typedef typename ExprTrait<ScalBlkTsrExpr_>::array_type array_type;
                                                            ///< The array type
      typedef typename ExprTrait<ScalBlkTsrExpr_>::reference reference;
                                                  ///< The array reference type
      typedef typename ExprTrait<ScalBlkTsrExpr_>::scalar_type scalar_type;
                                                               ///< Scalar type

    private:

      scalar_type factor_; ///< The scaling factor

    public:

      // Compiler generated functions
      ScalBlkTsrExpr() = delete;
      ScalBlkTsrExpr(const ScalBlkTsrExpr_&) = default;
      ScalBlkTsrExpr(ScalBlkTsrExpr_&&) = default;
      ~ScalBlkTsrExpr() = default;
      ScalBlkTsrExpr_& operator=(const ScalBlkTsrExpr_&) = delete;
      ScalBlkTsrExpr_& operator=(ScalBlkTsrExpr_&&) = delete;


      /// Block expression constructor

      /// \tparam Index A coordinate index type
      /// \param array The array object
      /// \param vars The array annotation variables
      /// \param factor The scaling factor
      /// \param lower_bound The lower bound of the tile block
      /// \param upper_bound The upper bound of the tile block
      template <typename Index>
      ScalBlkTsrExpr(reference array, const std::string& vars,
          const scalar_type factor,
          const Index& lower_bound, const Index& upper_bound) :
        BlkTsrExprBase_(array, vars, lower_bound, upper_bound), factor_(factor)
      { }

      /// Scaling factor accessor

      /// \return The scaling factor
      scalar_type factor() const { return factor_; }

    }; // class ScalBlkTsrExpr


    using TiledArray::detail::mult_t;

    /// Scaled-block expression factor

    /// \tparam Array The array type
    /// \tparam Scalar Array scalar type
    /// \param expr The block expression object
    /// \param factor The scaling factor
    /// \return Array scaled-block expression object
    template <typename Array, typename Scalar,
        typename std::enable_if<
            TiledArray::detail::is_numeric<Scalar>::value
        >::type* = nullptr>
    inline ScalBlkTsrExpr<Array, Scalar>
    operator*(const BlkTsrExpr<Array>& expr, const Scalar& factor) {
      return ScalBlkTsrExpr<Array, Scalar>(expr.array(), expr.vars(), factor,
          expr.lower_bound(), expr.upper_bound());
    }

    /// Scaled-block expression factor

    /// \tparam Array The array type
    /// \tparam Scalar A scalar type
    /// \param factor The scaling factor
    /// \param expr The block expression object
    /// \return A scaled-block expression object
    template <typename Array, typename Scalar,
        typename std::enable_if<
            TiledArray::detail::is_numeric<Scalar>::value
        >::type* = nullptr>
    inline ScalBlkTsrExpr<Array, Scalar>
    operator*(const Scalar& factor, const BlkTsrExpr<Array>& expr) {
      return ScalBlkTsrExpr<Array, Scalar>(expr.array(), expr.vars(), factor,
          expr.lower_bound(), expr.upper_bound());
    }

    /// Scaled-block expression factor

    /// \tparam Array The array type
    /// \tparam Scalar1 A scalar factor type
    /// \tparam Scalar2 A scalar factor type
    /// \param expr The block expression object
    /// \param factor The scaling factor
    /// \return A scaled-block expression object
    template <typename Array, typename Scalar1, typename Scalar2,
        typename std::enable_if<
            TiledArray::detail::is_numeric<Scalar2>::value
        >::type* = nullptr>
    inline ScalBlkTsrExpr<Array, mult_t<Scalar1, Scalar2> >
    operator*(const ScalBlkTsrExpr<Array, Scalar1>& expr, const Scalar2& factor) {
      return ScalBlkTsrExpr<Array, mult_t<Scalar1, Scalar2> >(expr.array(),
          expr.vars(), expr.factor() * factor, expr.lower_bound(),
          expr.upper_bound());
    }

    /// Scaled-block expression factor

    /// \tparam Array The array type
    /// \tparam Scalar1 A scalar factor type
    /// \tparam Scalar2 A scalar factor type
    /// \param factor The scaling factor
    /// \param expr The block expression object
    /// \return A scaled-block expression object
    template <typename Array, typename Scalar1, typename Scalar2,
        typename std::enable_if<
            TiledArray::detail::is_numeric<Scalar1>::value
        >::type * = nullptr>
    inline ScalBlkTsrExpr<Array, mult_t<Scalar2, Scalar1> >
    operator*(const Scalar1& factor, const ScalBlkTsrExpr<Array, Scalar2>& expr) {
      return ScalBlkTsrExpr<Array, mult_t<Scalar2, Scalar1> >(expr.array(),
          expr.vars(), expr.factor() * factor, expr.lower_bound(),
          expr.upper_bound());
    }

    /// Negated block expression factor

    /// \tparam Array The array type
    /// \param expr The block expression object
    /// \return A scaled-block expression object
    template <typename Array>
    inline ScalBlkTsrExpr<Array, typename ExprTrait<BlkTsrExpr<Array> >::scalar_type>
    operator-(const BlkTsrExpr<Array>& expr) {
      return ScalBlkTsrExpr<Array,
          typename ExprTrait<BlkTsrExpr<Array> >::scalar_type>(expr.array(),
              expr.vars(), -1, expr.lower_bound(), expr.upper_bound());
    }

    /// Negated scaled-block expression factor

    /// \tparam Array The array type
    /// \tparam Scalar A scalar factor type
    /// \param expr The block expression object
    /// \return A scaled-block expression object
    template <typename Array, typename Scalar>
    inline ScalBlkTsrExpr<Array, Scalar>
    operator-(const ScalBlkTsrExpr<Array, Scalar>& expr) {
      return ScalBlkTsrExpr<Array, Scalar>(expr.array(), expr.vars(), -expr.factor(),
          expr.lower_bound(), expr.upper_bound());
    }


    /// Conjugated block tensor expression factory

    /// \tparam A A `DistArray` type
    /// \param expr The block tensor expression object
    /// \return A conjugated expression object
    template <typename A>
    inline ConjBlkTsrExpr<A> conj(const BlkTsrExpr<A>& expr) {
      return ConjBlkTsrExpr<A>(expr.array(), expr.vars(), conj_op(),
          expr.lower_bound(), expr.upper_bound());
    }

    /// Conjugated block tensor expression factory

    /// \tparam A A `DistArray` type
    /// \param expr The block tensor expression object
    /// \return A conjugated expression object
    template <typename A>
    inline ConjBlkTsrExpr<A> conj(const BlkTsrExpr<const A>& expr) {
      return ConjBlkTsrExpr<A>(expr.array(), expr.vars(), conj_op(),
          expr.lower_bound(), expr.upper_bound());
    }

    /// Conjugate-conjugate block tensor expression factory

    /// \tparam A A `DistArray` type
    /// \param expr The tensor expression object
    /// \return A tensor expression object
    template <typename A>
    inline BlkTsrExpr<const A> conj(const ConjBlkTsrExpr<A>& expr) {
      return BlkTsrExpr<const A>(expr.array(), expr.vars(),
          expr.lower_bound(), expr.upper_bound());
    }

    /// Conjugated block tensor expression factor

    /// \tparam A A `DistArray` type
    /// \tparam S A scalar type
    /// \param expr The block tensor expression object
    /// \return A conjugated expression object
    template <typename A, typename S>
    inline ScalConjBlkTsrExpr<A, S> conj(const ScalBlkTsrExpr<A, S>& expr) {
      return ScalConjBlkTsrExpr<A, S>(expr.array(), expr.vars(),
          conj_op(TiledArray::detail::conj(expr.factor())),
          expr.lower_bound(), expr.upper_bound());
    }

    /// Conjugate-conjugate tensor expression factory

    /// \tparam A A `DistArray` type
    /// \tparam S A scalar type
    /// \param expr The scaled conjugate tensor expression object
    /// \return A conjugated expression object
    template <typename A, typename S>
    inline ScalBlkTsrExpr<A, S>
    conj(const ScalConjBlkTsrExpr<A, S>& expr) {
      return ScalBlkTsrExpr<A, S>(expr.array(), expr.vars(),
          TiledArray::detail::conj(expr.factor().factor()),
          expr.lower_bound(), expr.upper_bound());
    }

    /// Scaled block tensor expression factor

    /// \tparam A A `DistArray` type
    /// \tparam Scalar A scalar type
    /// \param expr The block tensor expression object
    /// \param factor The scaling factor
    /// \return A scaled-tensor expression object
    template <typename A, typename Scalar,
        typename std::enable_if<
            TiledArray::detail::is_numeric<Scalar>::value
        >::type* = nullptr>
    inline ScalConjBlkTsrExpr<A, Scalar>
    operator*(const ConjBlkTsrExpr<const A>& expr, const Scalar& factor) {
      return ScalConjBlkTsrExpr<A, Scalar>(expr.array(), expr.vars(),
          conj_op(factor), expr.lower_bound(), expr.upper_bound());
    }

    /// Scaled block tensor expression factor

    /// \tparam A A `DistArray` type
    /// \tparam Scalar A scalar type
    /// \param factor The scaling factor
    /// \param expr The block tensor expression object
    /// \return A scaled-tensor expression object
    template <typename A, typename Scalar,
        typename std::enable_if<
            TiledArray::detail::is_numeric<Scalar>::value
        >::type* = nullptr>
    inline ScalConjBlkTsrExpr<A, Scalar>
    operator*(const Scalar& factor, const ConjBlkTsrExpr<A>& expr) {
      return ScalConjBlkTsrExpr<A, Scalar>(expr.array(), expr.vars(),
          conj_op(factor), expr.lower_bound(), expr.upper_bound());
    }

    /// Scaled block tensor expression factor

    /// \tparam A A `DistArray` type
    /// \tparam Scalar A scalar type
    /// \param expr The scaled block tensor expression object
    /// \param factor The scaling factor
    /// \return A scaled-tensor expression object
    template <typename A, typename Scalar1, typename Scalar2,
        typename std::enable_if<
            TiledArray::detail::is_numeric<Scalar2>::value
        >::type* = nullptr>
    inline ScalConjBlkTsrExpr<A, mult_t<Scalar1, Scalar2> >
    operator*(const ScalConjBlkTsrExpr<A, Scalar1>& expr, const Scalar2& factor) {
      return ScalConjBlkTsrExpr<A, mult_t<Scalar1, Scalar2> >(expr.array(),
          expr.vars(), conj_op(expr.factor().factor() * factor),
          expr.lower_bound(), expr.upper_bound());
    }

    /// Scaled-tensor expression factor

    /// \tparam A A `DistArray` type
    /// \tparam Scalar A scalar type
    /// \param factor The scaling factor
    /// \param expr The scaled block tensor expression object
    /// \return A scaled-tensor expression object
    template <typename A, typename Scalar1, typename Scalar2,
        typename std::enable_if<
            TiledArray::detail::is_numeric<Scalar1>::value
        >::type* = nullptr>
    inline ScalConjBlkTsrExpr<A, mult_t<Scalar2, Scalar1> >
    operator*(const Scalar1& factor, const ScalConjBlkTsrExpr<A, Scalar2>& expr) {
      return ScalConjBlkTsrExpr<A, mult_t<Scalar2, Scalar1> >(expr.array(),
          expr.vars(), conj_op(expr.factor().factor() * factor),
          expr.lower_bound(), expr.upper_bound());
    }

    /// Negated-conjugated-tensor expression factor

    /// \tparam A A `DistArray` type
    /// \param expr The block tensor expression object
    /// \return A scaled-tensor expression object
    template <typename A>
    inline ScalConjBlkTsrExpr<A, typename ExprTrait<ConjBlkTsrExpr<A> >::scalar_type>
    operator-(const ConjBlkTsrExpr<A>& expr) {
      return ScalConjBlkTsrExpr<A, typename ExprTrait<ConjBlkTsrExpr<A> >::scalar_type>(
          expr.array(), expr.vars(),
          conj_op<typename ExprTrait<ConjBlkTsrExpr<A> >::scalar_type>(-1),
          expr.lower_bound(), expr.upper_bound());
    }

    /// Negated-conjugated-tensor expression factor

    /// \tparam A A `DistArray` type
    /// \tparam S A scalar type
    /// \param expr The scaled-conjugated block tensor expression object
    /// \return A scaled-tensor expression object
    template <typename A, typename S>
    inline ScalConjBlkTsrExpr<A, S>
    operator-(const ScalConjBlkTsrExpr<A, S>& expr) {
      return ScalConjBlkTsrExpr<A, S>(expr.array(), expr.vars(),
          conj_op(-expr.factor().factor()));
    }


  } // namespace expressions
} // namespace TiledArray

#endif // TILEDARRAY_EXPRESSIONS_BLK_TSR_EXPR_H__INCLUDED