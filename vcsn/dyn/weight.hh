#ifndef VCSN_DYN_WEIGHT_HH
# define VCSN_DYN_WEIGHT_HH

# include <iostream>
# include <memory>
# include <typeinfo>

# include <vcsn/ctx/fwd.hh>
# include <vcsn/dyn/weightset.hh>
# include <vcsn/misc/export.hh>

namespace vcsn
{
  namespace dyn
  {
    namespace detail
    {

      /// Aggregate a weight and its weightset.
      ///
      /// FIXME: Improperly named, it is not a base class for
      /// static weights.
      class LIBVCSN_API abstract_weight
      {
      public:
        /// A description of the weight type.
        /// \param full  whether to include the genset.
        ///              if false, same as sname.
        virtual std::string vname(bool full = true) const = 0;

        template <typename WeightSet>
        concrete_abstract_weight<WeightSet>& as()
        {
          return dynamic_cast<concrete_abstract_weight<WeightSet>&>(*this);
        }

        template <typename WeightSet>
        const concrete_abstract_weight<WeightSet>& as() const
        {
          std::cerr << "as: typeinfo: " << &typeid(*this) << std::endl;
          std::cerr << "as: typeinfo: " << &typeid(const concrete_abstract_weight<WeightSet>&) << std::endl;
          return dynamic_cast<const concrete_abstract_weight<WeightSet>&>(*this);
        }
      };

      /// Aggregate a weight and its weightset.
      ///
      /// FIXME: Improperly named, it is not a base class for
      /// static weights.
      template <typename WeightSet>
      class LIBVCSN_API concrete_abstract_weight: public abstract_weight
      {
      public:
        using weightset_t = WeightSet;
        using super_type = abstract_weight;
        using weight_t = typename weightset_t::value_t;
        concrete_abstract_weight(const weight_t& weight,
                                 const weightset_t& weightset)
          : weight_(weight)
          , weightset_(weightset)
        {}

        virtual std::string vname(bool full = true) const override
        {
          return get_weightset().vname(full);
        }

        const weight_t weight() const
        {
          return weight_;
        }

        const weightset_t& get_weightset() const
        {
          return weightset_;
        }

      protected:
        /// The weight.
        const weight_t weight_;
        /// The weight set.
        const weightset_t weightset_;
      };

    } // namespace detail

    using weight = std::shared_ptr<const detail::abstract_weight>;

    template <typename WeightSet>
    LIBVCSN_API
    inline
    weight
    make_weight(const WeightSet& ws,
                const typename WeightSet::value_t& weight)
    {
      auto res = std::make_shared<detail::concrete_abstract_weight<WeightSet>>
        (weight, ws);
      std::cerr << "makere: typeinfo: " << &typeid(*res) << std::endl;
      std::cerr << "makere: typeinfo: " << &typeid(detail::concrete_abstract_weight<WeightSet>) << std::endl;
      return res;
    }
  } // namespace dyn
} // namespace vcsn

#endif // !VCSN_DYN_WEIGHT_HH
