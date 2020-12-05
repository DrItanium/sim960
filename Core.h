#ifndef I960_CORE_H__
#define I960_CORE_H__
#include <array>
#include <tuple>
#include <variant>
#include <algorithm>

#include "CoreTypes.h"
#include "Register.h"
#include "DependentFalse.h"
#include "ArithmeticControls.h"
namespace i960
{

    class RegFormatInstruction {
    public:
        constexpr RegFormatInstruction(Ordinal value) noexcept : _value(value) { }
        constexpr RegLit getSrc1() const noexcept {
            // no sfr check
            if (m1) {
                return toLiteral(src1);
            } else {
                return toRegisterIndex(src1);
            }
        }
        constexpr RegLit getSrc2() const noexcept {
            // no sfr check
            if (m2) {
                return toLiteral(src2);
            } else {
                return toRegisterIndex(src2);
            }
        }
        constexpr RegisterIndex getDestination() const noexcept {
            return toRegisterIndex(srcDest);
        }
        constexpr RegLit getSrcDest() const noexcept {
            if (!m3) {
                return getDestination();
            } else {
               // can be a literal when src/dest is used as a source
               return toLiteral(srcDest);
            }
        }
        constexpr ShortOrdinal getOpcode() const noexcept {
            return ((static_cast<ShortOrdinal>(opcode) << 4) & 0x0FF0) | (static_cast<ShortOrdinal>(opcode) & 0x000F);
        }
    private:
        union {
            Ordinal _value;
            struct {
                int src1 : 5;
                int s1 : 1;
                int s2 : 1;
                int opcodeExt : 4;
                int m1 : 1;
                int m2 : 1;
                int m3 : 1;
                int src2 : 5;
                int srcDest : 5;
                int opcode : 8;
            };
        };
    };
    class COBRInstruction {
    public:
        constexpr COBRInstruction(Ordinal value) noexcept : _value(value) { }
        constexpr ShortOrdinal getOpcode() const noexcept { return (static_cast<ShortOrdinal>(opcode) << 4) & 0x0FF0; }
        constexpr RegisterIndex getSrc2() const noexcept { return toRegisterIndex(src2); }
        constexpr RegisterIndex getSrc1() const noexcept { return toRegisterIndex(src1); }
        constexpr bool getTBit() const noexcept { return t; }
        constexpr ShortInteger getDisplacement() const noexcept { return displacement; }
    private:
        union {
            Ordinal _value;
            struct {
                int s2 : 1;
                int t : 1;
                int displacement : 11;
                int m1 : 1;
                int src2 : 5;
                int src1 : 5;
                int opcode : 8;
            };
        };
    };
    class CTRLInstruction {
    public:
        constexpr CTRLInstruction(Ordinal value) noexcept : _value(value) { }
        constexpr ShortOrdinal getOpcode() const noexcept { return (static_cast<ShortOrdinal>(opcode) << 4) & 0x0FF0; }
        constexpr bool getTBit() const noexcept { return t; }
        constexpr Integer getDisplacement() const noexcept { return displacement; }
    private:
        union {
            Ordinal _value;
            struct {
                int unused : 1;
                int t : 1;
                int displacement : 22;
                int opcode : 8;
            };
        };
    };
    class MEMFormatInstruction {
    public:
        constexpr explicit MEMFormatInstruction(Ordinal lowerHalf) noexcept : lower(lowerHalf) { }
        constexpr ShortOrdinal getOpcode() const noexcept { return static_cast<ShortOrdinal>(opcode) << 4; }
        template<typename Core>
        Ordinal computeAddress(Core& referenceCore) const noexcept {
            if (isMEMAFormat()) {
                return computeAddress_MEMA(referenceCore);
            } else {
                return computeAddress_MEMB(referenceCore);
            }
        }
        constexpr RegisterIndex getSrcDest() const noexcept { return static_cast<RegisterIndex>(srcDest); }
    private:
        constexpr bool isMEMAFormat() const noexcept { return modeMajor & 1 == 0; }
        constexpr bool isMEMBFormat() const noexcept { return modeMajor & 1 != 0; }
        template<typename Core>
        Ordinal computeAddress_MEMA(Core& referenceCore) const noexcept {
            // the lsb of mema.mode will always be 0 to get to this point
            if (mema.mode == 0b00) {
                // absolute offset
                return static_cast<Ordinal>(mema.offset);
            } else {
                // register indirect with offset
                return static_cast<Ordinal>(mema.offset) + referenceCore.getRegister(toRegisterIndex(abase)).getOrdinal();
            }

        }
        template<typename Core>
        Ordinal computeAddress_MEMB(Core& referenceCore) const noexcept {
            // bit 12 of the instruction has to be 1 to get into this code, eight possible combos
            union {
                Ordinal next;
                Integer optionalDisplacement;
            } disp;
            disp.next = 0;
            switch (memb.mode) {
                case 0b0100: // register indirect
                    return referenceCore.getRegister(toRegisterIndex(memb.abase)).getOrdinal();
                case 0b0101: // ip with displacement
                    disp.next = referenceCore.getWordAtIP(true);
                    return static_cast<Ordinal>(referenceCore.getIP().getInteger() + disp.optionalDisplacement + 8);
                case 0b0110: // reserved
                    return -1;
                case 0b0111: // register indirect with index
                    return referenceCore.getRegister(toRegisterIndex(memb.abase)).getOrdinal() +
                           referenceCore.getRegister(toRegisterIndex(memb.index)).getOrdinal() *
                           computeScale(referenceCore);
                case 0b1100: // absolute displacement
                    disp.next = referenceCore.getWordAtIP(true);
                    return static_cast<Ordinal>(disp.optionalDisplacement);
                case 0b1101: // register indirect with displacement
                    disp.next = referenceCore.getWordAtIP(true);
                    return static_cast<Ordinal>(referenceCore.getRegister(toRegisterIndex(memb.abase)).getInteger() +
                                                disp.optionalDisplacement);
                case 0b1110: // index with displacement
                    disp.next = referenceCore.getWordAtIP(true);
                    return static_cast<Ordinal>(referenceCore.getRegister(toRegisterIndex(memb.index)).getInteger() *
                                                computeScale(referenceCore) +
                                                disp.optionalDisplacement);
                case 0b1111: // register indirect with index and displacement
                    disp.next = referenceCore.getWordAtIP(true);
                    return static_cast<Ordinal>(referenceCore.getRegister(toRegisterIndex(memb.abase)).getInteger() +
                                                referenceCore.getRegister(toRegisterIndex(memb.index)).getInteger() *
                                                computeScale(referenceCore) +
                                                disp.optionalDisplacement);
                default:
                    return -1;
            }
        }
        template<typename Core>
        Ordinal computeScale(Core& referenceCore) const noexcept {

            switch (memb.scale) {
                case 0b000: return 1;
                case 0b001: return 2;
                case 0b010: return 4;
                case 0b011: return 8;
                case 0b100: return 16;
                default:
                    /// @todo raise an invalid opcode fault here using referenceCore
                    return 1;
            }
        }
    private:
        union {
            Ordinal lower;
            struct {
                // generic view
                unsigned int differentiationBlock : 12;
                unsigned int modeMajor : 2;
                unsigned int abase : 5;
                unsigned int srcDest : 5;
                unsigned int opcode : 8;
            };
            struct {
                unsigned int offset : 12;
                unsigned int mode : 2;
                unsigned int abase : 5;
                unsigned int srcDest : 5;
                unsigned int opcode : 8;
            } mema;
            struct {
                unsigned int index : 5;
                unsigned int unused0 : 2;
                unsigned int scale : 3;
                unsigned int mode : 4;
                unsigned int abase : 5;
                unsigned int srcDest : 5;
                unsigned int opcode : 8;
            } memb;
        };
    };
    class Core {
    private:
        static constexpr Ordinal computeSingleBitShiftMask(Ordinal value) noexcept {
            return 1 << (value & 0b11111);
        }

