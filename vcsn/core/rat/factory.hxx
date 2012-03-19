#ifndef VCSN_CORE_RAT_RAT_EXP_HXX_
# define VCSN_CORE_RAT_RAT_EXP_HXX_

# include <algorithm>
# include <cassert>

# include <misc/cast.hh>
# include <core/rat/factory.hh>
# include <core/rat/node.hh>

namespace vcsn
{
  namespace rat
  {

    template <class WeightSet>
    factory<WeightSet>::factory()
      : ws_(0)
    {}

    template <class WeightSet>
    factory<WeightSet>::factory(const WeightSet& ws)
      : ws_(&ws)
    {}

    template <class WeightSet>
    inline
    exp*
    factory<WeightSet>::op_mul(exp* e)
    {
      return op_mul(down_cast<node_t*>(e));
    }

    template <class WeightSet>
    inline
    exp*
    factory<WeightSet>::op_mul(exp* l, exp* r)
    {
      auto left = down_cast<node_t*>(l);
      auto right = down_cast<node_t*>(r);
      return op_mul(left, right);
    }

    template <class WeightSet>
    inline
    exp*
    factory<WeightSet>::op_add(exp* l, exp* r)
    {
      auto left = down_cast<node_t*>(l);
      auto right = down_cast<node_t*>(r);
      return op_add(left, right);
    }

    template <class WeightSet>
    inline
    exp*
    factory<WeightSet>::op_kleene(exp* e)
    {
      return op_kleene(down_cast<node_t*>(e));
    }

    template <class WeightSet>
    inline
    auto
    factory<WeightSet>::op_mul(node_t* e)
      -> node_t*
    {
      return e;

      // if (concat<WeightSet>::CONCAT == e->type())
      // {
      auto res = down_cast<concat_t*>(e);
      //   assert(res);
      //   return res;
      // }
      // else
      // {
      //   concat<WeightSet>*res = new concat<WeightSet>();
      //   res->push_front(e);
      //   return res;
      // }
    }

    template <class WeightSet>
    inline
    auto
    factory<WeightSet>::op_mul(node_t* l, node_t* r)
      -> node_t*
    {
      // Trivial Identity
      // E.0 = 0.E = 0
      // E.1 = 1.E = E
      if (node_t::ZERO == l->type() || node_t::ONE == r->type())
        {
          delete r;
          return l;
        }
      if (node_t::ZERO == r->type() || node_t::ONE == l->type())
        {
          delete l;
          return r;
        }
      // END: Trivial Identity

      if (node_t::CONCAT == l->type())
        {
          auto left = down_cast<concat_t*>(l);
          if (node_t::CONCAT == r->type())
            {
              auto right = down_cast<concat_t*>(r);
              for (auto i : *right)
                left->push_back(i);
              // FIXME: Leak right
              return left;
            }
          else
            {
              left->push_back(r);
              return left;
            }
        }
      else if (node_t::CONCAT == r->type())
        {
          auto right = down_cast<concat_t*>(r);
          right->push_front(l);
          return right;
        }
      else
        {
          auto res = new concat_t();
          res->push_back(l);
          res->push_back(r);
          return res;
        }
    }

    template <class WeightSet>
    inline
    auto
    factory<WeightSet>::op_add(node_t* l, node_t* r)
      -> node_t*
    {
      // Trivial Identity
      // E+0 = 0+E = E
      if (node_t::ZERO == l->type())
        {
          delete l;
          return r;
        }
      if (node_t::ZERO == r->type())
        {
          delete r;
          return l;
        }
      // END: Trivial Identity

      if (node_t::PLUS == l->type())
        {
          auto res = down_cast<plus_t*>(l);
          if (node_t::PLUS == r->type())
            {
              auto right = down_cast<plus_t*>(r);
              for (auto it : *right)
                res->push_back(it);
            }
          else
            {
              res->push_back(r);
            }
          return res;
        }
      else if (node_t::PLUS == r->type())
        {
          auto res = down_cast<plus_t*>(r);
          res->push_front(l);
          return res;
        }
      else
        {
          plus_t* res = new plus_t();
          res->push_front(r);
          res->push_front(l);
          return res;
        }
    }

