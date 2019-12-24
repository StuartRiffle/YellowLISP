const char* gBootstrapCode = R"LISP_BOOTSTRAP(

;;;; YellowLISP
;;;; Copyright (C) 2019 Stuart Riffle

(defun >      (a b) (< b a))
(defun <=     (a b) (or (< a b) (= a b)))
(defun >=     (a b) (or (> a b) (= a b)))
(defun /=     (a b) (not (= a b)))

(defun zerop  (x) (=  x 0))
(defun plusp  (x) (<  0 x))
(defun minusp (x) (<  x 0))
(defun null   (x) (eq x nil))


;;; Really?

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

(defconstant +PI+ 3.141592653589793)

; greaterp - It takes one or more argument and returns t if either there is a single argument or the arguments are successively larger from left to right, or nil if otherwise.
; lessp

; The following code is from "The Roots of Lisp" by Paul Graham (2002)

(defun null (x)
    (eq x '()))

(defun and (x y)
    (cond
        (x (cond (y 't) ('t '())))
        ('t '())))

(defun not (x)
    (cond 
        (x '()) 
        ('t 't)))

(defun append (x y)
    (cond 
        ((null x) y)
        ('t (cons (car x) (append (cdr x) y)))))

(defun pair (x y)
    (cond
        ((and (null x) (null y)) 
            '())
        ((and (not (atom x)) (not (atom y)))
            (cons (list (car x) (car y)) (pair (cdr x) (cdr y))))))

(defun assoc (x y)
    (cond
        ((eq (caar y) x) (cadar y))
        ('t (assoc x (cdr y)))))

(defun eval (e a)
    (cond
        ((atom e) (assoc e a))
        ((atom (car e))
            (cond
                ((eq (car e) 'quote)    (cadr e))
                ((eq (car e) 'atom)     (atom (eval (cadr e) a)))
                ((eq (car e) 'eq)       (eq (eval (cadr e) a) (eval (caddr e) a)))
                ((eq (car e) 'car)      (car (eval (cadr e) a)))
                ((eq (car e) 'cdr)      (cdr (eval (cadr e) a)))
                ((eq (car e) 'cons)     (cons (eval (cadr e) a) (eval (caddr e) a)))
                ((eq (car e) 'cond)     (evcon (cdr e) a))
                ('t                     (eval (cons (assoc (car e) a) (cdr e)) a))))
        ((eq (caar e) 'label)
            (eval (cons (caddar e) (cdr e))
                (cons (list (cadar e) (car e)) a)))
        ((eq (caar e) 'lambda)
            (eval (caddar e)
                (append (pair (cadar e) (evlis (cdr e) a)) a)))))

(defun evcon (c a)
    (cond ((eval (caar c) a)
        (eval (cadar c) a))
        ('t (evcon (cdr c) a))))

(defun evlis (m a)
    (cond ((null m) '())
        ('t (cons (eval (car m) a)
            (evlis (cdr m) a)))))


)LISP_BOOTSTRAP";
