op_mul
op_add
op_sub
ellipsis
op_div
cmp_lt
cmp_le
cmp_eq
cond_recip
cmp_gt
cmp_ge
abs
and
append
apply
assoc
assq
assv
begin
is_binary_port
is_boolean_eq
is_boolean
bytevector
bytevector_append
bytevector_copy
bytevector_copy_mut
bytevector_length
bytevector_u8_ref
bytevector_u8_set_mut
is_bytevector
caar
cadr
call_with_current_continuation
call_with_port
call_with_values
call_cc
car
case
cdar
cddr
cdr
ceiling
char_to_integer
is_char_ready
is_char_le
is_char_lt
is_char_eq
is_char_ge
is_char_gt
is_char
close_input_port
close_output_port
close_port
is_complex
cond
cond_expand
cons
current_error_port
current_input_port
current_output_port
define
define_record_type
define_syntax
define_values
denominator
do
dynamic_wind
else
eof_object
is_eof_object
is_eq
is_equal
is_eqv
error
error_object_irritants
error_object_message
is_error_object
is_even
exact
exact_integer_sqrt
is_exact_integer
is_exact
expt
features
is_file_error
floor
floor_quotient
floor_remainder
floor_div
flush_output_port
for_each
gcd
get_output_bytevector
get_output_string
guard
if
include
include_ci
inexact
is_inexact
is_input_port_open
is_input_port
integer_to_char
is_integer
lambda
lcm
length
let
let_star
let_star_values
let_syntax
let_values
letrec
letrec_star
letrec_syntax
list
list_to_string
list_to_vector
list_copy
list_ref
list_set_mut
list_tail
is_list
make_bytevector
make_list
make_parameter
make_string
make_vector
map
max
member
memq
memv
min
modulo
is_negative
newline
not
is_null
number_to_string
is_number
numerator
is_odd
open_input_bytevector
open_input_string
open_output_bytevector
open_output_string
or
is_output_port_open
is_output_port
is_pair
parameterize
peek_char
peek_u8
is_port
is_positive
is_procedure
quasiquote
quote
quotient
raise
raise_continuable
is_rational
rationalize
read_bytevector
read_bytevector_mut
read_char
is_read_error
read_line
read_string
read_u8
is_real
remainder
reverse
round
set_mut
set_car_mut
set_cdr_mut
square
string
string_to_list
string_to_number
string_to_symbol
string_to_utf8
string_to_vector
string_append
string_copy
string_copy_mut
string_fill_mut
string_for_each
string_length
string_map
string_ref
string_set_mut
is_string_le
is_string_lt
is_string_eq
is_string_ge
is_string_gt
is_string
substring
symbol_to_string
is_symbol_eq
is_symbol
syntax_error
syntax_rules
is_textual_port
truncate
truncate_quotient
truncate_remainder
truncate_div
is_u8_ready
unless
unquote
unquote_splicing
utf8_to_string
values
vector
vector_to_list
vector_to_string
vector_append
vector_copy
vector_copy_mut
vector_fill_mut
vector_for_each
vector_length
vector_map
vector_ref
vector_set_mut
is_vector
when
with_exception_handler
write_bytevector
write_char
write_string
write_u8
is_zero


enum
{
    ARGTYPE_INVALID = 0,

    ARGTYPE_NONE,
    ARGTYPE_ALIST,
    ARGTYPE_BOOLEAN,
    ARGTYPE_BYTE,
    ARGTYPE_BYTEVECTOR,
    ARGTYPE_CHAR,
    ARGTYPE_EXACT_INTEGER,
    ARGTYPE_ALPHA,
    ARGTYPE_LIST,
    ARGTYPE_INTEGER,
    ARGTYPE_NUMBER,
    ARGTYPE_OBJ,
    ARGTYPE_PAIR,
    ARGTYPE_PORT,
    ARGTYPE_PROC,
    ARGTYPE_RATIONAL,
    ARGTYPE_STRING,
    ARGTYPE_SYMBOL,
    ARGTYPE_THUNK,
    ARGTYPE_VECTOR,
    ARGTYPE_REAL,
    ARGTYPE_COMPLEX,