        static constexpr Ordinal rotateOperation(Ordinal src, Ordinal length) noexcept {
            return (src << length) | (src >> ((-length) & 31u));
        }
    private:
        using DecodedInstruction = std::variant<RegFormatInstruction,
                MEMFormatInstruction,
                COBRInstruction,
                CTRLInstruction>;
        static DecodedInstruction decode(Ordinal value) noexcept {
            if (auto opcode = static_cast<ByteOrdinal>(value >> 24); opcode < 0x20) {
                return CTRLInstruction(value);
            } else if (opcode >= 0x20 && opcode < 0x58) {
                return COBRInstruction(value);
            } else if (opcode >= 0x58 && opcode < 0x80) {
                return RegFormatInstruction(value);
            } else {
                return MEMFormatInstruction(value);
            }
        }
    public:
        using RegisterFile = std::array<Register, 16>;
    public:
        explicit Core(unsigned int salign = 1) : _salign(salign) { }
        constexpr Ordinal computeAlignmentBoundaryConstant() const noexcept {
            return (_salign * 16) - 1;
        }
        void cycle();
        Register& getRegister(RegisterIndex index) noexcept;
        const Register& getRegister(RegisterIndex index) const noexcept;
        const Register& getIP() const noexcept { return ip; }
        /**
         * @brief Retrieve the word at the ip address
         * @param advance
         * @return
         */
        Ordinal
        getWordAtIP(bool advance = false) noexcept {
            auto ipLoc = ip.getOrdinal();
            if (advance) {
                ip.setOrdinal(ipLoc + 4);
            }
            return loadOrdinal(ipLoc);
        }
    private: // memory controller interface routines for abstraction purposes, must be implemented in the .ino file
        Ordinal loadOrdinal(Address address) noexcept;
        void storeOrdinal (Address address, Ordinal value) noexcept;
        Integer loadInteger(Address address) noexcept;
        void storeInteger (Address address, Integer value) noexcept;

        ByteOrdinal loadByteOrdinal(Address address) noexcept;
        void storeByteOrdinal (Address address, ByteOrdinal value) noexcept;
        ByteInteger loadByteInteger(Address address) noexcept;
        void storeByteInteger (Address address, ByteInteger value);

