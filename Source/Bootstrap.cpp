const char* gBootstrapCode = R"LISP_BOOTSTRAP(

;;;; YellowLISP
;;;; Copyright (C) 2019 Stuart Riffle

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

(defmacro if (test then else) `(cond (,test ,then) (t ,else)))
(defmacro defvar (n v) `(setq n ,v)))

(defun null   (x)   (eq x nil))
(defun and    (a b) (cond (a (cond (b T)))))
(defun or     (a b) (cond (a T) (b T)))
(defun not    (x)   (cond (x nil) (T T)))

(defun >      (a b) (< b a)))
(defun <=     (a b) (cond ((< b a) nil) (T T)))
(defun >=     (a b) (cond ((< a b) nil) (T T)))
(defun /=     (a b) (cond ((= a b) nil) (T T)))

(defun zero? (x) (=  x 0))
(ASSERT (zero? 0)))
(ASSERT (not (zero? 1)))
(ASSERT (not (zero? -1)))
(ASSERT (not (zero? T)))
(ASSERT (not (zero? NIL)))

(defun plus? (x) (<  0 x))
(ASSERT (plus? 1))
(ASSERT (not (plus? 0)))
(ASSERT (not (plus? -1)))
(ASSERT (not (plus? T)))
(ASSERT (not (plus? NIL)))

(defun minus? (x) (<  x 0))
(ASSERT (minus? -1))
(ASSERT (not (minus? 0)))
(ASSERT (not (minus? 1)))
(ASSERT (not (minus? T)))
(ASSERT (not (minus? NIL)))


)LISP_BOOTSTRAP";