    template <class WeightSet>
    inline
    auto
    factory<WeightSet>::op_kleene(node_t* e)
      -> node_t*
    {
      if (node_t::ZERO == e->type())
        {
          // Trivial identity
          // (0)* == 1
          delete e;
          return new one_t;
        }
      else
        return new kleene_t(e);
    }

    template <class WeightSet>
    inline
    auto
    factory<WeightSet>::op_one()
      -> one_t*
    {
      return new one_t();
    }

    template <class WeightSet>
    inline
    auto
    factory<WeightSet>::op_zero()
      -> zero_t*
    {
      return new zero_t();
    }

    template <class WeightSet>
    inline
    auto
    factory<WeightSet>::op_word(std::string* w)
      -> word_t*
    {
      if (ws_ != nullptr)
        return new word_t(w, *ws_);
      else
        return new word_t(w);
    }

    template <class WeightSet>
    inline
    auto
    factory<WeightSet>::op_weight(std::string* w)
      -> weight_str_container*
    {
      weight_str_container* res = new weight_str_container();
      res->push_front(w);
      return res;
    }

    template <class WeightSet>
    exp*
    factory<WeightSet>::op_weight(weight_str_container* w, exp* e)
    {
      if (!w)
        return e;
      // if w
      auto expr = down_cast<node_t*>(e);
      switch (expr->weight_type())
        {
        case node_t::L_WEIGHT:
          {
            auto lweight = down_cast<left_weighted_t*>(expr);
            return op_weight(lweight, w);
          }

        case node_t::LR_WEIGHT:
          {
            auto lrweight = down_cast<weighted_t*>(expr);
            return op_weight(w, lrweight);
          }

        default:
          assert(false);
          return 0;
        }
    }

    template <class WeightSet>
    exp*
    factory<WeightSet>::op_weight(exp* e, weight_str_container* w)
    {
      if (!w)
        return e;
      // if w
      auto expr = down_cast<node_t*>(e);
      switch (expr->weight_type())
        {
        case node_t::L_WEIGHT:
          {
            auto rweight = down_cast<left_weighted_t*>(expr);
            return op_weight(rweight, w);
          }
        case node_t::LR_WEIGHT:
          {
            auto rweight = down_cast<weighted_t*>(expr);
            return op_weight(rweight, w);
          }
        default:
          assert(false);
          return 0;
        }
    }

    template<class WeightSet>
    auto
    factory<WeightSet>::op_weight(left_weighted_t* e,
                                  weight_str_container* w)
      -> node_t*
    {
      for (auto i : *w)
        {
          weight_t new_weight = ws_->op_conv(*i);
          if (ws_->is_zero(new_weight))
            {
              delete e;
              return new zero_t();
            }
          e->left_weight() = ws_->mul(e->left_weight(), new_weight);
        }
      return e;
    }

    template<class WeightSet>
    auto
    factory<WeightSet>::op_weight(weight_str_container* w,
                                  weighted_t* e)
      -> node_t*
    {
      for (auto i : *w)
        {
          weight_t new_weight = ws_->op_conv(*i);
          if (ws_->is_zero(new_weight))
            {
              delete e;
              return new zero_t();
            }
          e->left_weight() = ws_->mul(e->left_weight(), new_weight);
        }
      return e;
    }

    template<class WeightSet>
    auto
    factory<WeightSet>::op_weight(weighted_t* e,
                                  weight_str_container* w)
      -> node_t*
    {
      for (auto i : *w)
        {
          weight_t new_weight = ws_->op_conv(*i);
          if (ws_->is_zero(new_weight))
            {
              delete e;
              return new zero_t();
            }
          e->right_weight() = ws_->mul(e->right_weight(), new_weight);
        }
      return e;
    }

    template <class WeightSet>
    inline
    auto
    factory<WeightSet>::op_weight(weight_str* w, weight_str_container* l)
      -> weight_str_container*
    {
      l->push_front(w);
      return l;
    }

    template <class WeightSet>
    inline
    auto
    factory<WeightSet>::op_weight(weight_str_container* l, weight_str* w)
      -> weight_str_container*
    {
      l->push_front(w);
      return l;
    }

  } // namespace rat
} // namespace vcsn

#endif // !VCSN_CORE_RAT_RAT_EXP_HXX_
