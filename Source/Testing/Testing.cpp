// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Console.h"
#include "Interpreter.h"
#include "Coverage.h"

#if DEBUG_BUILD
    CoverageTrackerType gCoverageTracker;
    CoverageMarker gCoverageMarker[MAX_COVERAGE_MARKERS];
#endif

bool CheckOutput(Console* console, Interpreter& lisp, const char* source, const char* expectedOutput, ErrorCode expectedError = ERROR_NONE)
{
    ErrorCode caughtError = ERROR_NONE;
    string output;

    /*
    if (expectedOutput)
        console->PrintDebug("%-40s =>  %s\n", source, expectedOutput);
    else
        console->PrintDebug("%-40s =>  %s\n", source, YellowError::GetDesc(expectedError));
    */

    try
    {
        output = lisp.Evaluate(source);
    }
    catch (YellowError error)
    {
        caughtError = error._code;
    }
    catch (...)
    {
        console->PrintErrorPrefix("INTERNAL ERROR");
        console->Print("unhandled exception!\n");
    }

    string shouldHave;
    string actually;

    std::stringstream ss;
    ss << '\"' << source << '\"';

    if (expectedOutput)
    {
        if (output == expectedOutput)
            return true;

        ss << " should output \"" << expectedOutput << "\", ";
    }
    else if (expectedError)
    {
        if (caughtError == expectedError)
            return true;

        ss << " should raise error " << expectedError << " (" << YellowError::GetDesc(expectedError) << "), ";
    }
    else
        assert(0);

    if(caughtError)
        ss << "but raised error " << caughtError << " (" << YellowError::GetDesc(caughtError) << ")";
    else 
        ss << "but output \"" << output << "\"";

    console->PrintErrorPrefix("INTERNAL TEST FAILED");
    console->Print("% s\n", ss.str().c_str());

    static int sRetryOnError = 1;
    if (sRetryOnError)
        CheckOutput(console, lisp, source, expectedOutput, expectedError);

    return false;
}

void CheckOutput(Console* console, Interpreter& lisp, const char* source, ErrorCode expectedError)
{
    CheckOutput(console, lisp, source, nullptr, expectedError);
}

#define VERIFY(_IN, _OUT) CheckOutput(console, lisp, _IN, _OUT)

