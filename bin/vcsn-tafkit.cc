#include <iostream>
#include <stdexcept>

#include <boost/lexical_cast.hpp>

#include <vcsn/dyn/algos.hh>
#include <vcsn/dyn/polynomial.hh>
#include <vcsn/dyn/ratexp.hh>
#include <vcsn/misc/raise.hh>

#include "parse-args.hh"

#define DEFINE_AUT_FUNCTION(Name)               \
  struct Name: vcsn_function                    \
  {                                             \
    int work_aut(const options& opts) const     \
    {                                           \
      using namespace vcsn::dyn;                \
      /* Input. */                              \
      auto aut = read_automaton(opts);          \
                                                \
      /* Process. */                            \
      auto res = vcsn::dyn::Name(aut);          \
                                                \
      /* Output. */                             \
      opts.print(res);                          \
      return 0;                                 \
    }                                           \
  }

#define DEFINE_AUT_VARIADIC_FUNCTION(Name)              \
  struct Name: vcsn_function                            \
  {                                                     \
    int work_aut(const options& opts) const             \
    {                                                   \
      using namespace vcsn::dyn;                        \
      /* Input. */                                      \
      auto res = read_automaton(opts);                  \
      for (unsigned i = 0; i < opts.argv.size(); ++i)   \
        {                                               \
          /* Hack. */                                   \
          options opts2 = opts;                         \
          opts2.input = opts.argv[i];                   \
          automaton rhs = read_automaton(opts2);        \
          res = vcsn::dyn::Name(res, rhs);              \
        }                                               \
                                                        \
      /* Output. */                                     \
      opts.print(res);                                  \
      return 0;                                         \
    }                                                   \
  }

#define DEFINE_AUT_SIZE_FUNCTION(Name)                          \
  struct Name: vcsn_function                                    \
  {                                                             \
    int work_aut(const options& opts) const                     \
    {                                                           \
      using namespace vcsn::dyn;                                \
      /* Input. */                                              \
      automaton aut = read_automaton(opts);                     \
      assert(1 <= opts.argv.size());                            \
      size_t n = boost::lexical_cast<size_t>(opts.argv[0]);     \
                                                                \
      /* Process. */                                            \
      auto res = vcsn::dyn::Name(aut, n);                       \
                                                                \
      /* Output. */                                             \
      opts.print(res);                                          \
      return 0;                                                 \
    }                                                           \
  }

#define DEFINE_ENUMERATION_FUNCTION(Name)                               \
  struct Name: vcsn_function                                            \
  {                                                                     \
    int work_aut(const options& opts) const                             \
    {                                                                   \
      using namespace vcsn::dyn;                                        \
      if (opts.output_format == "default"                               \
          || opts.output_format == "")                                  \
        vcsn::dyn::set_format(*opts.out, "list");                       \
                                                                        \
      auto aut = read_automaton(opts);                                  \
      unsigned max = (0 < opts.argv.size()                              \
                      ? boost::lexical_cast<unsigned>(opts.argv[0])     \
                      : 1);                                             \
                                                                        \
      auto res = vcsn::dyn::Name(aut, max);                             \
                                                                        \
      if (!res->empty() || vcsn::dyn::get_format(*opts.out) != "list")  \
        opts.print(res);                                                \
      return 0;                                                         \
    }                                                                   \
                                                                        \
    int work_exp(const options& opts) const                             \
    {                                                                   \
      using namespace vcsn::dyn;                                        \
      if (opts.output_format == "default"                               \
          || opts.output_format == "")                                  \
        vcsn::dyn::set_format(*opts.out, "list");                       \
                                                                        \
      auto exp = read_ratexp(opts);                                     \
      unsigned max = (0 < opts.argv.size()                              \
                      ? boost::lexical_cast<unsigned>(opts.argv[0])     \
                      : 1);                                             \
                                                                        \
      auto res = vcsn::dyn::Name(vcsn::dyn::standard(exp), max);        \
                                                                        \
      if (!res->empty() || vcsn::dyn::get_format(*opts.out) != "list")  \
        opts.print(res);                                                \
      return 0;                                                         \
    }                                                                   \
  }

