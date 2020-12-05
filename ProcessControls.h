//
// Created by jwscoggins on 12/5/20.
//

#ifndef I960_PROTOTYPE_SIMULATOR_PROCESSCONTROLS_H
#define I960_PROTOTYPE_SIMULATOR_PROCESSCONTROLS_H
#include "CoreTypes.h"
#include "Register.h"
namespace i960 {
    class ProcessControls {
    public:
        constexpr ProcessControls() noexcept : _raw(0) { }
        constexpr auto getPriority() const noexcept { return p; }
        constexpr auto getRawValue() const noexcept { return _raw; }
        constexpr auto traceFaultsEnabledGlobally() const noexcept { return static_cast<bool>(te); }
        constexpr auto inSupervisorMode() const noexcept { return em == 1; }
        constexpr auto inUserMode() const noexcept { return em == 0; }
        constexpr auto getExecutionMode() const noexcept { return em; }
        constexpr auto traceFaultPending() const noexcept { return static_cast<bool>(tfp); }
        constexpr auto isInterrupted() const noexcept { return static_cast<bool>(s); }
        constexpr auto isExecuting() const noexcept { return !static_cast<bool>(s); }
        void setPriority(Ordinal priority) noexcept { p = priority; }
        void setTraceEnableBit(bool value) noexcept { te = value; }
        void setExecutionMode(bool value) noexcept { em = value; }
        void setTraceFaultPending(bool value) noexcept { tfp = value; }
        void setStateFlag(bool value) noexcept { s = value; }
        void setRawValue(Ordinal value) noexcept { _raw = value; }
    private:
        union {
            Ordinal _raw;
            struct {
                unsigned te : 1;
                unsigned em : 1;
                unsigned unused0 : 8;
                unsigned tfp : 1;
                unsigned unused1 : 2;
                unsigned s : 1;
                unsigned unused2 : 2;
                unsigned p : 5;
                unsigned unused3 : 11;
            };
        };
        Ordinal _value = 0;
    };
}

#endif //I960_PROTOTYPE_SIMULATOR_PROCESSCONTROLS_H