        ShortOrdinal loadShortOrdinal(Address address) noexcept;
        void storeShortOrdinal (Address address, ShortOrdinal value) noexcept;
        ShortInteger loadShortInteger(Address address) noexcept;
        void storeShortInteger (Address address, ShortInteger value) noexcept;
    private:
        DecodedInstruction decodeInstruction(Ordinal currentInstruction);
        void executeInstruction(const DecodedInstruction& inst);
        Ordinal fetchInstruction();
    private: // fault related
        void raiseFault();
    private: // execution routines
        void execute(const RegFormatInstruction& inst) noexcept;
        void execute(const MEMFormatInstruction &inst) noexcept;
        void execute(const COBRInstruction &inst) noexcept;
        void execute(const CTRLInstruction &inst) noexcept;
    private: // fault operations
        void faultno();
        void faultg();
        void faultge();
        void faultl();
        void faultle();
        void faulte();
        void faultne();
        void faulto();

    private: // common internal functions
        void saveLocals() noexcept;
        void restoreLocals() noexcept;
        bool
        getCarryFlag() const noexcept {
            return ac.carryFlagSet();
        }
        void
        setCarryFlag(bool value) noexcept {
            ac.setCarryFlag(value);
        }
    private: // data movement operations
        void
        lda(Ordinal mem, RegisterIndex dest) {
            getRegister(dest).setOrdinal(mem);
        }
        void
        ld(Ordinal address, RegisterIndex dest) {
            getRegister(dest).setOrdinal(loadOrdinal(address));
        }
        void
        ldob(Ordinal mem, RegisterIndex dest) {
            getRegister(dest).setByteOrdinal(loadByteOrdinal(mem));
        }

        void
        ldos(Ordinal mem, RegisterIndex dest) {
            getRegister(dest).setShortOrdinal(loadShortOrdinal(mem));
        }

        void
        ldib(Ordinal mem, RegisterIndex dest) {
            getRegister(dest).setByteInteger(loadByteInteger(mem));
        }

        void
        ldis(Ordinal mem, RegisterIndex dest) {
            getRegister(dest).setShortInteger(loadShortInteger(mem));
        }

        void
        st(RegisterIndex src, Ordinal dest) {
            storeOrdinal(dest, getRegister(src).getOrdinal());
        }

        void
        stob(RegisterIndex src, Ordinal dest) {
            storeByteOrdinal(dest, getRegister(src).getByteOrdinal());
        }

        void
        stib(RegisterIndex src, Ordinal dest) {
            storeByteInteger(dest, getRegister(src).getByteInteger());
        }

        void
        stis(RegisterIndex src, Ordinal dest) {
            storeShortInteger(dest, getRegister(src).getShortInteger());
        }

        void
        stos(RegisterIndex src, Ordinal dest) {
            storeShortOrdinal(dest, getRegister(src).getShortOrdinal());
        }

        void
        stl(RegisterIndex src, Ordinal address) {
            if (!divisibleByTwo(src)) {
                /// @todo raise a operation.invalid_operand fault
                raiseFault();
            } else {
                storeOrdinal(address, getRegister(src).getOrdinal());
                storeOrdinal(address + 4, getRegister(nextRegisterIndex(src)).getOrdinal());
                if ((address & 0b111) != 0 && _unalignedFaultEnabled) {
                    /// @todo generate an OPERATION.UNALIGNED fault
                    raiseFault();
                }
            }
        }
        void
        stt(RegisterIndex src, Ordinal address) {
            if (!divisibleByFour(src)) {
                /// @todo raise a operation.invalid_operand fault
                raiseFault();
            } else {
                storeOrdinal(address, getRegister(src).getOrdinal());
                storeOrdinal(address + 4, getRegister(nextRegisterIndex(src)).getOrdinal());
                storeOrdinal(address + 8, getRegister(nextRegisterIndex(nextRegisterIndex(src))).getOrdinal());
                if ((address & 0b1111) != 0 && _unalignedFaultEnabled) {
                    /// @todo generate an OPERATION.UNALIGNED_FAULT
                    raiseFault();
                }
            }
        }

        void
        stq(RegisterIndex src, Ordinal address) {
            if (!divisibleByFour(src)) {
                raiseFault();
                /// @todo raise a operation.invalid_operand fault
            } else {
                storeOrdinal(address, getRegister(src).getOrdinal());
                storeOrdinal(address + 4, getRegister(nextRegisterIndex(src)).getOrdinal());
                storeOrdinal(address + 8, getRegister(nextRegisterIndex(nextRegisterIndex(src))).getOrdinal());
                storeOrdinal(address + 12, getRegister(nextRegisterIndex(nextRegisterIndex(src))).getOrdinal());
                if ((address & 0b1111) != 0 && _unalignedFaultEnabled) {
                    raiseFault();
                    /// @todo generate an OPERATION.UNALIGNED_FAULT
                }
            }
        }

