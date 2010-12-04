divert(-1)
######################################################################
# lush.m4: xFlow -> LUSH converter
# this file contains M4 rules to convert xFlow into LUSH
#
# Author: Clement Farabet
######################################################################

# define a for loop
define(`for',`ifelse($#,0,``$0'',`ifelse(eval($2<=$3),1,
    `pushdef(`$1',$2)$4`'popdef(`$1')$0(`$1',incr($2),$3,`$4')')')')

# Top level keywords
define(`require', ``(==> flow require "$1")'')
define(`version', ``(==> flow version $1)'')
define(`array', ``(==> :flow:array new (list $*))'')

# Array indexers
define(_, 1)
define(`SELECT', (==> ``$1'' select 0 ``$2''))

# Basic keywords
define(`SEP', )
define(`FROM', )
define(`TO', )

# Definitions/Instantiations
define(`DSYMB', (defparameter $1 ``$2''))
define(INDSYMB, (defparameter $1 (or $1 ``$2'')))
define(OUTDSYMB, (defparameter $1 (or $1 ``$2'')))
define(`DNODE', (defparameter $1 ``(==> :flow:node def $2)''))
define(`NODE', (==> ``$1'' new (list ``$2'')))

# Scope, for node definition
define(`SCOPE_OPEN', ``(quote ('')
define(`SCOPE_CLOSE', ``))'')

# Init/List
define(`LIST', (list $*))
define(`INIT', (==> ``$1'' fill $2) )

# stdlib
define(`flow', ``:flow:node:flow'')

# mathlib
define(`math', ``:flow:node:math'')
define(`generate', ``:flow:node:generate'')
define(`reduce', ``:flow:node:reduce'')

# neurallib
define(linear_combination, ``:flow:node:linear'')
define(linear_filter_bank, ``:flow:node:filter-bank'')
define(mean_normalization, ``:flow:node:sub-norm'')
define(std_normalization, ``:flow:node:std-norm'')
define(max_pooling, ``:flow:node:max-pooling'')
define(average_pooling, ``:flow:node:average-pooling'')
define(math_nn, ``:flow:node:math-nn'')

# change comment
changecom(`;;')
define(`COMMENT',`;;')

divert
