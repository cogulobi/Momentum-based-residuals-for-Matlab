
/**
 * @file get_KukaSnum.h
 * @brief Numerical computation of the Coriolis matrix S for KUKA LWR IV robot
 *
 * @author Giammarco Tonti
 * @author Corrado Guarino Lo Bianco
 * @date 2025-11-10
 */

/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * get_KukaSnum.h
 *
 * Code generation for function 'get_KukaSnum'
 *
 */

#ifndef _KUKA_NUMERICAL_S_FUNC_
#define _KUKA_NUMERICAL_S_FUNC_

/* Include files */
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Function Declarations */
void get_KukaSnum(double in1[7], double in2[7], double S_tmp[49], double S[7], double C[7]);

/* End of code generation (get_KukaSnum.h) */

#endif