    ARGTYPE_COUNT,
    ARGTYPE_MULTIPLE = 0X80
};


    "floor", { ARGTYPE_REAL }, &Base::Floor,
    { 
        "(floor -4.3)", "-5.0",
        "(floor 3.5)",  "3.0",
    },
    "ceiling", { ARGTYPE_REAL }, &Base::Ceiling,
    { 
        "(ceiling -4.3)", "-4.0",
        "(ceiling 3.5)",  "4.0",
    },
    "truncate", { ARGTYPE_REAL }, &Base::Truncate,
    { 
        "(truncate -4.3)", "-4.0",
        "(truncate 3.5)",  "3.0",
    },
    "round", { ARGTYPE_REAL }, &Base::Round,
    { 
        "(round -4.3)",         "-4.0",
        "(round 3.5)",          "4.0",
        "(exact? (round 3.5))", "#f",
        "(round 7/2)",          "4",
        "(exact? (round 7/2))", "#t",
        "(round 7)",            "7",
    },
    "floor/", { ARGTYPE_INTEGER, ARGTYPE_INTEGER }, &Base::FloorDiv,
    {
        "(floor/ 5 2)",   "(values 2 1)",
        "(floor/ -5 2)",  "(values -3 1)",
        "(floor/ 5 -2)",  "(values -3 -1)",
        "(floor/ -5 -2)", "(values 2 -1)",
    },
    "floor-quotient", { ARGTYPE_INTEGER, ARGTYPE_INTEGER }, &Base::FloorQuotient,
    {
        "(floor-quotient 5 2)",   "2",
        "(floor-quotient -5 2)",  "-3",
        "(floor-quotient 5 -2)",  "-3",
        "(floor-quotient -5 -2)", "2",
    },
    "floor-remainder", { ARGTYPE_INTEGER, ARGTYPE_INTEGER }, &Base::FloorRemainder,
    {
        "(floor-remainder 5 2)",   "1",
        "(floor-remainder -5 2)",  "1",
        "(floor-remainder 5 -2)",  "-1",
        "(floor-remainder -5 -2)", "-1",
    },
    "truncate/", { ARGTYPE_INTEGER, ARGTYPE_INTEGER }, &Base::TruncateDiv,
    {
        "(truncate/ 5 2)",     "(values 2 1)",
        "(truncate/ -5 2)",    "(values -2 -1)",
        "(truncate/ 5 -2)",    "(values -2 1)",
        "(truncate/ -5 -2)",   "(values 2 -1)",
        "(truncate/ -5.0 -2)", "(values 2.0 -1.0)",
    },
    "truncate-quotient", { ARGTYPE_INTEGER, ARGTYPE_INTEGER }, &Base::TruncateQuotient,
    {
        "(truncate-quotient 5 2)",     "2",
        "(truncate-quotient -5 2)",    "-2",
        "(truncate-quotient 5 -2)",    "-2",
        "(truncate-quotient -5 -2)",   "2",
        "(truncate-quotient -5.0 -2)", "2.0",
    },

    "truncate-remainder", &Base::TruncateRemainder, R"#(
        (pre-condition
            (= (length args) 2)
            (integer? (nth 0 args))
            (integer? (nth 1 args))
        (post-condition
            (integer? result))
        (test-case
            (= (truncate-remainder 5 2) 1)
            (= (truncate-remainder -5 2) -1)
            (= (truncate-remainder 5 -2) 1)
            (= (truncate-remainder -5 -2) -1)
            (= (truncate-remainder -5.0 -2) -1.0)))#",

    "quotient", { ARGTYPE_INTEGER, ARGTYPE_INTEGER }, 
    &Base::TruncateQuotient, 
    {
        "(quotient 5 2)",     "2",
        "(quotient -5 2)",    "-2",
        "(quotient 5 -2)",    "-2",
        "(quotient -5 -2)",   "2",
        "(quotient -5.0 -2)", "2.0",
    },
    "remainder", { ARGTYPE_INTEGER, ARGTYPE_INTEGER }, &Base::TruncateRemainder,
    {
        "(remainder 5 2)",     "1",
        "(remainder -5 2)",    "-1",
        "(remainder 5 -2)",    "1",
        "(remainder -5 -2)",   "-1",
        "(remainder -5.0 -2)", "-1.0",
    },
    "modulo", { ARGTYPE_INTEGER, ARGTYPE_INTEGER }, &Base::FloorRemainder,
    {
        "(modulo 5 2)",   "1",
        "(modulo -5 2)",  "1",
        "(modulo 5 -2)",  "-1",
        "(modulo -5 -2)", "-1",
    },
    "gcd", { ARGTYPE_INTEGER | ARGTYPE_MULTI }, &Base::GreatestCommonDivisor,
    {
        "(gcd 32 -36)", "4",
        "(gcd)", "0",
    },
    "lcm", { ARGTYPE_INTEGER | ARGTYPE_MULTI }, &Base::LeastCommonMultiple,
    {
        "(lcm 32 -36)", "288",
        "(lcm 32.0 -36)", "288.0",
        "(exact? (lcm 32.0 -36))", "#f",
        "(lcm)", "1",
    },
    "numerator", { ARGTYPE_RATIONAL }, &Base::Numerator,
    {

    }

