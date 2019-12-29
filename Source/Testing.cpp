// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Console.h"
#include "Interpreter.h"

void CheckOutput(Interpreter& lisp, const char* source, const char* expectedOutput, ErrorCode expectedError = ERROR_NONE)
{
    ErrorCode caughtError = ERROR_NONE;
    string output;

    try
    {
        output = lisp.Evaluate(source);
    }
    catch (YellowError error)
    {
        caughtError = error._code;
    }

    string shouldHave;
    string actually;

    std::stringstream ss;
    ss << '\"' << source << '\"';

    if (expectedOutput)
    {
        if (output == expectedOutput)
            return;

        ss << " should output \"" << expectedOutput << "\", ";
    }
    else if (expectedError)
    {
        if (caughtError == expectedError)
            return;

        ss << " should raise error " << expectedError << " (" << YellowError::GetDesc(expectedError) << "), ";
    }
    else
        assert(0);

    if(caughtError)
        ss << "but raised error " << caughtError << " (" << YellowError::GetDesc(caughtError) << ")";
    else 
        ss << "but output \"" << output << "\"";

    SetTextColor(ANSI_RED);
    printf("SANITY CHECK FAILED: "); 
    ResetTextColor();

    printf("% s\n", ss.str().c_str());
}

void CheckOutput(Interpreter& lisp, const char* source, ErrorCode expectedError)
{
    CheckOutput(lisp, source, NULL, expectedError);
}

#define VERIFY(_IN, _OUT) CheckOutput(lisp, _IN, _OUT)

