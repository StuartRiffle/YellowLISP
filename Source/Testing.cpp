// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Console.h"
#include "Interpreter.h"

void CheckOutput(Interpreter& lisp, const char* source, const char* expectedOutput, ErrorCode expectedError = ERROR_NONE)
{
    ErrorCode caughtError = ERROR_NONE;
    string output;

    //if (expectedOutput)
    //    std::cout << source << " => " << expectedOutput << std::endl;

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
        SetTextColor(ANSI_RED);
        printf("UNHANDLED EXCEPTION!\n"); 
        ResetTextColor();
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

    static int sRetryOnError = 0;
    if (sRetryOnError)
        CheckOutput(lisp, source, expectedOutput, expectedError);
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

    VERIFY("t", "t");
    VERIFY("nil", "nil");
    VERIFY("'nil", "nil");
    VERIFY("()", "nil");
    VERIFY("'()", "nil");

    VERIFY("'foo", "foo");
    VERIFY("'FOO", "FOO");
    VERIFY("\"foo\"", "\"foo\"");
    VERIFY("\"FOO\"", "\"FOO\"");

    VERIFY("(quote ('1 2))", "('1 2)");
    VERIFY("'('1 2)", "('1 2)");

    VERIFY("(atom nil)", "t");
    VERIFY("(atom ())", "t");
    VERIFY("(atom '())", "t");
    VERIFY("(atom 3)", "t");
    VERIFY("(atom 'foo)", "t");
    VERIFY("(atom (atom 3))", "t");
    VERIFY("(atom [atom 3])", ERROR_PARSER_INVALID_IDENTIFIER);
    VERIFY("[atom [atom 3]]", ERROR_PARSER_INVALID_IDENTIFIER);
    VERIFY("[atom (atom 3)]", ERROR_PARSER_INVALID_IDENTIFIER);
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
    VERIFY("(= \"foo\" \"bar\")", ERROR_RUNTIME_INVALID_ARGUMENT);
    VERIFY("(progn (setq a 123) (= a 123))", "t");
    VERIFY("(= a a)", "t");
    VERIFY("(progn (setq b a) (= a b))", "t");
    VERIFY("(progn (setq b 'a) (= a b))", ERROR_RUNTIME_INVALID_ARGUMENT);
    VERIFY("(= b 123)", ERROR_RUNTIME_INVALID_ARGUMENT);
    VERIFY("(= () ())", ERROR_RUNTIME_INVALID_ARGUMENT);
    VERIFY("(eq () ())", "t");
    VERIFY("(eq () nil)", "t");
    VERIFY("(eq '(a) '(b))", "nil");
    VERIFY("(eq '(1 2 3) '(1 2 3))", "nil");
    VERIFY("(eql '(1 2 3) '(1 2 3))", "nil");
    VERIFY("(equal '(1 2 3) '(1 2 3))", "t");
    VERIFY("(equal '(1 (2 3) 4) '(1 (2 3) 4))", "t");
    VERIFY("(equal '(1 (2 3) 4) '(1 2 (3 4)))", "nil");
    VERIFY("(eq (atom 'foo) (atom 'bar))", "t");
    VERIFY("(eq 'foo 'foo)", "t");
    VERIFY("(eq 'foo 'bar)", "nil");

    VERIFY("(= 3)", "t");                
    VERIFY("(= 3 3.0)", "t"); 
    VERIFY("(= 3 3 3 3)", "t");                 
    VERIFY("(= 3 3 5 3)", "nil");                
    VERIFY("(= 3 6 5 2)", "nil");                
    VERIFY("(= 3 2 3)", "nil");              
    VERIFY("(= 0.0 -0.0)", "t");
    VERIFY("(= 0 -0.0)", "t");            
    VERIFY("(< 3 5)", "t");                  
    VERIFY("(< 3 -5)", "nil");                
    VERIFY("(< 3 3)", "nil");                 
    VERIFY("(< 0.0 -0.0)", "nil");
    VERIFY("(<= 3 5)", "t");          
    VERIFY("(<= 3 -5)", "nil");        
    VERIFY("(<= 3 3)", "t");          
    VERIFY("(> 4 3)", "t");             
    VERIFY("(> 0.0 -0.0)", "nil");
    VERIFY("(>= 4 3)", "t");       

    VERIFY("(eq 'a 'b)", "nil");
    VERIFY("(eq 'a 'a)", "t");
    VERIFY("(eq 3 3.0)", "nil");
    VERIFY("(eq (cons 'a 'b) (cons 'a 'c))", "nil");
    VERIFY("(eq (cons 'a 'b) (cons 'a 'b))", "nil");
    VERIFY("(progn (setq x (cons 'a 'b)) (eq x x))", "t");
    VERIFY("(progn (setq x '(a . b)) (eq x x))", "t");
    VERIFY("(eq \"FOO\" \"foo\")", "nil");

    VERIFY("(eql 'a 'b)", "nil");
    VERIFY("(eql 'a 'a)", "t");
    VERIFY("(eql 3 3)", "t");
    VERIFY("(eql 3 3.0)", "nil");
    VERIFY("(eql 3.0 3.0)", "t");
    VERIFY("(eql (cons 'a 'b) (cons 'a 'c))", "nil");
    VERIFY("(eql (cons 'a 'b) (cons 'a 'b))", "nil");
    VERIFY("(progn (setq x (cons 'a 'b)) (eql x x))", "t");
    VERIFY("(progn (setq x '(a . b)) (eql x x))", "t");
    VERIFY("(eql \"FOO\" \"foo\")", "nil");

    VERIFY("(equal 'a 'b)", "nil");
    VERIFY("(equal 'a 'a)", "t");
    VERIFY("(equal 3 3)", "t");
    VERIFY("(equal 3 3.0)", "nil");
    VERIFY("(equal 3.0 3.0)", "t");
    VERIFY("(equal (cons 'a 'b) (cons 'a 'c))", "nil");
    VERIFY("(equal (cons 'a 'b) (cons 'a 'b))", "t");
    VERIFY("(equal \"Foo\" \"Foo\")", "t");
    VERIFY("(equal \"FOO\" \"foo\")", "nil");
    VERIFY("(equal \"This-string\" \"This-string\")", "t");
    VERIFY("(equal \"This-string\" \"this-string\")", "nil");

    VERIFY("(equalp 'a 'b)", "nil");
    VERIFY("(equalp 'a 'a)", "t");
    VERIFY("(equalp 3 3)", "t");
    VERIFY("(equalp 3 3.0)", "t");
    VERIFY("(equalp 3.0 3.0)", "t");
    VERIFY("(equalp (cons 'a 'b) (cons 'a 'c))", "nil");
    VERIFY("(equalp (cons 'a 'b) (cons 'a 'b))", "t");
    VERIFY("(equalp \"Foo\" \"Foo\")", "t");
    VERIFY("(equalp \"FOO\" \"foo\")", "t");

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


    //    VERIFY("(let ((x \"Foo\")) (eq x x))", "t");

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

    // A lot more error test cases are needed everywhere

    VERIFY("\"foo",       ERROR_PARSER_STRING_UNTERMINATED);
    VERIFY("(",           ERROR_PARSER_LIST_UNTERMINATED);
    VERIFY("(setq } 3)",  ERROR_PARSER_INVALID_IDENTIFIER);
    VERIFY("(setq quote 3)", ERROR_RUNTIME_RESERVED_SYMBOL);
    VERIFY("(foo 1 2)",   ERROR_RUNTIME_UNDEFINED_FUNCTION);
    VERIFY("(quote 1 2)", ERROR_RUNTIME_WRONG_NUM_PARAMS);
    VERIFY("(atom unbound)", ERROR_RUNTIME_VARIABLE_UNBOUND);
    VERIFY("(< 1 nil)",   ERROR_RUNTIME_TYPE_MISMATCH);
}


/*
(defun fib (n) (if (< n 2) n (+ (fib (- n 1)) (fib (- n 2)))))
*/


