
/*

Closure library 1.1
file "closure.h"

Written by Ivan Yankov aka _Winnie (woowoowoow@bk.ru)
Many thanks to Wolfhound

*/



#ifndef CLOSURE_HEADER_
#define CLOSURE_HEADER_

#ifdef _MSC_VER
#  pragma once
#endif

#define TEMPLATE_PARAM_LIST class R
#define PARAM_TYPE_LIST 
#define PARAM_TYPE_LIST_COMMA 
#define PARAM_FORM_ARG_LIST 
#define PARAM_FORM_ARG_LIST_COMMA 
#define PARAM_ARG_LIST 
#define PARAM_ARG_LIST_COMMA 
#define CLOSURE_NUM Closure0
#include "closure_impl.h"
                          
#define TEMPLATE_PARAM_LIST class R, class P0
#define PARAM_TYPE_LIST P0
#define PARAM_TYPE_LIST_COMMA , P0
#define PARAM_FORM_ARG_LIST P0 p0
#define PARAM_FORM_ARG_LIST_COMMA , P0 p0
#define PARAM_ARG_LIST p0
#define PARAM_ARG_LIST_COMMA , p0
#define CLOSURE_NUM Closure1
#include "closure_impl.h"

#define TEMPLATE_PARAM_LIST class R, class P0, class P1
#define PARAM_TYPE_LIST P0, P1
#define PARAM_TYPE_LIST_COMMA , P0, P1
#define PARAM_FORM_ARG_LIST P0 p0, P1 p1 
#define PARAM_FORM_ARG_LIST_COMMA , P0 p0, P1 p1
#define PARAM_ARG_LIST p0, p1
#define PARAM_ARG_LIST_COMMA , p0, p1
#define CLOSURE_NUM Closure2
#include "closure_impl.h"

#define TEMPLATE_PARAM_LIST class R, class P0, class P1, class P2
#define PARAM_TYPE_LIST P0, P1, P2
#define PARAM_TYPE_LIST_COMMA , P0, P1, P2
#define PARAM_FORM_ARG_LIST P0 p0, P1 p1, P2 p2
#define PARAM_FORM_ARG_LIST_COMMA , P0 p0, P1 p1, P2 p2
#define PARAM_ARG_LIST p0, p1, p2
#define PARAM_ARG_LIST_COMMA , p0, p1, p2
#define CLOSURE_NUM Closure3
#include "closure_impl.h"

#define TEMPLATE_PARAM_LIST class R, class P0, class P1, class P2, class P3
#define PARAM_TYPE_LIST P0, P1, P2, P3
#define PARAM_TYPE_LIST_COMMA , P0, P1, P2, P3
#define PARAM_FORM_ARG_LIST P0 p0, P1 p1, P2 p2, P3 p3
#define PARAM_FORM_ARG_LIST_COMMA , P0 p0, P1 p1, P2 p2, P3 p3
#define PARAM_ARG_LIST p0, p1, p2, p3
#define PARAM_ARG_LIST_COMMA , p0, p1, p2, p3
#define CLOSURE_NUM Closure4
#include "closure_impl.h"

#define TEMPLATE_PARAM_LIST class R, class P0, class P1, class P2, class P3, class P4
#define PARAM_TYPE_LIST P0, P1, P2, P3, P4
#define PARAM_TYPE_LIST_COMMA , P0, P1, P2, P3, P4
#define PARAM_FORM_ARG_LIST P0 p0, P1 p1, P2 p2, P3 p3, P4 p4
#define PARAM_FORM_ARG_LIST_COMMA , P0 p0, P1 p1, P2 p2, P3 p3, P4 p4
#define PARAM_ARG_LIST p0, p1, p2, p3, p4
#define PARAM_ARG_LIST_COMMA , p0, p1, p2, p3, p4
#define CLOSURE_NUM Closure5
#include "closure_impl.h"

#define TEMPLATE_PARAM_LIST class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7
#define PARAM_TYPE_LIST P0, P1, P2, P3, P4, P5, P6, P7
#define PARAM_TYPE_LIST_COMMA , P0, P1, P2, P3, P4, P5, P6, P7
#define PARAM_FORM_ARG_LIST P0 p0, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7
#define PARAM_FORM_ARG_LIST_COMMA , P0 p0, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7
#define PARAM_ARG_LIST p0, p1, p2, p3, p4, p5, p6, p7
#define PARAM_ARG_LIST_COMMA , p0, p1, p2, p3, p4, p5, p6, p7
#define CLOSURE_NUM Closure8
#include "closure_impl.h"

#define TEMPLATE_PARAM_LIST class R, class P0, class P1, class P2, class P3, class P4, class P5
#define PARAM_TYPE_LIST P0, P1, P2, P3, P4, P5
#define PARAM_TYPE_LIST_COMMA , P0, P1, P2, P3, P4, P5
#define PARAM_FORM_ARG_LIST P0 p0, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5
#define PARAM_FORM_ARG_LIST_COMMA , P0 p0, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5
#define PARAM_ARG_LIST p0, p1, p2, p3, p4, p5
#define PARAM_ARG_LIST_COMMA , p0, p1, p2, p3, p4, p5
#define CLOSURE_NUM Closure6
#include "closure_impl.h"


#define closure_bind(PTR, MEM_PTR) (winnie::detail::CreateClosure(MEM_PTR).Init<MEM_PTR>(PTR))

#endif


