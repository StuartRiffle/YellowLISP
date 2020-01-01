const char* gBootstrapCode = R"LISP_BOOTSTRAP(

;;;; YellowLISP
;;;; Copyright (C) 2020 Stuart Riffle

(defmacro if (test then else) `(cond (,test ,then) (t ,else)))
(defmacro defvar (n v) `(setq ,n ,v))

(defun null   (x)   (eq x nil))
(defun not    (x)   (if (x) nil t))
(defun or     (a b) (cond (a t) (b t)))
(defun and    (a b) (cond (a (cond (b t)))))

(defun >      (a b) (< b a))
(defun <=     (a b) (if (< b a) nil t))
(defun >=     (a b) (if (< a b) nil t))
(defun /=     (a b) (if (= a b) nil t))

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

)LISP_BOOTSTRAP";
