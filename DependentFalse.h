//
// Created by jwscoggins on 11/27/20.
//

#ifndef I960_PROTOTYPE_SIMULATOR_DEPENDENTFALSE_H
#define I960_PROTOTYPE_SIMULATOR_DEPENDENTFALSE_H
template<typename ... Deps>
inline constexpr bool DependentFalse = false;
#endif //I960_PROTOTYPE_SIMULATOR_DEPENDENTFALSE_H