#define DEFINE_RATEXP_FUNCTION(Name)            \
  struct Name: vcsn_function                    \
  {                                             \
    int work_exp(const options& opts) const     \
    {                                           \
      using namespace vcsn::dyn;                \
      /* Input. */                              \
      auto exp = read_ratexp(opts);             \
                                                \
      /* Process. */                            \
      auto res = vcsn::dyn::Name(exp);          \
                                                \
      /* Output. */                             \
      opts.print(res);                          \
      return 0;                                 \
    }                                           \
  }

#define DEFINE_AUT__RATEXP_FUNCTION(Name)       \
  struct Name: vcsn_function                    \
  {                                             \
    int work_aut(const options& opts) const     \
    {                                           \
      using namespace vcsn::dyn;                \
      automaton aut = read_automaton(opts);     \
                                                \
      auto res = vcsn::dyn::Name(aut);          \
                                                \
      opts.print(res);                          \
      return 0;                                 \
    }                                           \
                                                \
    int work_exp(const options& opts) const     \
    {                                           \
      using namespace vcsn::dyn;                \
      ratexp exp = read_ratexp(opts);           \
                                                \
      auto res = vcsn::dyn::Name(exp);          \
                                                \
      opts.print(res);                          \
      return 0;                                 \
    }                                           \
  }


DEFINE_AUT_FUNCTION(accessible);
DEFINE_AUT_FUNCTION(aut_to_exp);
DEFINE_AUT_SIZE_FUNCTION(chain);
DEFINE_AUT_FUNCTION(coaccessible);
DEFINE_AUT_FUNCTION(complement);
DEFINE_AUT_FUNCTION(complete);
DEFINE_AUT_VARIADIC_FUNCTION(concatenate);
DEFINE_RATEXP_FUNCTION(constant_term);
DEFINE_AUT_VARIADIC_FUNCTION(difference);
DEFINE_ENUMERATION_FUNCTION(enumerate);
DEFINE_RATEXP_FUNCTION(expand);
DEFINE_AUT_VARIADIC_FUNCTION(infiltration);
DEFINE_AUT_FUNCTION(is_ambiguous);
DEFINE_AUT_FUNCTION(is_complete);
DEFINE_AUT_FUNCTION(is_deterministic);
DEFINE_AUT_FUNCTION(is_empty);
DEFINE_AUT_FUNCTION(is_eps_acyclic);
DEFINE_AUT_FUNCTION(is_normalized);
DEFINE_AUT_FUNCTION(is_proper);
DEFINE_AUT_FUNCTION(is_standard);
DEFINE_AUT_FUNCTION(is_trim);
DEFINE_AUT_FUNCTION(is_useless);
DEFINE_AUT__RATEXP_FUNCTION(is_valid);
DEFINE_AUT__RATEXP_FUNCTION(lift);
DEFINE_AUT_SIZE_FUNCTION(power);
DEFINE_AUT_FUNCTION(proper);
DEFINE_ENUMERATION_FUNCTION(shortest);
DEFINE_AUT_VARIADIC_FUNCTION(shuffle);
DEFINE_RATEXP_FUNCTION(split);
DEFINE_AUT__RATEXP_FUNCTION(standard);
DEFINE_AUT_FUNCTION(star);
DEFINE_RATEXP_FUNCTION(star_normal_form);
DEFINE_AUT_VARIADIC_FUNCTION(sum);
DEFINE_RATEXP_FUNCTION(thompson);
DEFINE_AUT__RATEXP_FUNCTION(transpose);
DEFINE_AUT_FUNCTION(trim);
DEFINE_AUT_VARIADIC_FUNCTION(union_a);
DEFINE_AUT_FUNCTION(universal);

struct are_equivalent: vcsn_function
{
  int work_aut(const options& opts) const
  {
    using namespace vcsn::dyn;
    // Input.
    automaton lhs = read_automaton(opts);
    // FIXME: hack.
    options opts2 = opts;
    opts2.input = opts.argv[0];
    automaton rhs = read_automaton(opts2);

    // Process.
    auto res = vcsn::dyn::are_equivalent(lhs, rhs);

    // Output.
    opts.print(res);
    return res ? 0 : 2;
  }

