;;;; YellowLISP
;;;; Copyright (C) 2020 Stuart Riffle

(defmacro if (test then else) `(cond (,test ,then) (t ,else)))
(defmacro defvar (n v) `(setq n ,v))

(defun null   (x)   (eq x nil))
(defun and    (a b) (cond (a (cond (b t)))))
(defun or     (a b) (cond (a t) (b t)))
(defun not    (x)   (cond (x nil) (t t)))
(defun >      (a b) (< b a))
(defun <=     (a b) (cond ((< b a) nil) (t t)))
(defun >=     (a b) (cond ((< a b) nil) (t t)))
(defun /=     (a b) (cond ((= a b) nil) (t t)))

(defun caar   (x) (car (car   x)))
(defun cadr   (x) (car (cdr   x)))
(defun cdar   (x) (cdr (car   x)))
(defun cddr   (x) (cdr (cdr   x)))
(defun caaar  (x) (car (caar  x)))
(defun caadr  (x) (car (cadr  x)))
(defun cadar  (x) (car (cdar  x)))
(defun caddr  (x) (car (cddr  x)))
(defun cdaar  (x) (cdr (caar  x)))
(defun cdadr  (x) (cdr (cadr  x)))
(defun cddar  (x) (cdr (cdar  x)))
(defun cdddr  (x) (cdr (cddr  x)))
(defun caaaar (x) (car (caaar x)))
(defun caaadr (x) (car (caadr x)))
(defun caadar (x) (car (cadar x)))
(defun caaddr (x) (car (caddr x)))
(defun cadaar (x) (car (cdaar x)))
(defun cadadr (x) (car (cdadr x)))
(defun caddar (x) (car (cddar x)))
(defun cadddr (x) (car (cdddr x)))
(defun cdaaar (x) (cdr (caaar x)))
(defun cdaadr (x) (cdr (caadr x)))
(defun cdadar (x) (cdr (cadar x)))
(defun cdaddr (x) (cdr (caddr x)))
(defun cddaar (x) (cdr (cdaar x)))
(defun cddadr (x) (cdr (cdadr x)))
(defun cdddar (x) (cdr (cddar x)))
(defun cddddr (x) (cdr (cdddr x)))

(defun sum (x) (cond ((< x 1) 0) (t (+ x (sum (- x 1))))))


