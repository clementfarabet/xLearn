;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; xFlow language mode
;;
(define-derived-mode xflow-mode m4-mode
  (setq mode-name "xFlow"))

(font-lock-add-keywords 
 'xflow-mode
 (list
  ;; standard keywords :
  (list 
    (regexp-opt '("require" "version" "true" "false" "static" "input" "output") t)
   '(1 font-lock-keyword-face))

  ;; types :
  (list
   (concat "\\(" (regexp-opt '("array" "arrays" "scalar" "boolean" "string"
                               "scalars" "list" "lists") t) "\\)" "")
   '(1 font-lock-type-face t t))

  ;; declare a node :
  (list
   (concat "\\(" (regexp-opt '("math" "copy" "delay"
                               "generate" "linear_combination"
                               ) t) "\\)" "[ \\t]*(")
   '(1 font-lock-function-name-face t t))

  ;; special variables :
  (list 
   (regexp-opt '("--blackbox--") t)
   '(1 font-lock-variable-name-face))

  ;; libraries :
  (list 
   (regexp-opt '("libneural" "libstd" "libmath") t)
   '(1 font-lock-string-face))

  ;; connect nodes
  (list
   "\\(&\\sw+\\||[^|]+|\\)?[ \\t]*:"
   '(1 font-lock-reference-face nil t)
   )

  ;; connect nodes
  (list
   "\\(&\\sw+\\||[^|]+|\\)?[ \\t]*("
   '(1 font-lock-function-name-face t t)
   )

  ;; Specials
  (list 
   "\\(_\\)"
   '(1 font-lock-variable-name-face))))


;; Extension
(setq auto-mode-alist (cons (cons "\\.xfl$" 'xflow-mode) auto-mode-alist))
