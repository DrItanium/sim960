//
// Created by jwscoggins on 12/1/20.
//

#include "HasOnboardNeoPixel.h"

HasOnboardNeoPixel::HasOnboardNeoPixel(int pinIndex, int flags) : np(1, pinIndex, flags) { }