(define (mymap func elems) 
    (cond 
        ((null? elems) '())
        (else (cons (func (car elems)) (mymap func (cdr elems))))))
        
        

(define (mymap func elems) (cond  ((null? elems) '())   (else (cons (func (car elems)) (mymap func (cdr elems))))))

(defun mymap (func elems) (cond  ((nullp elems) '())   (t (cons (func (car elems)) (mymap func (cdr elems))))))


(defun mymap (func elems) (apply func (car elems)))

        
                
(define (mymap func elems) (cond  ((null? elems) '())   (#t (mymap func (cdr elems)))))



        

(define (my-last f L)
  (cond ((null? L) #f)
        ((null? (cdr L)) (f (car L)))
        (else (my-last f (cdr L)))))



(test-case "(setq x 123)"                            "123")

(test-case "`(x x)"                                  "(x x)")
(test-case "`(x ,x)"                                 "(x 123)")
(test-case "`(x ',x)"                                "(x '123)")
(test-case "`(x ,'x)"                                "(x x)")
(test-case "`(x ,'`(x ,'x))"                         "(x `(x ,'x))")
(test-case "`(x `(x x))"                             "(x `(x x))")
(test-case "`(x ,`(x x))"                            "(x (x x))")
(test-case "`(x ',`(x x))"                           "(x '(x x))")
(test-case "`(x ,'`(x x))"                           "(x `(x x))")
(test-case "`(x `(x ,x))"                            "(x `(x ,x))")
(test-case "`(x ,`(x ,x))"                           "(x (x 123))")
(test-case "`(x ',`(x ,x))"                          "(x '(x 123))")
(test-case "`(x ,'`(x ,x))"                          "(x `(x ,x))")
(test-case "`(x `(x ',x))"                           "(x `(x ',x))")
(test-case "`(x ,`(x ',x))"                          "(x (x '123))")
(test-case "`(x ',`(x ',x))"                         "(x '(x '123))")
(test-case "`(x ,'`(x ',x))"                         "(x `(x ',x))")
(test-case "`(x `(x ,'x))"                           "(x `(x ,'x))")
(test-case "`(x ,`(x ,'x))"                          "(x (x x))")
(test-case "`(x ',`(x ,'x))"                         "(x '(x x))")
(test-case "`(x ,'`(x ,'x))"                         "(x `(x ,'x))")
(test-case ""                                        "")
(test-case ";("                                      "")
(test-case "1"                                       "1")
(test-case "-1"                                      "-1")
(test-case " 2 "                                     "2")
(test-case "3.4"                                     "3.4")
(test-case "-5.6"                                    "-5.6")
(test-case "78e-3"                                   "0.078")
(test-case "9 ; ("                                   "9")
(test-case "t"                                       "t")
(test-case "nil"                                     "nil")
(test-case "'nil"                                    "nil")
(test-case "()"                                      "nil")
(test-case "'()"                                     "nil")
(test-case "'foo"                                    "foo")
(test-case "'FOO"                                    "FOO")
(test-case "\"foo\""                                 "\"foo\"")
(test-case "\"FOO\""                                 "\"FOO\"")
(test-case "(quote ('1 2))"                          "('1 2)")
(test-case "'('1 2)"                                 "('1 2)")
(test-case "(atom nil)"                              "t")
(test-case "(atom ())"                               "t")
(test-case "(atom '())"                              "t")
(test-case "(atom 3)"                                "t")
(test-case "(atom 'foo)"                             "t")
(test-case "(atom (atom 3))"                         "t")
(test-case "(atom [atom 3])"                         ERROR_PARSER_INVALID_IDENTIFIER)
(test-case "[atom [atom 3]]"                         ERROR_PARSER_INVALID_IDENTIFIER)
(test-case "[atom (atom 3)]"                         ERROR_PARSER_INVALID_IDENTIFIER)
(test-case "(atom '(atom 3))"                        "nil")
(test-case "(atom (list 1 2))"                       "nil")
(test-case "(atom (cons 1 2))"                       "nil")
(test-case "(car (list 1 2))"                        "1")
(test-case "(cdr (list 1 2))"                        "(2)")
(test-case "(car ())"                                "nil")
(test-case "(cdr ())"                                "nil")
(test-case "(car (cdr (list 1 2 3)))"                "2")
(test-case "(cdr (cdr (list 1 2 3)))"                "(3)")
(test-case "(list)"                                  "nil")
(test-case "(list 1)"                                "(1)")
(test-case "(list 1 'foo 3)"                         "(1 foo 3)")
(test-case "(list (list ()))"                        "((nil))")
(test-case "(setq x (list 4 5 6))"                   "(4 5 6)")
(test-case "(setq x '(4 5 6))"                       "(4 5 6)")
(test-case "(atom x)"                                "nil")
(test-case "(atom 'x)"                               "t")
(test-case "(setq y x)"                              "(4 5 6)")
(test-case "(setq y 'x)"                             "x")
(test-case "(atom y)"                                "t")
(test-case "y"                                       "x")
(test-case "(eval y)"                                "(4 5 6)")

(test-case "(= 1 1)"                                 "t")
(test-case "(= 1 2)"                                 "nil")
(test-case "(= 1 1.0)"                               "t")
(test-case "(= (+ 1 2) 3)"                           "t")
(test-case "(= \"foo\" \"bar\")"                     ERROR_RUNTIME_INVALID_ARGUMENT)
(test-case "(progn (setq a 123) (= a 123))"          "t")
(test-case "(= a a)"                                 "t")
(test-case "(progn (setq b a) (= a b))"              "t")
(test-case "(progn (setq b 'a) (= a b))"             ERROR_RUNTIME_INVALID_ARGUMENT)
(test-case "(= b 123)"                               ERROR_RUNTIME_INVALID_ARGUMENT)
(test-case "(= () ())"                               ERROR_RUNTIME_INVALID_ARGUMENT)
(test-case "(eq () ())"                              "t")
(test-case "(eq () nil)"                             "t")
(test-case "(eq '(a) '(b))"                          "nil")
(test-case "(eq '(1 2 3) '(1 2 3))"                  "nil")
(test-case "(eql '(1 2 3) '(1 2 3))"                 "nil")
(test-case "(equal '(1 2 3) '(1 2 3))"               "t")
(test-case "(equal '(1 (2 3) 4) '(1 (2 3) 4))"       "t")
(test-case "(equal '(1 (2 3) 4) '(1 2 (3 4)))"       "nil")
(test-case "(eq (atom 'foo) (atom 'bar))"            "t")
(test-case "(eq 'foo 'foo)"                          "t")
(test-case "(eq 'foo 'bar)"                          "nil")
(test-case "(= 3)"                                   "t")
(test-case "(= 3 3.0)"                               "t")
(test-case "(= 3 3 3 3)"                             "t")
(test-case "(= 3 3 5 3)"                             "nil")
(test-case "(= 3 6 5 2)"                             "nil")
(test-case "(= 3 2 3)"                               "nil")
(test-case "(= 0.0 -0.0)"                            "t")
(test-case "(= 0 -0.0)"                              "t")
(test-case "(eq 'a 'b)"                              "nil")
(test-case "(eq 'a 'a)"                              "t")
(test-case "(eq 3 3.0)"                              "nil")
(test-case "(eq (cons 'a 'b) (cons 'a 'c))"          "nil")
(test-case "(eq (cons 'a 'b) (cons 'a 'b))"          "nil")
(test-case "(progn (setq x (cons 'a 'b)) (eq x x))"  "t")
(test-case "(progn (setq x '(a . b)) (eq x x))"      "t")
(test-case "(eq \"FOO\" \"foo\")"                    "nil")
(test-case "(eql 'a 'b)"                             "nil")
(test-case "(eql 'a 'a)"                             "t")
(test-case "(eql 3 3)"                               "t")
(test-case "(eql 3 3.0)"                             "nil")
(test-case "(eql 3.0 3.0)"                           "t")
(test-case "(eql (cons 'a 'b) (cons 'a 'c))"         "nil")
(test-case "(eql (cons 'a 'b) (cons 'a 'b))"         "nil")
(test-case "(progn (setq x (cons 'a 'b)) (eql x x))" "t")
(test-case "(progn (setq x '(a . b)) (eql x x))"     "t")
(test-case "(eql \"FOO\" \"foo\")"                   "nil")
(test-case "(equal 'a 'b)"                           "nil")
(test-case "(equal 'a 'a)"                           "t")
(test-case "(equal 3 3)"                             "t")
(test-case "(equal 3 3.0)"                           "nil")
(test-case "(equal 3.0 3.0)"                         "t")
(test-case "(equal (cons 'a 'b) (cons 'a 'c))"       "nil")
(test-case "(equal (cons 'a 'b) (cons 'a 'b))"       "t")
(test-case "(equal \"Foo\" \"Foo\")"                 "t")
(test-case "(equal \"FOO\" \"foo\")"                 "nil")
(test-case "(equal \"This-string\" \"This-string\")" "t")
(test-case "(equal \"This-string\" \"this-string\")" "nil")
(test-case "(equalp 'a 'b)"                          "nil")
(test-case "(equalp 'a 'a)"                          "t")
(test-case "(equalp 3 3)"                            "t")
(test-case "(equalp 3 3.0)"                          "t")
(test-case "(equalp 3.0 3.0)"                        "t")
(test-case "(equalp (cons 'a 'b) (cons 'a 'c))"      "nil")
(test-case "(equalp (cons 'a 'b) (cons 'a 'b))"      "t")
(test-case "(equalp \"Foo\" \"Foo\")"                "t")
(test-case "(equalp \"FOO\" \"foo\")"                "t")

(test-case "(< 3 5)"                                 "t")
(test-case "(< 5 3)"                                 "nil")
(test-case "(< 3 -5)"                                "nil")
(test-case "(< 3 3)"                                 "nil")
(test-case "(< 0.0 -0.0)"                            "nil")
(test-case "(<= 3 5)"                                "t")
(test-case "(<= 3 -5)"                               "nil")
(test-case "(<= 3 3)"                                "t")
(test-case "(> 4 3)"                                 "t")
(test-case "(> 0.0 -0.0)"                            "nil")
(test-case "(>= 4 3)"                                "t")
(test-case "(< 1 2)"                                 "t")
(test-case "(< 2 1)"                                 "nil")
(test-case "(< 1 1)"                                 "nil")
(test-case "(< -2 -1)"                               "t")
(test-case "(< -1 -2)"                               "nil")
(test-case "(< 1.1 2.1)"                             "t")
(test-case "(< 1 2.1)"                               "t")
(test-case "(< -1.1 2)"                              "t")

(test-case "(defun sqr (x) (* x x))"                 "sqr")
(test-case "(sqr 5)"                                 "25")

(test-case "(cons 1 2)"                              "(1 . 2)")
(test-case "(cons 1 '2)"                             "(1 . 2)")
(test-case "(cons 1 '(2))"                           "(1 2)")
(test-case "(cons 1 nil)"                            "(1)")
(test-case "(cons nil 2)"                            "(nil . 2)")
(test-case "(cons nil nil)"                          "(nil)")
(test-case "(cons 1 (cons 2 (cons 3 (cons 4 nil))))" "(1 2 3 4)")
(test-case "(cons 'a 'b)"                            "(a . b)")
(test-case "(cons 'a (cons 'b (cons 'c '())))"       "(a b c)")
(test-case "(cons 'a '(b c d))"                      "(a b c d)")
(test-case "(cons (list 'a) 'b)"                     "((a) . b)")
(test-case "(cons 'a (list 'b))"                     "(a b)")
(test-case "(cons (list 'a) (list 'b))"              "((a) b)")
(test-case "(cons 1 . (2))"                          "(1 . 2)")
(test-case "(list 1 . (2))"                          "(1 2)")
(test-case "(list (cons 1 2))"                       "((1 . 2))")
(test-case "(list (cons 1 2) (cons 3 4))"            "((1 . 2) (3 . 4))")
(test-case "(list 'a 'b . ('c 'd 'e . ()))"          "(a b c d e)")