void SanityCheck()
{
    InterpreterSettings settings;
    settings._repl = false;
    settings._catchExceptions = false;

    Interpreter lisp(&settings);
    
    lisp.Evaluate("(setq x 123)");

    VERIFY("`(x x)", "(x x)");
    VERIFY("`(x ,x)", "(x 123)");
    VERIFY("`(x ',x)", "(x '123)");
    VERIFY("`(x ,'x)", "(x x)");

    VERIFY("`(x ,'`(x ,'x))", "(x `(x ,'x))"); 
    VERIFY("`(x `(x x))", "(x `(x x))"); 
    VERIFY("`(x ,`(x x))", "(x (x x))"); 
    VERIFY("`(x ',`(x x))", "(x '(x x))"); 
    VERIFY("`(x ,'`(x x))", "(x `(x x))"); 
    VERIFY("`(x `(x ,x))", "(x `(x ,x))"); 
    VERIFY("`(x ,`(x ,x))", "(x (x 123))"); 
    VERIFY("`(x ',`(x ,x))", "(x '(x 123))"); 
    VERIFY("`(x ,'`(x ,x))", "(x `(x ,x))"); 
    VERIFY("`(x `(x ',x))", "(x `(x ',x))"); 
    VERIFY("`(x ,`(x ',x))", "(x (x '123))"); 
    VERIFY("`(x ',`(x ',x))", "(x '(x '123))"); 
    VERIFY("`(x ,'`(x ',x))", "(x `(x ',x))"); 
    VERIFY("`(x `(x ,'x))", "(x `(x ,'x))"); 
    VERIFY("`(x ,`(x ,'x))", "(x (x x))"); 
    VERIFY("`(x ',`(x ,'x))", "(x '(x x))"); 
    VERIFY("`(x ,'`(x ,'x))", "(x `(x ,'x))"); 

    VERIFY("`(a `(b ,(+ 1 2) ,(foo ,(+ 1 3) d) e) f)", "(a `(b ,(+ 1 2) ,(foo 4 d) e) f)");





    VERIFY("", "");
    VERIFY(";(", "");
    VERIFY("\n", "");
    VERIFY("1", "1");
    VERIFY("-1", "-1");
    VERIFY(" 2 ", "2");
    VERIFY("3.4", "3.4");
    VERIFY("-5.6", "-5.6");
    VERIFY("78e-3", "0.078");
    VERIFY("9 ; (", "9");

    VERIFY("t", "t");
    VERIFY("nil", "nil");
    VERIFY("'nil", "nil");
    VERIFY("()", "nil");
    VERIFY("'()", "nil");

    VERIFY("'foo", "foo");
    VERIFY("'FOO", "FOO");
    VERIFY("\"foo\"", "\"foo\"");
    VERIFY("\"FOO\"", "\"FOO\"");

    VERIFY("(quote ('1 2))", "((quote 1) 2)");
    VERIFY("'('1 2)", "((quote 1) 2)");

    VERIFY("(atom nil)", "t");
    VERIFY("(atom ())", "t");
    VERIFY("(atom '())", "t");
    VERIFY("(atom 3)", "t");
    VERIFY("(atom 'foo)", "t");
    VERIFY("(atom (atom 3))", "t");
    VERIFY("(atom [atom 3])", "t");
    VERIFY("[atom [atom 3]]", "t");
    VERIFY("[atom (atom 3)]", "t");
    VERIFY("(atom '(atom 3))", "nil");
    VERIFY("(atom (list 1 2))", "nil");
    VERIFY("(atom (cons 1 2))", "nil");

    VERIFY("(car (list 1 2))", "1");
    VERIFY("(cdr (list 1 2))", "(2)");
    VERIFY("(car ())", "nil");
    VERIFY("(cdr ())", "nil");
    VERIFY("(car (cdr (list 1 2 3)))", "2");
    VERIFY("(cdr (cdr (list 1 2 3)))", "(3)");

    VERIFY("(list)", "nil");
    VERIFY("(list 1)", "(1)");
    VERIFY("(list 1 'foo 3)", "(1 foo 3)");
    VERIFY("(list (list ()))", "((nil))");

    VERIFY("(setq x (list 4 5 6))", "(4 5 6)");
    VERIFY("(setq x '(4 5 6))", "(4 5 6)");
    VERIFY("(atom x)", "nil");
    VERIFY("(atom 'x)", "t");
    VERIFY("(setq y x)", "(4 5 6)");
    VERIFY("(setq y 'x)", "x");
    VERIFY("(atom y)", "t");
    VERIFY("y", "x");
    VERIFY("(eval y)", "(4 5 6)");

    VERIFY("(= 1 1)", "t");
    VERIFY("(= 1 2)", "nil");
    VERIFY("(= 1 1.0)", "t");
    VERIFY("(= (+ 1 2) 3)", "t");
    VERIFY("(= \"foo\" \"bar\")", "nil");
    VERIFY("(= \"foo\" \"foo\")", "t");
    VERIFY("(= \"foo\" \"FOO\")", "nil");
    VERIFY("(progn (setq a 123) (= a 123)", "t");
    VERIFY("(= a a)", "t");
    VERIFY("(progn (setq b a) (= a b))", "t");
    VERIFY("(progn (setq b 'a) (= a b))", "t");
    VERIFY("(= b 123)", "t");
    VERIFY("(= () ())", "t");
    VERIFY("(= () nil)", "t");
    VERIFY("(= '(a) '(b))", "t");
    VERIFY("(= '(1 2 3) '(1 2 3))", "t");
    VERIFY("(= '(1 (2 3) 4) '(1 (2 3) 4))", "t");
    VERIFY("(= (atom 'foo) (atom 'bar))", "nil");
    VERIFY("(= 'foo 'foo)", "t");
    VERIFY("(= 'foo 'bar)", "nil");

    VERIFY("(< 1 2)", "t");
    VERIFY("(< 2 1)", "nil");
    VERIFY("(< 1 1)", "nil");
    VERIFY("(< -2 -1)", "t");
    VERIFY("(< -1 -2)", "nil");
    VERIFY("(< 1.1 2.1)", "t");
    VERIFY("(< 1 2.1)", "t");
    VERIFY("(< -1.1 2)", "t");

    VERIFY("(defun sqr (x) (* x x))", "sqr");
    VERIFY("(sqr 5)", "25");

    VERIFY("(cons 'a 'b)", "(a . b)");
    VERIFY("(cons (list 'a) 'b)", "((a) . b)");
    VERIFY("(cons 'a (list 'b))", "(a b)");
    VERIFY("(cons (list 'a) (list 'b))", "((a) b)");

    VERIFY("(list 1 . (2))", "(1 . 2)");
    VERIFY("(list 'a 'b . ('c 'd 'e . ()))", "(a b c d e)");

    // Need test cases:
    //  unquote
    //  quasiquote
    //  defmacro
    //  defun
    //  lambda
    //  setq
    //  cond
    //  eq
    //  eval
    //  progn
    //  + - * / %

    //
    //
    // `(,a b)
    // (qq ((uq a) b)



    //
    /*

    (defmacro if (test then else) `(cond (,test ,then) (T ,else)))

    `(cond (,test ,then) (T ,else))
    (qq (cond ((uq test) (uq then)) (T (uq else))))

    ((q cond) ((test then) (T else))

    (if ((< 1 3) 1 2))
    `(cond (,test foo ,then) (T ,else))
    (qq (cond ((uq test) foo (uq then)) (T (uq else))))
    ((q cond) ((test (q foo) then) (T else))
    (cond (((eval (< 1 3)) foo

    qq has one parameter
    recurse
    first level every list element that is not quoted, gets quoted
    every element (uq foo)
    elements of the form (q foo) become (q (q foo))

    every element that is not uq gets q, and the uq are elided



    qq sets q state on
    recurse in


    */


    //VERIFY("(append '(a b c) '())", "(a b c)");
    //VERIFY("(append '() '(a b c))", "(a b c)");
    //VERIFY("(append '(a b) '(c d))", "(a b c d)");
    //VERIFY("(append '(a b) 'c)", "(a b . c)");
    //VERIFY("(append '(a b c) '(d e f) '() '(g))", "(a b c d e f g)");
    //VERIFY("(append '(a b c) 'd)", "(a b c . d)");
    //VERIFY("(setq lst '(a b c))", "(a b c)");
    //VERIFY("(append lst '(d))", "(a b c d)");
    //VERIFY("lst", "(a b c)");
    //VERIFY("(append)", "nil");
    //VERIFY("(append 'a)", "a");

    //

    // TODO: dot notation
    // TODO: backquote

    // The error section needs a *lot* more test cases

    VERIFY("\"foo",       ERROR_PARSER_STRING_UNTERMINATED);
    VERIFY("(",           ERROR_PARSER_LIST_UNTERMINATED);
    VERIFY("(setq } 3)",  ERROR_PARSER_INVALID_IDENTIFIER);
    VERIFY("(setq quote 3)", ERROR_RUNTIME_RESERVED_SYMBOL);
    VERIFY("(foo 1 2)",   ERROR_RUNTIME_UNDEFINED_FUNCTION);
    VERIFY("(atom 3]",    ERROR_PARSER_BRACE_MISMATCH);
    VERIFY("(atom [3)]",  ERROR_PARSER_BRACE_MISMATCH);
    VERIFY("(quote 1 2)", ERROR_RUNTIME_WRONG_NUM_PARAMS);
    VERIFY("(atom unbound)", ERROR_RUNTIME_VARIABLE_UNBOUND);
    VERIFY("(< 1 nil)",   ERROR_RUNTIME_TYPE_MISMATCH);
}


/*

*/