  int work_exp(const options& opts) const
  {
    using namespace vcsn::dyn;
    // Input.
    ratexp lhs = read_ratexp(opts);
    // FIXME: hack.
    options opts2 = opts;
    opts2.input = opts.argv[0];
    ratexp rhs = read_ratexp(opts2);

    // Process.
    auto res = vcsn::dyn::are_equivalent(lhs, rhs);

    // Output.
    opts.print(res);
    return res ? 0 : 2;
  }
};

struct are_isomorphic: vcsn_function
{
  int work_aut(const options& opts) const
  {
    using namespace vcsn::dyn;
    // Input.
    automaton lhs = read_automaton(opts);
    // FIXME: hack.
    options opts2 = opts;
    opts2.input = opts.argv[0];
    automaton rhs = read_automaton(opts2);

    // FIXME: generalize to an arbitrary number of automata

    // Process.
    auto res = vcsn::dyn::are_isomorphic(lhs, rhs);

    // Output.
    opts.print(res);
    return res ? 0 : 2;
  }
};

struct cat: vcsn_function
{
  virtual int work_aut(const options& opts) const override
  {
    using namespace vcsn::dyn;
    // Input.
    auto aut = read_automaton(opts);
    // Output.
    opts.print(aut);
    return 0;
  }

  virtual int work_exp(const options& opts) const override
  {
    using namespace vcsn::dyn;
    // Input.
    auto exp = read_ratexp(opts);
    // Output.
    opts.print(exp);
    return 0;
  }

  virtual int work_polynomial(const options& opts) const override
  {
    using namespace vcsn::dyn;
    // Input.
    auto exp = read_polynomial(opts);
    // Output.
    opts.print(exp);
    return 0;
  }

  virtual int work_weight(const options& opts) const override
  {
    using namespace vcsn::dyn;
    // Input.
    auto w = read_weight(opts);
    // Output.
    opts.print(w);
    return 0;
  }
};

struct derivation: vcsn_function
{
  int work_exp(const options& opts) const
  {
    // Input.
    using namespace vcsn::dyn;
    ratexp exp = read_ratexp(opts);
    assert(0 < opts.argv.size());
    std::string s = opts.argv[0];
    bool breaking = (1 < opts.argv.size()
                     ? boost::lexical_cast<bool>(opts.argv[1])
                     : false);

    // Process.
    auto res = vcsn::dyn::derivation(exp, s, breaking);

    // Output.
    opts.print(res);
    return 0;
  }
};

struct derived_term: vcsn_function
{
  int work_exp(const options& opts) const
  {
    // Input.
    using namespace vcsn::dyn;
    ratexp exp = read_ratexp(opts);
    bool breaking = (0 < opts.argv.size()
                     ? boost::lexical_cast<bool>(opts.argv[0])
                     : false);

    // Process.
    automaton res = vcsn::dyn::derived_term(exp, breaking);

    // Output.
    opts.print(res);
    return 0;
  }
};

struct determinize: vcsn_function
{
  int work_aut(const options& opts) const
  {
    using namespace vcsn::dyn;
    // Input.
    auto aut = read_automaton(opts);
    bool complete = (0 < opts.argv.size()
                     ? boost::lexical_cast<bool>(opts.argv[0])
                     : false);

    // Process.
    auto res = vcsn::dyn::determinize(aut, complete);

    // Output.
    opts.print(res);
    return 0;
  }
};

struct eliminate_state: vcsn_function
{
  int work_aut(const options& opts) const
  {
    using namespace vcsn::dyn;
    // Input.
    auto aut = read_automaton(opts);
    int n = (0 < opts.argv.size()
             ? boost::lexical_cast<int>(opts.argv[0])
             : -1);

    // Process.
    auto res = vcsn::dyn::eliminate_state(aut, n);

    // Output.
    opts.print(res);
    return 0;
  }
};


struct evaluate: vcsn_function
{
  int work_aut(const options& opts) const
  {
    using namespace vcsn::dyn;
    // Input.
    auto aut = read_automaton(opts);
    options opts2 = opts;
    opts2.input = opts.argv[0];
    opts2.input_is_file = false;
    label l = read_word(opts2);

    // Process.
    auto res = vcsn::dyn::eval(aut, l);

    // Output.
    opts.print(res);
    return 0;
  }
};

