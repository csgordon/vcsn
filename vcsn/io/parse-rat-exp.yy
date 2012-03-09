// -*- mode: c++ -*-
%pure-parser
%require "2.5"
%language "C++"

%name-prefix "rat_exp_parse_"
%expect 0
%error-verbose
%defines
%debug

%locations

%code requires
{
  #include <iostream>
  #include <list>
  #include <string>
  #include "location.hh"
  #include <core/rat_exp/node.hh>
  #include <core/rat_exp/RatExpFactory.hh>
  #include <core/rat_exp/print_debug_visitor.hh>

  struct foo
  {
    typedef int value_t;
    static int one() { return 1; }
    static int zero() { return 0; }
    static void op_mul_eq(int i, std::string *str) {
      i *= atoi(str->c_str());
      }
  }; // FIXME

  union YYSTYPE
  {
    int ival;
    std::string* sval;
    vcsn::rat_exp::weight_type* weight;
    vcsn::rat_exp::weights_type* weights;
    vcsn::rat_exp::RatExp *nodeval;
  };

  #define YY_DECL                                                       \
    int rat_exp_parse_lex(YYSTYPE* yylval,                              \
                          rat_exp_parse_::location* yylloc)
  YY_DECL;
}

%code
{
  #include <cassert>
  #include <sstream>
  #define STRING_(Out, In)                        \
    do {                                        \
      std::stringstream o;                      \
      o << In;                                  \
      Out = new std::string(o.str());           \
    } while (false)
  #define STRING(Out, In)                         \
    STRING_(Out, '(' << In << ')')

  #define MAKE(Kind, ...)                         \
    fact.op_ ## Kind(__VA_ARGS__)

  typedef vcsn::rat_exp::weight_type weight_type;
  typedef vcsn::rat_exp::weights_type weights_type;

  // static
  // weights_type*
  // make_weights(weight_type* w)
  // {
  //   return new weights_type {w};
  // }

  // static
  // weights_type*
  // make_weights(weight_type* w, weights_type* ws)
  // {
  //   ws->push_front(w);
  //   return ws;
  // }

  std::ostream&
  operator<<(std::ostream& o, const weights_type& ws)
  {
    o << "{";
    bool first = true;
    for (weight_type* w: ws)
      {
        if (!first)
          o << ", ";
        first = false;
        o << *w;
      }
    o << "}";
    return o;
  }

  std::string*
  make_term(std::string *lexp, weights_type* ws)
  {
    assert(lexp);
    std::string* res = lexp;
    if (ws)
      {
        STRING(res, "r" << *ws << *lexp);
        delete lexp;
        delete ws;
      }
    return res;
  }

  std::string*
  make_term(weights_type* ws, std::string *term)
  {
    assert(term);
    std::string *res = term;
    if (ws)
      {
        STRING(res, "l" << *ws << *term);
        delete term;
        delete ws;
      }
    return res;
  }

  std::string*
  make_term(std::string* lexp, weights_type* ws, std::string *factors)
  {
    assert(lexp);
    assert(ws);
    assert(factors);
    std::string* res;
    STRING(res, *lexp << "#(l" << *ws << *factors << ")");
    delete lexp;
    delete ws;
    delete factors;
    return res;
  }

  // define the factory
  vcsn::rat_exp::RatExpFactory<foo> fact; // FIXME: specialization
}

%printer { debug_stream() << *$$; } <sval>;
%printer { debug_stream() << $$; } <ival>;

%destructor { delete $$; } <sval>;

%destructor { delete $$; } <rat_exp *> <rat_concat *> <rat_plus *>
        <rat_kleene *> <rat_one *> <rat_zero *> <rat_atom *> <rat_word *>
        <rat_left_weight *> <rat_right_weight *>

%token <ival>   LPAREN          "("
                RPAREN          ")"
%token  PLUS            "+"
        DOT             "."
        STAR            "*"
        ONE             "\\e"
        ZERO            "\\z"
;

%token  <sval> WORD    "word";
%token <weight> WEIGHT  "weight"

%type <weights> weights weights.opt;
%type <nodeval> exps exp term lexp factor word factors;


%left "+"
%left "."

%start exps
%%

exps:
  exp
  {
    auto node = fact.cleanNode($1);
    auto down = down_cast<vcsn::rat_exp::RatExpNode<foo> *>(node);
    assert(down);
    vcsn::rat_exp::PrintDebugVisitor<foo> print(std::cout);
    down->accept(print);
    $$ = node;
  }
;

exp:
  term                          { $$ = $1; }
| exp "." exp                   { $$ = MAKE(mul, $1, $3); }
| exp "+" exp                   { $$ = MAKE(add, $1, $3); }
;

term:
  lexp weights.opt              { $$ = $2 ? MAKE(weight, $1, $2) : $1; }
;

lexp:
  weights.opt factors           { $$ = MAKE(weight, $1, $2); }
| lexp weights factors          { $$ = MAKE(mul, $1, MAKE(weight, $2, $3)); }
;

factors:
  factor                        { $$ = MAKE(mul, $1); }
| factors factor                { $$ = MAKE(mul, $1, $2); }
;

factor:
  word                          { $$ = $1; }
| factor "*"                    { $$ = MAKE(kleene, $1); }
| "(" exp ")"                   { $$ = fact.cleanNode($2); assert($1 == $3); }
;

word:
  ZERO                          { $$ = MAKE(zero); }
| ONE                           { $$ = MAKE(one); }
| WORD                          { $$ = MAKE(word, $1); }
;

weights.opt:
  /* empty */                   { $$ = nullptr; }
| weights                       { $$ = $1; }
;

weights:
  "weight"                      { $$ = MAKE(weight, $1); }
| "weight" weights              { $$ = MAKE(weight, $1, $2); }
;

%%

void
rat_exp_parse_::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << std::endl;
}

int main()
{
  rat_exp_parse_::parser p;
  extern int yy_flex_debug;
  yy_flex_debug = !!getenv("YYSCAN");
  p.set_debug_level(!!getenv("YYDEBUG"));
  return p.parse();
}
