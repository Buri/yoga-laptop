/*
 * File:   math.h
 * Author: buri
 *
 * Created on 3. květen 2014, 18:31
 */

#ifndef MATH_H
#define	MATH_H

#ifdef	__cplusplus
extern "C" {
#endif


/**
 *
 */
int limit_interval(int min, int max, int nmr) {
	if (nmr < min) return min;
	if (nmr > max) return max;
	return nmr;
}



#ifdef	__cplusplus
}
#endif

#endif	/* MATH_H */