void SanityCheck(Console* console)
{
    InterpreterSettings settings;
    settings._repl = false;
    settings._catchExceptions = false;

    Interpreter lisp(console, &settings);
    
    // (progn (defmacro foo (x) `(+ ,x 1)) (defmacro bar (x) `(* ,x (foo ,x))) (bar 123)) => 15252

    VERIFY("(setq x '(4 5 6))", "(4 5 6)");
    VERIFY("(atom x)", "#f");

    VERIFY("(setq x 123)", "123");
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

    VERIFY("#t", "#t");
    VERIFY("#f", "#f");
    VERIFY("'nil", "#f");
    VERIFY("()", "#f");
    VERIFY("'()", "#f");

    VERIFY("'foo", "foo");
    VERIFY("'FOO", "FOO");
    VERIFY("\"foo\"", "\"foo\"");
    VERIFY("\"FOO\"", "\"FOO\"");

    VERIFY("(quote ('1 2))", "('1 2)");
    VERIFY("'('1 2)", "('1 2)");

    VERIFY("(atom nil)", "#t");
    VERIFY("(atom ())", "#t");
    VERIFY("(atom '())", "#t");
    VERIFY("(atom 3)", "#t");
    VERIFY("(atom 'foo)", "#t");
    VERIFY("(atom (atom 3))", "#t");
    VERIFY("(atom [atom 3])", ERROR_PARSER_INVALID_IDENTIFIER);
    VERIFY("[atom [atom 3]]", ERROR_PARSER_INVALID_IDENTIFIER);
    VERIFY("[atom (atom 3)]", ERROR_PARSER_INVALID_IDENTIFIER);
    VERIFY("(atom '(atom 3))", "#f");
    VERIFY("(atom (list 1 2))", "#f");
    VERIFY("(atom (cons 1 2))", "#f");

    VERIFY("(car (list 1 2))", "1");
    VERIFY("(cdr (list 1 2))", "(2)");
    VERIFY("(car ())", "#f");
    VERIFY("(cdr ())", "#f");
    VERIFY("(car (cdr (list 1 2 3)))", "2");
    VERIFY("(cdr (cdr (list 1 2 3)))", "(3)");

    VERIFY("(list)", "#f");
    VERIFY("(list 1)", "(1)");
    VERIFY("(list 1 'foo 3)", "(1 foo 3)");
    VERIFY("(list (list ()))", "((nil))");

    VERIFY("(setq x (list 4 5 6))", "(4 5 6)");
    VERIFY("(setq x '(4 5 6))", "(4 5 6)");
    VERIFY("(atom x)", "#f");
    VERIFY("(atom 'x)", "#t");
    VERIFY("(setq y x)", "(4 5 6)");
    VERIFY("(setq y 'x)", "x");
    VERIFY("(atom y)", "#t");
    VERIFY("y", "x");
    VERIFY("(eval y)", "(4 5 6)");

    VERIFY("(= 1 1)", "#t");
    VERIFY("(= 1 2)", "#f");
    VERIFY("(= 1 1.0)", "#t");
    VERIFY("(= (+ 1 2) 3)", "#t");
    VERIFY("(= \"foo\" \"bar\")", ERROR_RUNTIME_INVALID_ARGUMENT);
    VERIFY("(progn (setq a 123) (= a 123))", "#t");
    VERIFY("(= a a)", "#t");
    VERIFY("(progn (setq b a) (= a b))", "#t");
    VERIFY("(progn (setq b 'a) (= a b))", ERROR_RUNTIME_INVALID_ARGUMENT);
    VERIFY("(= b 123)", ERROR_RUNTIME_INVALID_ARGUMENT);
    VERIFY("(= () ())", ERROR_RUNTIME_INVALID_ARGUMENT);
    VERIFY("(eq () ())", "#t");
    VERIFY("(eq () nil)", "#t");
    VERIFY("(eq '(a) '(b))", "#f");
    VERIFY("(eq '(1 2 3) '(1 2 3))", "#f");
    VERIFY("(eql '(1 2 3) '(1 2 3))", "#f");
    VERIFY("(equal '(1 2 3) '(1 2 3))", "#t");
    VERIFY("(equal '(1 (2 3) 4) '(1 (2 3) 4))", "#t");
    VERIFY("(equal '(1 (2 3) 4) '(1 2 (3 4)))", "#f");
    VERIFY("(eq (atom 'foo) (atom 'bar))", "#t");
    VERIFY("(eq 'foo 'foo)", "#t");
    VERIFY("(eq 'foo 'bar)", "#f");

    VERIFY("(= 3)", "#t");                
    VERIFY("(= 3 3.0)", "#t"); 
    VERIFY("(= 3 3 3 3)", "#t");                 
    VERIFY("(= 3 3 5 3)", "#f");                
    VERIFY("(= 3 6 5 2)", "#f");                
    VERIFY("(= 3 2 3)", "#f");              
    VERIFY("(= 0.0 -0.0)", "#t");
    VERIFY("(= 0 -0.0)", "#t");            
    VERIFY("(< 3 5)", "#t");                  
    VERIFY("(< 3 -5)", "#f");                
    VERIFY("(< 3 3)", "#f");                 
    VERIFY("(< 0.0 -0.0)", "#f");
    VERIFY("(<= 3 5)", "#t");          
    VERIFY("(<= 3 -5)", "#f");        
    VERIFY("(<= 3 3)", "#t");          
    VERIFY("(> 4 3)", "#t");             
    VERIFY("(> 0.0 -0.0)", "#f");
    VERIFY("(>= 4 3)", "#t");       

    VERIFY("(eq 'a 'b)", "#f");
    VERIFY("(eq 'a 'a)", "#t");
    VERIFY("(eq 3 3.0)", "#f");
    VERIFY("(eq (cons 'a 'b) (cons 'a 'c))", "#f");
    VERIFY("(eq (cons 'a 'b) (cons 'a 'b))", "#f");
    VERIFY("(progn (setq x (cons 'a 'b)) (eq x x))", "#t");
    VERIFY("(progn (setq x '(a . b)) (eq x x))", "#t");
    VERIFY("(eq \"FOO\" \"foo\")", "#f");

    VERIFY("(eql 'a 'b)", "#f");
    VERIFY("(eql 'a 'a)", "#t");
    VERIFY("(eql 3 3)", "#t");
    VERIFY("(eql 3 3.0)", "#f");
    VERIFY("(eql 3.0 3.0)", "#t");
    VERIFY("(eql (cons 'a 'b) (cons 'a 'c))", "#f");
    VERIFY("(eql (cons 'a 'b) (cons 'a 'b))", "#f");
    VERIFY("(progn (setq x (cons 'a 'b)) (eql x x))", "#t");
    VERIFY("(progn (setq x '(a . b)) (eql x x))", "#t");
    VERIFY("(eql \"FOO\" \"foo\")", "#f");

    VERIFY("(equal 'a 'b)", "#f");
    VERIFY("(equal 'a 'a)", "#t");
    VERIFY("(equal 3 3)", "#t");
    VERIFY("(equal 3 3.0)", "#f");
    VERIFY("(equal 3.0 3.0)", "#t");
    VERIFY("(equal (cons 'a 'b) (cons 'a 'c))", "#f");
    VERIFY("(equal (cons 'a 'b) (cons 'a 'b))", "#t");
    VERIFY("(equal \"Foo\" \"Foo\")", "#t");
    VERIFY("(equal \"FOO\" \"foo\")", "#f");
    VERIFY("(equal \"This-string\" \"This-string\")", "#t");
    VERIFY("(equal \"This-string\" \"this-string\")", "#f");

    VERIFY("(equalp 'a 'b)", "#f");
    VERIFY("(equalp 'a 'a)", "#t");
    VERIFY("(equalp 3 3)", "#t");
    VERIFY("(equalp 3 3.0)", "#t");
    VERIFY("(equalp 3.0 3.0)", "#t");
    VERIFY("(equalp (cons 'a 'b) (cons 'a 'c))", "#f");
    VERIFY("(equalp (cons 'a 'b) (cons 'a 'b))", "#t");
    VERIFY("(equalp \"Foo\" \"Foo\")", "#t");
    VERIFY("(equalp \"FOO\" \"foo\")", "#t");

    VERIFY("(< 1 2)", "#t");
    VERIFY("(< 2 1)", "#f");
    VERIFY("(< 1 1)", "#f");
    VERIFY("(< -2 -1)", "#t");
    VERIFY("(< -1 -2)", "#f");
    VERIFY("(< 1.1 2.1)", "#t");
    VERIFY("(< 1 2.1)", "#t");
    VERIFY("(< -1.1 2)", "#t");

    VERIFY("(defun sqr (x) (* x x))", "sqr");
    VERIFY("(sqr 5)", "25");

    VERIFY("(cons 1 2)", "(1 . 2)");
    VERIFY("(cons 1 '2)", "(1 . 2)");
    VERIFY("(cons 1 '(2))", "(1 2)");
    VERIFY("(cons 1 nil)", "(1)");
    VERIFY("(cons nil 2)", "(nil . 2)");
    VERIFY("(cons nil nil)", "(nil)");
    VERIFY("(cons 1 (cons 2 (cons 3 (cons 4 nil))))", "(1 2 3 4)");
    VERIFY("(cons 'a 'b)", "(a . b)");
    VERIFY("(cons 'a (cons 'b (cons 'c '())))", "(a b c)");
    VERIFY("(cons 'a '(b c d))", "(a b c d)");

    VERIFY("(cons (list 'a) 'b)", "((a) . b)");
    VERIFY("(cons 'a (list 'b))", "(a b)");
    VERIFY("(cons (list 'a) (list 'b))", "((a) b)");

    VERIFY("(cons 1 . (2))", "(1 . 2)");
    VERIFY("(list 1 . (2))", "(1 2)");
    VERIFY("(list (cons 1 2))", "((1 . 2))");
    VERIFY("(list (cons 1 2) (cons 3 4))", "((1 . 2) (3 . 4))");
    VERIFY("(list 'a 'b . ('c 'd 'e . ()))", "(a b c d e)");


    //VERIFY("(let ((x \"Foo\")) (eq x x))", "#t");

    //VERIFY("(append '(a b c) '())", "(a b c)");
    //VERIFY("(append '() '(a b c))", "(a b c)");
    //VERIFY("(append '(a b) '(c d))", "(a b c d)");
    //VERIFY("(append '(a b) 'c)", "(a b . c)");
    //VERIFY("(append '(a b c) '(d e f) '() '(g))", "(a b c d e f g)");
    //VERIFY("(append '(a b c) 'd)", "(a b c . d)");
    //VERIFY("(setq lst '(a b c))", "(a b c)");
    //VERIFY("(append lst '(d))", "(a b c d)");
    //VERIFY("lst", "(a b c)");
    //VERIFY("(append)", "#f");
    //VERIFY("(append 'a)", "a");

    // A lot more error test cases are needed everywhere

    VERIFY("\"foo",       ERROR_PARSER_STRING_UNTERMINATED);
    VERIFY("(",           ERROR_PARSER_LIST_UNTERMINATED);
    VERIFY("(setq } 3)",  ERROR_PARSER_INVALID_IDENTIFIER);
    VERIFY("(setq quote 3)", ERROR_RUNTIME_RESERVED_SYMBOL);
    VERIFY("(foo 1 2)",   ERROR_RUNTIME_UNDEFINED_FUNCTION);
    VERIFY("(quote 1 2)", ERROR_RUNTIME_WRONG_NUM_PARAMS);
    VERIFY("(atom unbound)", ERROR_RUNTIME_VARIABLE_UNBOUND);
    VERIFY("(< 1 nil)",   ERROR_RUNTIME_TYPE_MISMATCH);

    static volatile int never = 0;
    RAISE_ERROR_IF(never, ERROR_RUNTIME_TYPE_MISMATCH, "");

#if DEBUG_BUILD
    gCoverageTracker.RefreshCoverage();
#endif
}


/*
(defun fib (n) (if (< n 2) n (+ (fib (- n 1)) (fib (- n 2)))))
*/