/*

(define (validate-atan2 result y x) (begin
    (if (and (= y  0.0) (> x 0.0)) (assert (= result  0.0)))
    (if (and (= y +0.0) (> x 0.0)) (assert (= result +0.0)))
    (if (and (= y -0.0) (> x 0.0)) (assert (= result -0.0)))


// input validation - detect errors in program
// output validation - detect errors in implementation
// output testing - 


    (any (inexact?


*/




"*",                                        op_mul
"+",                                        op_add
"-",                                        op_sub
"...",                                      ellipsis
"/",                                        op_div
"<",                                        cmp_lt
"<=",                                       cmp_le
"=",                                        cmp_eq
"=>",                                       cond_recip
">",                                        cmp_gt
">=",                                       cmp_ge
"abs",                                      abs
"and",                                      and
"append",                                   append
"apply",                                    apply
"assoc",                                    assoc
"assq",                                     assq
"assv",                                     assv
"begin",                                    begin
"binary-port?",                             is_binary_port
"boolean=?",                                is_boolean_eq
"boolean?",                                 is_boolean
"bytevector",                               bytevector
"bytevector-append",                        bytevector_append
"bytevector-copy",                          bytevector_copy
"bytevector-copy!",                         bytevector_copy_mut
"bytevector-length",                        bytevector_length
"bytevector-u8-ref",                        bytevector_u8_ref
"bytevector-u8-set!",                       bytevector_u8_set_mut
"bytevector?",                              is_bytevector
"caar",                                     caar
"cadr",                                     cadr
"call-with-current-continuation",           call_with_current_continuation
"call-with-port",                           call_with_port
"call-with-values",                         call_with_values
"call/cc",                                  call_cc
"car",                                      car
"case",                                     case
"cdar",                                     cdar
"cddr",                                     cddr
"cdr",                                      cdr
"ceiling",                                  ceiling
"char->integer",                            char_to_integer
"char-ready?",                              is_char_ready
"char<=?",                                  is_char_le
"char<?",                                   is_char_lt
"char=?",                                   is_char_eq
"char>=?",                                  is_char_ge
"char>?",                                   is_char_gt
"char?",                                    is_char
"close-input-port",                         close_input_port
"close-output-port",                        close_output_port
"close-port",                               close_port
"complex?",                                 is_complex
"cond",                                     cond
"cond-expand",                              cond_expand
"cons",                                     cons
"current-error-port",                       current_error_port
"current-input-port",                       current_input_port
"current-output-port",                      current_output_port
"define",                                   define
"define-record-type",                       define_record_type
"define-syntax",                            define_syntax
"define-values",                            define_values
"denominator",                              denominator
"do",                                       do
"dynamic-wind",                             dynamic_wind
"else",                                     else
"eof-object",                               eof_object
"eof-object?",                              is_eof_object
"eq?",                                      is_eq
"equal?",                                   is_equal
"eqv?",                                     is_eqv
"error",                                    error
"error-object-irritants",                   error_object_irritants
"error-object-message",                     error_object_message
"error-object?",                            is_error_object
"even?",                                    is_even
"exact",                                    exact
"exact-integer-sqrt",                       exact_integer_sqrt
"exact-integer?",                           is_exact_integer
"exact?",                                   is_exact
"expt",                                     expt
"features",                                 features
"file-error?",                              is_file_error
"floor",                                    floor
"floor-quotient",                           floor_quotient
"floor-remainder",                          floor_remainder
"floor/",                                   floor_div
"flush-output-port",                        flush_output_port
"for-each",                                 for_each
"gcd",                                      gcd
"get-output-bytevector",                    get_output_bytevector
"get-output-string",                        get_output_string
"guard",                                    guard
"if",                                       if
"include",                                  include
"include-ci",                               include_ci
"inexact",                                  inexact
"inexact?",                                 is_inexact
"input-port-open?",                         is_input_port_open
"input-port?",                              is_input_port
"integer->char",                            integer_to_char
"integer?",                                 is_integer
"lambda",                                   lambda
"lcm",                                      lcm
"length",                                   length
"let",                                      let
"let*",                                     let_star
"let*-values",                              let_star_values
"let-syntax",                               let_syntax
"let-values",                               let_values
"letrec",                                   letrec
"letrec*",                                  letrec_star
"letrec-syntax",                            letrec_syntax
"list",                                     list
"list->string",                             list_to_string
"list->vector",                             list_to_vector
"list-copy",                                list_copy
"list-ref",                                 list_ref
"list-set!",                                list_set_mut
"list-tail",                                list_tail
"list?",                                    is_list
"make-bytevector",                          make_bytevector
"make-list",                                make_list
"make-parameter",                           make_parameter
"make-string",                              make_string
"make-vector",                              make_vector
"map",                                      map
"max",                                      max
"member",                                   member
"memq",                                     memq
"memv",                                     memv
"min",                                      min
"modulo",                                   modulo
"negative?",                                is_negative
"newline",                                  newline
"not",                                      not
"null?",                                    is_null
"number->string",                           number_to_string
"number?",                                  is_number
"numerator",                                numerator
"odd?",                                     is_odd
"open-input-bytevector",                    open_input_bytevector
"open-input-string",                        open_input_string
"open-output-bytevector",                   open_output_bytevector
"open-output-string",                       open_output_string
"or",                                       or
"output-port-open?",                        is_output_port_open
"output-port?",                             is_output_port
"pair?",                                    is_pair
"parameterize",                             parameterize
"peek-char",                                peek_char
"peek-u8",                                  peek_u8
"port?",                                    is_port
"positive?",                                is_positive
"procedure?",                               is_procedure
"quasiquote",                               quasiquote
"quote",                                    quote
"quotient",                                 quotient
"raise",                                    raise
"raise-continuable",                        raise_continuable
"rational?",                                is_rational
"rationalize",                              rationalize
"read-bytevector",                          read_bytevector
"read-bytevector!",                         read_bytevector_mut
"read-char",                                read_char
"read-error?",                              is_read_error
"read-line",                                read_line
"read-string",                              read_string
"read-u8",                                  read_u8
"real?",                                    is_real
"remainder",                                remainder
"reverse",                                  reverse
"round",                                    round
"set!",                                     set_mut
"set-car!",                                 set_car_mut
"set-cdr!",                                 set_cdr_mut
"square",                                   square
"string",                                   string
"string->list",                             string_to_list
"string->number",                           string_to_number
"string->symbol",                           string_to_symbol
"string->utf8",                             string_to_utf8
"string->vector",                           string_to_vector
"string-append",                            string_append
"string-copy",                              string_copy
"string-copy!",                             string_copy_mut
"string-fill!",                             string_fill_mut
"string-for-each",                          string_for_each
"string-length",                            string_length
"string-map",                               string_map
"string-ref",                               string_ref
"string-set!",                              string_set_mut
"string<=?",                                is_string_le
"string<?",                                 is_string_lt
"string=?",                                 is_string_eq
"string>=?",                                is_string_ge
"string>?",                                 is_string_gt
"string?",                                  is_string
"substring",                                substring
"symbol->string",                           symbol_to_string
"symbol=?",                                 is_symbol_eq
"symbol?",                                  is_symbol
"syntax-error",                             syntax_error
"syntax-rules",                             syntax_rules
"textual-port?",                            is_textual_port
"truncate",                                 truncate
"truncate-quotient",                        truncate_quotient
"truncate-remainder",                       truncate_remainder
"truncate/",                                truncate_div
"u8-ready?",                                is_u8_ready
"unless",                                   unless
"unquote",                                  unquote
"unquote-splicing",                         unquote_splicing
"utf8->string",                             utf8_to_string
"values",                                   values
"vector",                                   vector
"vector->list",                             vector_to_list
"vector->string",                           vector_to_string
"vector-append",                            vector_append
"vector-copy",                              vector_copy
"vector-copy!",                             vector_copy_mut
"vector-fill!",                             vector_fill_mut
"vector-for-each",                          vector_for_each
"vector-length",                            vector_length
"vector-map",                               vector_map
"vector-ref",                               vector_ref
"vector-set!",                              vector_set_mut
"vector?",                                  is_vector
"when",                                     when
"with-exception-handler",                   with_exception_handler
"write-bytevector",                         write_bytevector
"write-char",                               write_char
"write-string",                             write_string
"write-u8",                                 write_u8
"zero?",                                    is_zero