struct left_mult: vcsn_function
{
  int work_aut(const options& opts) const
  {
    using namespace vcsn::dyn;
    // Input.
    automaton aut = read_automaton(opts);
    assert(1 <= opts.argv.size());

    // FIXME: hack.
    options opts2 = opts;
    opts2.input = opts.argv[0];
    opts2.input_is_file = false;
    weight w = read_weight(opts2);

    // Process.
    auto res = vcsn::dyn::left_mult(w, aut);

    // Output.
    opts.print(res);
    return 0;
  }
};

struct minimize: vcsn_function
{
  int work_aut(const options& opts) const
  {
    using namespace vcsn::dyn;
    // Input.
    auto aut = read_automaton(opts);
    std::string algo = 1 <= opts.argv.size() ? opts.argv[0] : "signature";

    // Process.
    auto res = vcsn::dyn::minimize(aut, algo);

    // Output.
    opts.print(res);
    return 0;
  }
};

struct product: vcsn_function
{
  int work_aut(const options& opts) const
  {
    using namespace vcsn::dyn;
    /* Input. */
    std::vector<automaton> as;
    as.emplace_back(read_automaton(opts));
    for (unsigned i = 0; i < opts.argv.size(); ++i)
      {
        options opts2 = opts;
        opts2.input = opts.argv[i];
        as.emplace_back(read_automaton(opts2));
      }
    auto res = vcsn::dyn::product(as);

    /* Output. */
    opts.print(res);
    return 0;
  }
};

struct right_mult: vcsn_function
{
  int work_aut(const options& opts) const
  {
    using namespace vcsn::dyn;
    // Input.
    automaton aut = read_automaton(opts);
    assert(1 <= opts.argv.size());

    // FIXME: hack.
    options opts2 = opts;
    opts2.input = opts.argv[0];
    opts2.input_is_file = false;
    weight w = read_weight(opts2);

    // Process.
    auto res = vcsn::dyn::right_mult(aut, w);

    // Output.
    opts.print(res);
    return 0;
  }
};

/*-------------------.
| "Main" functions.  |
`-------------------*/

static int de_bruijn(int argc, char * const argv[])
{
  options opts;
  opts.input_format = "text";
  parse_args(opts, argc, argv);

  // Input.
  using namespace vcsn::dyn;
  auto ctx = vcsn::dyn::make_context(opts.context);
  assert(1 <= argc);
  size_t n = boost::lexical_cast<size_t>(argv[0]);

  // Process.
  automaton aut = de_bruijn(ctx, n);

  // Output.
  opts.print(aut);
  return 0;
}

static int divkbaseb(int argc, char* const argv[])
{
  options opts;
  opts.input_format = "text";
  parse_args(opts, argc, argv);

  // Input.
  using namespace vcsn::dyn;
  auto ctx = make_context(opts.context);
  assert(2 <= argc);
  auto k = boost::lexical_cast<unsigned>(argv[0]);
  auto b = boost::lexical_cast<unsigned>(argv[1]);

  // Process.
  automaton aut = divkbaseb(ctx, k, b);

  // Output.
  opts.print(aut);
  return 0;
}

static int double_ring(int argc, char * const argv[])
{
  options opts;
  opts.input_format = "text";
  parse_args(opts, argc, argv);

  // Input.
  using namespace vcsn::dyn;
  auto ctx = vcsn::dyn::make_context(opts.context);
  assert(1 <= argc);
  size_t n = boost::lexical_cast<size_t>(argv[0]);
  // final states.
  std::vector<unsigned> f;
  for (int i = 1; i < argc; ++i)
    f.emplace_back(boost::lexical_cast<unsigned>(argv[i]));

  // Process.
  automaton aut = double_ring(ctx, n, f);

  // Output.
  opts.print(aut);
  return 0;
}

