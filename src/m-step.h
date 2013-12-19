#ifndef M_Step_H
#define M_Step_H
#include <stdio.h>
#include "lbfgs.h"
#include "scfg.h"

/* compute the value of the object function */
lbfgsfloatval_t evaluate_function_value( void *instance, int n, const lbfgsfloatval_t *x) ;
// compute the gradient of the object function under the current parameters */
void evaluate_function_gradient1 ( void *instance, const lbfgsfloatval_t *x ,lbfgsfloatval_t *g, const int n, const lbfgsfloatval_t step);

//combine the above function
lbfgsfloatval_t evaluate_main(void* instance,const lbfgsfloatval_t* t,lbfgsfloatval_t* g,const int n,const lbfgsfloatval_t step);

lbfgsfloatval_t evaluate(
    void *instance,
    const lbfgsfloatval_t *x,
    lbfgsfloatval_t *g,
    const int n,
    const lbfgsfloatval_t step
    );

int progress(
    void *instance,
    const lbfgsfloatval_t *x,
    const lbfgsfloatval_t *g,
    const lbfgsfloatval_t fx,
    const lbfgsfloatval_t xnorm,
    const lbfgsfloatval_t gnorm,
    const lbfgsfloatval_t step,
    int n,
    int k,
    int ls
    );

struct LBFGS_Param
{
    SCFG* scfg;
    double sum_expect_count;
    vector<double> sum_expect_feature_value;
    double L2_coefficient;
};

void PreComputeLBFGS_Param(LBFGS_Param* lbfgs_param,int n);
#endif