        void
        ldl(Ordinal mem, RegisterIndex dest) {
            if(!divisibleByTwo(dest)) {
                /// @todo raise invalid_operand fault
                // the Hx docs state that dest is modified
                getRegister(dest).setOrdinal(-1);
                raiseFault();
            } else {
                getRegister(dest).setOrdinal(loadOrdinal(mem));
                getRegister(nextRegisterIndex(dest)).setOrdinal(loadOrdinal(mem+4));
                if ((mem & 0b111) != 0 && _unalignedFaultEnabled) {
                    /// @todo generate an OPERATION.UNALIGNED_FAULT
                    raiseFault();
                }
            }
        }
        void
        ldt(Ordinal mem, RegisterIndex dest) {
            if(!divisibleByFour(dest)) {
                /// @todo raise invalid_operand fault
                // the Hx docs state that dest is modified
                getRegister(dest).setOrdinal(-1);
                raiseFault();
            } else {
                getRegister(dest).setOrdinal(loadOrdinal(mem));
                getRegister(nextRegisterIndex(dest)).setOrdinal(loadOrdinal(mem+4));
                getRegister(nextRegisterIndex(nextRegisterIndex(dest))).setOrdinal(loadOrdinal(mem+8));
                if ((mem & 0b1111) != 0 && _unalignedFaultEnabled) {
                    /// @todo generate an OPERATION.UNALIGNED_FAULT
                    raiseFault();
                }
            }
        }
        void
        ldq(Ordinal mem, RegisterIndex dest) {
            if(!divisibleByFour(dest)) {
                /// @todo raise invalid_operand fault
                // the Hx docs state that dest is modified
                getRegister(dest).setOrdinal(-1);
                raiseFault();
            } else {
                getRegister(dest).setOrdinal(loadOrdinal(mem));
                getRegister(nextRegisterIndex(dest)).setOrdinal(loadOrdinal(mem+4));
                getRegister(nextRegisterIndex(nextRegisterIndex(dest))).setOrdinal(loadOrdinal(mem+8));
                getRegister(nextRegisterIndex(nextRegisterIndex(nextRegisterIndex(dest)))).setOrdinal(loadOrdinal(mem+12));
                if ((mem & 0b1111) != 0 && _unalignedFaultEnabled) {
                    /// @todo generate an OPERATION.UNALIGNED_FAULT
                    raiseFault();
                }
            }
        }

        void
        mov(RegLit src, RegisterIndex dest) {
            getRegister(dest).setOrdinal(extractValue(src, TreatAsOrdinal{}));
        }
        void
        movl(RegLit src, RegisterIndex dest) {
            // so this is a bit of a hack but according to the i960Hx manual only the least significant register gets the literal
            if (!divisibleByTwo(dest) || (isRegisterIndex(src) && !divisibleByTwo(std::get<RegisterIndex>(src)))) {
                getRegister(dest).setInteger(-1);
                getRegister(nextRegisterIndex(dest)).setInteger(-1);
                /// @todo generate a fault here!
                raiseFault();
            } else {
                getRegister(dest).setOrdinal(extractValue(src, TreatAsOrdinal{}));
                getRegister(nextRegisterIndex(dest)).setOrdinal(extractValue(nextValue(src), TreatAsOrdinal{}));
            }
        }
        void
        movt(RegLit src, RegisterIndex dest) {
            if (!divisibleByFour(dest) || (isRegisterIndex(src) && !divisibleByFour(std::get<RegisterIndex>(src)))) {
                getRegister(dest).setInteger(-1);
                getRegister(nextRegisterIndex(dest)).setInteger(-1);
                getRegister(nextRegisterIndex(nextRegisterIndex(dest))).setInteger(-1);
                /// @todo generate a fault here!
                raiseFault();
            } else {
                getRegister(dest).setOrdinal(extractValue(src, TreatAsOrdinal{}));
                getRegister(nextRegisterIndex(dest)).setOrdinal(extractValue(nextValue(src), TreatAsOrdinal{}));
                getRegister(nextRegisterIndex(nextRegisterIndex(dest))).setOrdinal(extractValue(nextValue(nextValue(src)), TreatAsOrdinal{}));
            }
        }
        void
        movq(RegLit src, RegisterIndex dest) {
            if (!divisibleByFour(dest) || (isRegisterIndex(src) && !divisibleByFour(std::get<RegisterIndex>(src)))) {
                getRegister(dest).setInteger(-1);
                getRegister(nextRegisterIndex(dest)).setInteger(-1);
                getRegister(nextRegisterIndex(nextRegisterIndex(dest))).setInteger(-1);
                getRegister(nextRegisterIndex(nextRegisterIndex(nextRegisterIndex(dest)))).setInteger(-1);
                /// @todo generate a fault here!
                raiseFault();
            } else {
                getRegister(dest).setOrdinal(extractValue(src, TreatAsOrdinal{}));
                getRegister(nextRegisterIndex(dest)).setOrdinal(extractValue(nextValue(src), TreatAsOrdinal{}));
                getRegister(nextRegisterIndex(nextRegisterIndex(dest))).setOrdinal(extractValue(nextValue(nextValue(src)), TreatAsOrdinal{}));
                getRegister(nextRegisterIndex(nextRegisterIndex(nextRegisterIndex(dest)))).setOrdinal(extractValue(nextValue(nextValue(nextValue(src))), TreatAsOrdinal{}));
            }
        }