static int ladybird(int argc, char * const argv[])
{
  options opts;
  opts.input_format = "text";
  parse_args(opts, argc, argv);

  // Input.
  using namespace vcsn::dyn;
  auto ctx = vcsn::dyn::make_context(opts.context);
  assert(1 <= argc);
  size_t n = boost::lexical_cast<size_t>(argv[0]);

  // Process.
  automaton aut = ladybird(ctx, n);

  // Output.
  opts.print(aut);
  return 0;
}

static int random(int argc, char * const argv[])
{
  options opts;
  parse_args(opts, argc, argv);

  // Input.
  using namespace vcsn::dyn;
  auto ctx = vcsn::dyn::make_context(opts.context);
  assert(0 < argc);
  assert(argc <= 4);
  unsigned num_states  =          boost::lexical_cast<unsigned>(argv[0]);
  float density        = 1 < argc ? boost::lexical_cast<float>(argv[1])    : .1;
  unsigned num_initial = 2 < argc ? boost::lexical_cast<unsigned>(argv[2]) : 1;
  unsigned num_final   = 3 < argc ? boost::lexical_cast<unsigned>(argv[3]) : 1;

  // Process.
  automaton aut = random_automaton(ctx, num_states, density,
                                   num_initial, num_final);

  // Output.
  opts.print(aut);
  return 0;
}

static int u(int argc, char * const argv[])
{
  options opts;
  opts.input_format = "text";
  parse_args(opts, argc, argv);

  // Input.
  using namespace vcsn::dyn;
  auto ctx = vcsn::dyn::make_context(opts.context);
  assert(1 <= argc);
  auto n = boost::lexical_cast<unsigned>(argv[0]);

  // Process.
  automaton aut = u(ctx, n);

  // Output.
  opts.print(aut);
  return 0;
}


/*-------.
| Main.  |
`-------*/

int main(int argc, char* const argv[])
try
{
  vcsn::require(1 < argc, "no command given");
  std::string cmd{argv[1]};
  if (cmd == "divkbaseb") return divkbaseb(argc - 1, argv + 1);
  if (cmd == "de_bruijn") return de_bruijn(argc - 1, argv + 1);
  if (cmd == "double_ring") return double_ring(argc - 1, argv + 1);
  if (cmd == "ladybird") return ladybird(argc - 1, argv + 1);
  if (cmd == "random") return random(argc - 1, argv + 1);
  if (cmd == "u") return u(argc - 1, argv + 1);
  else
    {
      if (cmd == "union") cmd = "union_a";
      std::unique_ptr<vcsn_function> f;
#define ALGO(Name)                                                      \
      else if (cmd == #Name) f = std::unique_ptr<Name>(new Name{})
      if (false) {}
      ALGO(accessible);
      ALGO(are_equivalent);
      ALGO(are_isomorphic);
      ALGO(aut_to_exp);
      ALGO(cat);
      ALGO(chain);
      ALGO(coaccessible);
      ALGO(complement);
      ALGO(complete);
      ALGO(concatenate);
      ALGO(constant_term);
      ALGO(derivation);
      ALGO(derived_term);
      ALGO(determinize);
      ALGO(difference);
      ALGO(eliminate_state);
      ALGO(enumerate);
      ALGO(evaluate);
      ALGO(expand);
      ALGO(infiltration);
      ALGO(is_ambiguous);
      ALGO(is_complete);
      ALGO(is_deterministic);
      ALGO(is_empty);
      ALGO(is_eps_acyclic);
      ALGO(is_normalized);
      ALGO(is_proper);
      ALGO(is_standard);
      ALGO(is_trim);
      ALGO(is_useless);
      ALGO(is_valid);
      ALGO(lift);
      ALGO(minimize);
      ALGO(power);
      ALGO(product);
      ALGO(proper);
      ALGO(shortest);
      ALGO(shuffle);
      ALGO(split);
      ALGO(standard);
      ALGO(star);
      ALGO(star_normal_form);
      ALGO(sum);
      ALGO(thompson);
      ALGO(transpose);
      ALGO(trim);
      ALGO(union_a);
      ALGO(universal);
      return vcsn_main(argc - 1, argv + 1, *f);
    }
  vcsn::raise("unknown command: " + cmd);
}
catch (const std::exception& e)
{
  std::cerr << e.what() << std::endl;
  exit(EXIT_FAILURE);
 }
