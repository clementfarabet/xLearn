divert(-1)
######################################################################
# lush.m4: xFlow -> Lua converter
# this file contains M4 rules to convert xFlow into Lua
#
# Author: Clement Farabet
######################################################################

# define a for loop
define(`for',`ifelse($#,0,``$0'',`ifelse(eval($2<=$3),1,
    `pushdef(`$1',$2)$4`'popdef(`$1')$0(`$1',incr($2),$3,`$4')')')')

# Top level keywords
define(require, ``luaFlow.require("$1")'')
define(version, ``luaFlow.version''($1))
define(array, ``luaFlow.Array''($*))

# Array indexers
define(_, 1)
define(SELECT, $1[`eval($2+1)'])

# Basic keywords
define(SEP, COMMA)
define(FROM, =)
define(TO, =)

# Definitions/Instantiations
define(DSYMB, $1 = $2)
define(INDSYMB, table.insert(input_vars,"$1"); $1 = $1 or $2)
define(OUTDSYMB, table.insert(output_vars,"$1"); $1 = $1 or $2)
define(DNODE, $1 = ``luaFlow.Node.def''($2))
define(NODE, $1{$2})

# Scope, for node definition
define(SCOPE_OPEN, [[)
define(SCOPE_CLOSE, ]])

# Init/List
define(LIST, {$*})
define(INIT, $1:fill($2) )

# stdlib
define(flow, luaFlow.``Node.Flow'')

# mathlib
define(math, luaFlow.``Node.Math'')
define(generate, luaFlow.``Node.Generate'')
define(reduce, luaFlow.``Node.Reduce'')

# neurallib
define(linear_combination, luaFlow.``Node.LinearCombination'')
define(linear_filter_bank, luaFlow.``Node.LinearFilterBank'')
define(mean_normalization, luaFlow.``Node.MeanNormalization'')
define(std_normalization, luaFlow.``Node.StdNormalization'')
define(max_pooling, luaFlow.``Node.MaxPooling'')
define(average_pooling, luaFlow.``Node.AveragePooling'')
define(math_nn, luaFlow.``Node.Math'')

# change comment
changecom(`--')
define(`COMMENT',`--')

divert 
COMMENT create tables to hold names of I/Os
input_vars = {}
output_vars = {}