    private: // arithmetic
        Ordinal extractValue(RegLit value, TreatAsOrdinal) const noexcept {
            return std::visit([this](auto&& value) -> Ordinal {
                using K = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<K, Literal>) {
                    return static_cast<Ordinal>(toInteger(value));
                } else if constexpr (std::is_same_v<K, RegisterIndex>) {
                    return getRegister(value).getOrdinal();
                } else {
                    static_assert(DependentFalse<K>, "Unimplemented type!");
                    return 0;
                }
            }, value);
        }
        Integer extractValue(RegLit value, TreatAsInteger) const noexcept {
            return std::visit([this](auto&& value) -> Integer{
                using K = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<K, Literal>) {
                    return static_cast<Integer>(toInteger(value));
                } else if constexpr (std::is_same_v<K, RegisterIndex>) {
                    return getRegister(value).getInteger();
                } else {
                    static_assert(DependentFalse<K>, "Unimplemented type!");
                    return 0;
                }
            }, value);
        }
        RegLit nextValue(RegLit value) const noexcept {
            return std::visit([this](auto&& value) -> RegLit {
                using K = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<K, Literal>) {
                    // in this case it should always be zero
                    return toLiteral(0);
                } else if constexpr (std::is_same_v<K, RegisterIndex>) {
                    return nextRegisterIndex(value);
                } else {
                    static_assert(DependentFalse<K>, "Unimplemented type!");
                }
            }, value);
        }
        /// @todo figure out the different code forms
        void
        addc(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = static_cast<LongOrdinal>(extractValue(src1, TreatAsOrdinal{}));
            auto s2 = static_cast<LongOrdinal>(extractValue(src2, TreatAsOrdinal{}));
            auto c = getCarryFlag() ? 1 : 0;
            auto result = s2 + s1 + c;
            auto upperHalf = static_cast<Ordinal>(result >> 32);
            setCarryFlag(upperHalf != 0) ;
            getRegister(dest).setOrdinal(static_cast<Ordinal>(result));
            /// @todo check for integer overflow condition
        }
        void
        addi(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = extractValue(src1, TreatAsInteger{}) ;
            auto s2 = extractValue(src2, TreatAsInteger{}) ;
            getRegister(dest).setInteger(s2 + s1) ;
            /// @todo implement fault detection
        }

        void
        addo(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = extractValue(src1, TreatAsOrdinal{}) ;
            auto s2 = extractValue(src2, TreatAsOrdinal{}) ;
            getRegister(dest).setOrdinal(s2 + s1) ;
            /// @todo implement fault detection
        }
        void
        subi(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = extractValue(src1, TreatAsInteger{}) ;
            auto s2 = extractValue(src2, TreatAsInteger{}) ;
            getRegister(dest).setInteger(s2 - s1) ;
            /// @todo implement fault detection
        }
        void
        subo(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = extractValue(src1, TreatAsOrdinal{}) ;
            auto s2 = extractValue(src2, TreatAsOrdinal{}) ;
            getRegister(dest).setOrdinal(s2 - s1) ;
            /// @todo implement fault detection
        }
        void
        subc(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = static_cast<LongOrdinal>(extractValue(src1, TreatAsOrdinal{}));
            auto s2 = static_cast<LongOrdinal>(extractValue(src2, TreatAsOrdinal{}));
            auto c = getCarryFlag() ? 1 : 0;
            auto result = s2 - s1 + c;
            auto upperHalf = static_cast<Ordinal>(result >> 32);
            setCarryFlag(upperHalf != 0);
            /// @todo do integer overflow subtraction check
            getRegister(dest).setOrdinal(static_cast<Ordinal>(result));
        }
        void
        muli(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = extractValue(src1, TreatAsInteger{}) ;
            auto s2 = extractValue(src2, TreatAsInteger{}) ;
            getRegister(dest).setInteger(s2 * s1) ;
            /// @todo implement fault detection
        }
        void
        mulo(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = extractValue(src1, TreatAsOrdinal{}) ;
            auto s2 = extractValue(src2, TreatAsOrdinal{}) ;
            getRegister(dest).setOrdinal(s2 * s1) ;
            /// @todo implement fault detection
        }
        void
        divi(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = extractValue(src1, TreatAsInteger{}) ;
            auto s2 = extractValue(src2, TreatAsInteger{}) ;
            getRegister(dest).setInteger(s2 / s1) ;
            /// @todo implement fault detection
        }
        void
        divo(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = extractValue(src1, TreatAsOrdinal{}) ;
            auto s2 = extractValue(src2, TreatAsOrdinal{}) ;
            getRegister(dest).setOrdinal(s2 / s1) ;
            /// @todo implement fault detection
        }
        void ediv(RegLit src1, RegLit src2, RegisterIndex dest);
        void emul(RegLit src1, RegLit src2, RegisterIndex dest);
        void
        remi(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s2 = extractValue(src2, TreatAsInteger{});
            auto s1 = extractValue(src1, TreatAsInteger{});
            getRegister(dest).setInteger(((s2 / s1) * s1));
        }
        void
        remo(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s2 = extractValue(src2, TreatAsOrdinal{});
            auto s1 = extractValue(src1, TreatAsOrdinal{});
            getRegister(dest).setOrdinal(((s2 / s1) * s1));
        }
        void
        modi(RegLit src1, RegLit src2, RegisterIndex dest) {
            // taken from the manual
            auto denominator = extractValue(src1, TreatAsInteger{});
            auto numerator = extractValue(src2, TreatAsInteger{});
            if (denominator == 0) {
                // @todo raise Arithmetic Zero Divide fault
                raiseFault();
                return;
            }
            auto theDestValue = numerator - ((numerator / denominator) * denominator);
            auto& dReg = getRegister(dest);
            dReg.setInteger(theDestValue);
            if (((numerator * denominator) < 0) && (theDestValue != 0)) {
                dReg.setInteger(theDestValue + denominator);
            }
        }
        void
        shlo(RegLit len, RegLit src, RegisterIndex dest) {
            auto theLength = extractValue(len, TreatAsOrdinal{});
            auto theSrc = extractValue(src, TreatAsOrdinal{});
            if (theLength < 32) {
                getRegister(dest).setOrdinal(theSrc << theLength);
            } else {
                getRegister(dest).setOrdinal(0);
            }
        }
        void
        shro(RegLit len, RegLit src, RegisterIndex dest) {
            auto theLength = extractValue(len, TreatAsOrdinal{});
            auto theSrc = extractValue(src, TreatAsOrdinal{});
            if (theLength < 32) {
                getRegister(dest).setOrdinal(theSrc >> theLength);
            } else {
                getRegister(dest).setOrdinal(0);
            }
        }
        void
        shli(RegLit len, RegLit src, RegisterIndex dest) {
            auto theLength = extractValue(len, TreatAsInteger{});
            auto theSrc = extractValue(src, TreatAsInteger{});
            getRegister(dest).setInteger(theSrc << theLength);
        }
        /// @todo correctly implement shri and shrdi
        void
        shri(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto src = extractValue(src2, TreatAsInteger{});
            auto len = abs(extractValue(src1, TreatAsInteger{}));
            if (len > 32)  {
                len = 32;
            }
            getRegister(dest).setInteger(src >> len);
        }

        void
        shrdi(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto src = extractValue(src2, TreatAsInteger{});
            auto len = abs(extractValue(src1, TreatAsInteger{}));
            auto result = src >> len;
            if (src < 0 && result < 0) {
                ++result;
            }
            getRegister(dest).setInteger(result);
        }
        void
        rotate(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto len = extractValue(src1, TreatAsOrdinal {});
            auto src = extractValue(src2, TreatAsOrdinal {});
            getRegister(dest).setOrdinal(rotateOperation(src, len));
        }

    private: // logical operations
        void
        logicalAnd(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal(
                    extractValue(src2, TreatAsOrdinal{}) &
                    extractValue(src1, TreatAsOrdinal{}));
        }
        void
        andnot(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal(
                    (extractValue(src2, TreatAsOrdinal{})) &
                    (~extractValue(src1, TreatAsOrdinal{})));
        }
        void
        logicalNand(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal((~extractValue(src2, TreatAsOrdinal{})) | (~extractValue(src1, TreatAsOrdinal{})));
        }

        void
        logicalNor(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal((~extractValue(src2, TreatAsOrdinal{})) & (~extractValue(src1, TreatAsOrdinal{})));
        }

        void
        logicalNot(RegLit src, RegisterIndex dest) {
            getRegister(dest).setOrdinal(~extractValue(src, TreatAsOrdinal{}));
        }
        void
        notand(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal((~extractValue(src2, TreatAsOrdinal{})) & extractValue(src1,TreatAsOrdinal{}));
        }
        void
        notor(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal((~extractValue(src2, TreatAsOrdinal{})) | extractValue(src1,TreatAsOrdinal{}));
        }
        void
        logicalOr(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal(extractValue(src2, TreatAsOrdinal{}) | extractValue(src1,TreatAsOrdinal{}));
        }
        void
        ornot(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal(extractValue(src2, TreatAsOrdinal{}) | (~extractValue(src1,TreatAsOrdinal{})));
        }
        void
        logicalXor(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal(extractValue(src2, TreatAsOrdinal{}) ^ extractValue(src1, TreatAsOrdinal{}));
        }
        void
        logicalXnor(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal(~(extractValue(src2, TreatAsOrdinal{}) ^ extractValue(src1, TreatAsOrdinal{})));
        }
    private: // bit and bit-field operations
        void
        setbit(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto bitpos = extractValue(src1, TreatAsOrdinal{}) ;
            auto src = extractValue(src2, TreatAsOrdinal{});
            getRegister(dest).setOrdinal(src | (1 << (bitpos & 0b11111)));
        }
        void
        clrbit(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto bitpos = extractValue(src1, TreatAsOrdinal{});
            auto src = extractValue(src2, TreatAsOrdinal{});
            auto bitposModified = ~(computeSingleBitShiftMask(bitpos));
            getRegister(dest).setOrdinal(src & bitposModified);
        }
        void
        notbit(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto bitpos = extractValue(src1, TreatAsOrdinal{});
            auto src = extractValue(src2, TreatAsOrdinal{});
            getRegister(dest).setOrdinal(src ^ (1 << (bitpos & 0b11111)));
        }

        void
        alterbit(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto bitpos = extractValue(src1, TreatAsOrdinal{});
            auto src = extractValue(src2, TreatAsOrdinal{});
            if ((ac.getConditionCode() & 0b010) == 0) {
                getRegister(dest).setOrdinal(src & (~(1 << (bitpos & 0b11111))));
            } else {
                getRegister(dest).setOrdinal(src | (1 << (bitpos & 0b11111)));
            }
        }
        void
        chkbit(RegLit src1, RegLit src2) {
            ac.setConditionCode(((extractValue(src2, TreatAsOrdinal{})& computeSingleBitShiftMask(extractValue(src1, TreatAsOrdinal{}))) == 0) ? 0b000 : 0b010);
        }
        static constexpr Ordinal largestOrdinal = 0xFFFF'FFFF;
        void
        spanbit(RegLit src1, RegisterIndex dest) {
            /**
             * Find the most significant clear bit
             */
            auto result = largestOrdinal;
            ac.clearConditionCode();
            if (auto src = extractValue(src1, TreatAsOrdinal{}); src != largestOrdinal) {
                for (Integer i = 31; i >= 0; --i) {
                    if (auto k = (1 << i); (src & k) == 0) {
                        result = i;
                        ac.setConditionCode(0b010);
                        break;
                    }
                }
            }
            getRegister(dest).setOrdinal(result);
        }
        void
        scanbit(RegLit src, RegisterIndex dest) {
            // find the most significant set bit
            auto result = largestOrdinal;
            ac.clearConditionCode();
            // while the psuedo-code in the programmers manual talks about setting
            // the destination to all ones if src is equal to zero, there is no short
            // circuit in the action section for not iterating through the loop when
            // src is zero. A small optimization
            if (auto theSrc = extractValue(src, TreatAsOrdinal{}); theSrc != 0) {
                for (Integer i = 31; i >= 0; --i) {
                    if (auto k = 1 << i; (theSrc & k) != 0) {
                        ac.setConditionCode(0b010);
                        result = i;
                        break;
                    }
                }
            }
            getRegister(dest).setOrdinal(result);
        }
        void
        extract(RegLit src1, RegLit src2, RegisterIndex dest) {
            // taken from the i960Hx manual
            getRegister(dest).setOrdinal((extractValue(dest, TreatAsOrdinal{}) >> std::min(extractValue(src1, TreatAsOrdinal{}), static_cast<Ordinal>(32))) &
                                         (~(0xFFFF'FFFF << extractValue(src2, TreatAsOrdinal{}))));
        }
        void
        modify(RegLit mask, RegLit src, RegisterIndex srcDest) {
            auto& sd = getRegister(srcDest);
            auto theMask = extractValue(mask, TreatAsOrdinal{});
            sd.setOrdinal((extractValue(src, TreatAsOrdinal{}) & theMask)  | (sd.getOrdinal() & (~theMask)));
        }

        void
        scanbyte(RegLit src1, RegLit src2) {
            ac.clearConditionCode();
            if (auto s1 = extractValue(src1, TreatAsOrdinal{}), s2 = extractValue(src2, TreatAsOrdinal{});
                    ((s1 & 0x0000'00FF) == (s2 & 0x0000'00FF)) ||
                    ((s1 & 0x0000'FF00) == (s2 & 0x0000'FF00)) ||
                    ((s1 & 0x00FF'0000) == (s2 & 0x00FF'0000)) ||
                    ((s1 & 0xFF00'0000) == (s2 & 0xFF00'0000))) {
                ac.setConditionCode(0b010);
            } else {
                ac.setConditionCode(0b000);
            }
        }
    private: // compare and increment or decrement
        void
        cmpo(RegLit src1, RegLit src2) {
            auto s1 = extractValue(src1, TreatAsOrdinal{});
            auto s2 = extractValue(src2, TreatAsOrdinal{});
            if (s1 < s2) {
                ac.setConditionCode(0b100);
            } else if (s1 == s2) {
                ac.setConditionCode(0b010);
            } else {
                ac.setConditionCode(0b001);
            }
        }
        void
        cmpi(RegLit src1, RegLit src2) {
            auto s1 = extractValue(src1, TreatAsInteger{});
            auto s2 = extractValue(src2, TreatAsInteger{});
            if (s1 < s2) {
                ac.setConditionCode(0b100);
            } else if (s1 == s2) {
                ac.setConditionCode(0b010);
            } else {
                ac.setConditionCode(0b001);
            }
        }
        void
        concmpo(RegLit src1, RegLit src2) {
            // don't care what the least significant two bits are of the cond code so just mask them out
            if ((ac.getConditionCode() & 0b100) == 0) {
                auto s1 = extractValue(src1, TreatAsOrdinal{});
                auto s2 = extractValue(src2, TreatAsOrdinal{});
                ac.setConditionCode(s1 <= s2 ? 0b010 : 0b000);
            }
        }
        void
        concmpi(RegLit src1, RegLit src2) {
            // don't care what the least significant two bits are of the cond code so just mask them out
            if ((ac.getConditionCode() & 0b100) == 0) {
                auto s1 = extractValue(src1, TreatAsInteger{});
                auto s2 = extractValue(src2, TreatAsInteger{});
                ac.setConditionCode(s1 <= s2 ? 0b010 : 0b000);
            }

        }
        void
        cmpinco(RegLit src1, RegLit src2, RegisterIndex dest) {
            cmpo(src1, src2);
            auto s2 = extractValue(src2, TreatAsOrdinal{});
            getRegister(dest).setOrdinal(s2 + 1);
        }
        void
        cmpinci(RegLit src1, RegLit src2, RegisterIndex dest) {
            cmpi(src1, src2);
            auto s2 = extractValue(src2, TreatAsInteger{});
            getRegister(dest).setInteger(s2 + 1); // manual states that this instruction suppresses overflow
        }
        void
        cmpdeco(RegLit src1, RegLit src2, RegisterIndex dest) {
            cmpo(src1, src2);
            auto s2 = extractValue(src2, TreatAsOrdinal{});
            getRegister(dest).setOrdinal(s2 - 1);
        }
        void
        cmpdeci(RegLit src1, RegLit src2, RegisterIndex dest) {
            cmpi(src1, src2);
            auto s2 = extractValue(src2, TreatAsInteger{});
            getRegister(dest).setInteger(s2 - 1); // manual states that this instruction suppresses overflow
        }
    private: // branching
        /// @todo figure out correct signatures
        void
        b(Displacement22 targ) {
            ip.setInteger(ip.getInteger() + targ.getValue());
        }
        void
        bal(Displacement22 targ) {
            globals[14].setOrdinal(ip.getOrdinal() + 4);
            // make sure that the code is consistent
            b(targ);
        }
        void
        bx(Ordinal targ) {
            ip.setOrdinal(targ);
        }
        void
        balx(Ordinal targ, RegisterIndex dest) {
            getRegister(dest).setOrdinal(ip.getOrdinal());
            ip.setOrdinal(targ);
        }

        /// @todo condense this operation set down to a single function since the mask is embedded in the opcode itself :)
        template<ConditionCodeKind cck>
        void
        conditionalBranch(Displacement22 dest) {
            if (ac.conditionIs<cck>()) {
                ip.setInteger(ip.getInteger() + dest.getValue());
            }
        }
        template<ConditionCodeKind cck>
        void
        conditionalFault() {
            if (ac.conditionIs<cck>()) {
                raiseFault();
            }
        }
        template<ConditionCodeKind cck>
        void
        compareAndBranch(RegLit src1, RegisterIndex src2, ShortInteger targ, TreatAsInteger) {
            cmpi(src1, src2);
            conditionalBranch<cck>(Displacement22{targ});
        }
        template<ConditionCodeKind cck>
        void
        compareAndBranch(RegLit src1, RegisterIndex src2, ShortInteger targ, TreatAsOrdinal) {
            cmpo(src1, src2);
            conditionalBranch<cck>(Displacement22{targ});
        }
    private: // compare and branch
        void bbc(RegLit bitpos, RegisterIndex src, ShortInteger targ);
        void bbs(RegLit bitpos, RegisterIndex src, ShortInteger targ);
    private: // test condition codes
        void testo(RegisterIndex dest);
        void testno(RegisterIndex dest);
        void teste(RegisterIndex dest);
        void testne(RegisterIndex dest);
        void testl(RegisterIndex dest);
        void testle(RegisterIndex dest);
        void testg(RegisterIndex dest);
        void testge(RegisterIndex dest);
    private:
        static constexpr RegisterIndex PFP = static_cast<RegisterIndex>(0b00000);
        static constexpr RegisterIndex SP = static_cast<RegisterIndex>(0b00001);
        static constexpr RegisterIndex RIP = static_cast<RegisterIndex>(0b00010);
        static constexpr RegisterIndex FP = static_cast<RegisterIndex>(0b11111);
        Ordinal
        getStackPointerAddress() const noexcept {
            return getRegister(SP).getOrdinal();
        }
        void
        setRIP(const Register& ip) noexcept {
            getRegister(RIP).setOrdinal(ip.getOrdinal());
        }
        Ordinal
        getFramePointerAddress() const noexcept {
            return getRegister(FP).getOrdinal();
        }
        void
        setPFP(Ordinal value) noexcept {
            getRegister(PFP).setOrdinal(value);
        }
        void
        setFramePointer(Ordinal value) noexcept {
            getRegister(FP).setOrdinal(value);
        }
        void
        setStackPointer(Ordinal value) noexcept {
            getRegister(SP).setOrdinal(value);
        }
        void
        allocateNewLocalRegisterSet() {
            /// @todo implement at some point
        }
    private: // call and return
        void call(Displacement22 targ);
        void callx(Ordinal targ);
        void calls(RegLit targ);
        void ret();
    private: // processor management
        void flushreg();
        void modtc(const RegFormatInstruction& inst);
        void modpc(const RegFormatInstruction& inst);
        void modac(const RegFormatInstruction& inst);
        void syncf();
    private: // marking
        void fmark();
        void mark();
    private: // Numerics Architecture addons
        void dsubc(RegisterIndex src1, RegisterIndex src2, RegisterIndex dest);
        void dmovt(RegisterIndex src1, RegisterIndex dest);
        /**
         * @brief Decimal Add with Carry. Adds bits 0 through 3 of src2 and src1 and bit 1 of the condition code (used here as a carry bit).
         * The result is stored in bits 0 through 3 of dest. If the addition results in a carry, bit 1 of the condition code is set. Bits 4
         * through 31 of src2 are copied to dst unchanged.
         *
         * The instruction is intended to be used iteratively to add binary-coded-decimal (BCD) values in which the least-significant four bits
         * of the operands represent the decimal numbers 0 to 9. The instruction assumes that the least significant 4 bits of both operands
         * are valid BCD numbers. If these bits are not valid BCD numbers, the resulting value in dest is unpredictable.
         * @param src1 The first bcd number
         * @param src2 The second bcd number
         * @param dest The destination register to store the result in
         */
        void daddc(RegisterIndex src1, RegisterIndex src2, RegisterIndex dest);

    private:
        RegisterFile globals, locals;
        Register ip; // always start at address zero
        ArithmeticControls ac;
        bool _unalignedFaultEnabled = false;
        unsigned int _salign = 1;
    };

}
#endif // end I960_CORE_H__
