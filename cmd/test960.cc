//
// Created by jwscoggins on 12/5/20.
//

#include "Core.h"
#include "InstructionFormats.h"
#include <iostream>
#include <memory>
#include <tuple>
// custom implementation (this is on purpose)
namespace i960 {
    // allocate memory in 16 megabyte sections, 256 of them!
    // since this is the testing area it means the entire memory space is ram
    union Cell {
        constexpr explicit Cell(Ordinal value = 0) noexcept : ord(value) { }
        Ordinal ord;
        Integer ival;
        ShortInteger si[2];
        ShortOrdinal so[2];
        ByteInteger bi[4];
        ByteOrdinal bo[4];
    };
    static_assert(sizeof(Cell) == sizeof(Ordinal));
    using Section = std::shared_ptr<Cell[]>;
    constexpr size_t SectionSize = (0xFF'FFFF + 1) / sizeof(Cell);
    Section
    makeSection() noexcept {
        return Section(new Cell[SectionSize]());
    }
    std::array<Section, 256> theMemorySpace;

    constexpr bool addressCellUnaligned(Address address) noexcept {
        return (address & 0b11) != 0;
    }
    constexpr bool addressCellAligned(Address address) noexcept {
        return !addressCellUnaligned(address);
    }
    class CellTarget {
    public:
        constexpr explicit CellTarget(Address address) :
        raw(address),
        sectionId((address & 0xFF00'0000) >> 24),
        cellId((address & 0x00FF'FFFF) >> 2),
        cellByteOffset(address & 0b11),
        cellShortOffset((address & 0b10) ? 1 : 0) { }
        constexpr auto getSectionId() const noexcept { return sectionId; }
        constexpr auto getCellId() const noexcept { return cellId; }
        constexpr auto getByteOffset() const noexcept { return cellByteOffset; }
        constexpr auto getCellShortOffset() const noexcept { return cellShortOffset; }
        constexpr auto isWordAligned() const noexcept { return cellByteOffset == 0; }
        constexpr CellTarget nextTarget() const noexcept { return CellTarget{raw + 4}; }
    private:
        Address raw;
        ByteOrdinal sectionId;
        Address cellId;
        ByteOrdinal cellByteOffset;
        ByteOrdinal cellShortOffset;
    };
    void
    Core::busTestFailed() noexcept {
        throw "BUS TEST FAILED!";
    }
    class TestBusInterfaceUnit : public BusInterfaceUnit {
    public:
        using BusInterfaceUnit::BusInterfaceUnit;
        ~TestBusInterfaceUnit() override = default;
        Cell& loadCell(Address address) noexcept {
            CellTarget cell(address);
            return theMemorySpace[cell.getSectionId()][cell.getCellId()];
        }
        ByteOrdinal load(Address address, TreatAsByteOrdinal ordinal) override {
            CellTarget cell(address);
            return loadCell(address).bo[cell.getByteOffset()];
        }
        ByteInteger load(Address address, TreatAsByteInteger integer) override {
            CellTarget cell(address);
            return theMemorySpace[cell.getSectionId()][cell.getCellId()].bi[cell.getByteOffset()];
        }
        ShortOrdinal load(Address address, TreatAsShortOrdinal ordinal) override {
            CellTarget cell(address);
            return theMemorySpace[cell.getSectionId()][cell.getCellId()].so[cell.getCellShortOffset()];
        }
        ShortInteger load(Address address, TreatAsShortInteger integer) override {
            CellTarget cell(address);
            return theMemorySpace[cell.getSectionId()][cell.getCellId()].si[cell.getCellShortOffset()];
        }
        Ordinal load(Address address, TreatAsOrdinal ordinal) override {
            if (address == 0xFFFF0104) {
                std::cout << "Accessing io constant" << std::endl;
                return 1;
            } else {
                return loadCell(address).ord;
            }
        }
        Integer load(Address address, TreatAsInteger integer) override {
            CellTarget cell(address);
            return theMemorySpace[cell.getSectionId()][cell.getCellId()].ival;
        }
        void store(Address address, ByteOrdinal value, TreatAsByteOrdinal ordinal) override {
            CellTarget cell(address);
            theMemorySpace[cell.getSectionId()][cell.getCellId()].bo[cell.getByteOffset()]= value;
        }
        void store(Address address, ByteInteger value, TreatAsByteInteger integer) override {
            CellTarget cell(address);
            theMemorySpace[cell.getSectionId()][cell.getCellId()].bi[cell.getByteOffset()]= value;
        }
        void store(Address address, ShortOrdinal value, TreatAsShortOrdinal ordinal) override {
            CellTarget cell(address);
            theMemorySpace[cell.getSectionId()][cell.getCellId()].so[cell.getCellShortOffset()]= value;
        }
        void store(Address address, ShortInteger value, TreatAsShortInteger integer) override {
            CellTarget cell(address);
            theMemorySpace[cell.getSectionId()][cell.getCellId()].si[cell.getCellShortOffset()]= value;
        }
        void store(Address address, Ordinal value, TreatAsOrdinal ordinal) override {
            CellTarget cell(address);
            theMemorySpace[cell.getSectionId()][cell.getCellId()].ord = value;
            if (address == 0xFFFF0100) {
                if (value != 0) {
                    std::cout << "LED ON" << std::endl;
                } else {
                    std::cout << "LED OFF" << std::endl;
                }
            }
        }
        void store(Address address, Integer value, TreatAsInteger integer) override {
            CellTarget cell(address);
            theMemorySpace[cell.getSectionId()][cell.getCellId()].ival = value;
            if (address == 0xFFFF0100) {
                if (value != 0) {
                    std::cout << "LED ON" << std::endl;
                } else {
                    std::cout << "LED OFF" << std::endl;
                }
            }
        }
    };
    /// @todo handle unaligned load/store and loads/store which span multiple sections

    void
    installOrdinal(Address at, Ordinal value) {
        auto outerId = (at & 0xFF000000) >> 24;
        auto innerId = at & 0x00FF'FFFF;
        theMemorySpace[outerId][innerId].ord = value;
    }
    void
    installProgram(Address startingAddress, const std::vector<Ordinal>& instructions) {
        auto index = startingAddress;
        for (auto i : instructions) {
            installOrdinal(index, i);
            ++index;
        }
    }
    template<typename First>
    void
    installProgramFragments(Address startingAddress, First value) noexcept {
        installOrdinal(startingAddress, value);
    }
    template<typename First, typename ... Args>
    void
    installProgramFragments(Address startingAddress, First value, Args... values) noexcept {
        installOrdinal(startingAddress, value);
        installProgramFragments(startingAddress+1, values...);
    }

    void
    Core::badInstruction(DecodedInstruction inst) {
        std::cerr << "BAD INSTRUCTION @ 0x" << ip.getOrdinal()  << std::endl;
        std::visit([](auto &&value) {
            using K = std::decay_t<decltype(value)>;
            std::cerr << "\tInstruction opcode: 0x";
            if constexpr (std::is_same_v<K, MEMFormatInstruction>) {
                std::cerr << std::hex << value.upperHalf();
            }
            std::cerr << std::hex << value.lowerHalf() << std::endl;
            if (auto name = value.decodeName(); !name.empty()) {
                std::cerr << "\tName: " << name << std::endl;
            }
        }, inst);
        raiseFault();
    }



    void
    test0() {
        std::cout << __PRETTY_FUNCTION__  << std::endl;
        TestBusInterfaceUnit tbiu;
        // make sure that each instruction operates as expected
        i960::Core testCore(tbiu, 0,4);
        testCore.post();
        // double check that registers are clear at this point
        auto l4 = static_cast<i960::RegisterIndex>(4);
        auto l5 = static_cast<i960::RegisterIndex>(5);
        auto l6 = static_cast<i960::RegisterIndex>(6);
        auto& r4 = testCore.getRegister(l4);
        if (r4.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r4!, got " << std::hex << r4.getOrdinal() << " instead!" << std::endl;
        }
        auto& r5 = testCore.getRegister(l5);
        if (r5.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r5!, got " << std::hex << r5.getOrdinal() << " instead!" << std::endl;
        }
        auto& r6 = testCore.getRegister(l6);
        if (r6.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r6!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }
        // okay now here is the test itself
        // run a simple program:
        // lda 0xfded, r4
        // mov 2, r5
        // addo r4, r5, r6
        testCore.cycle(0x8c203000, 0x0000fded); // LDA 0xfded, r4
        std::cout << "lda 0xfded, r4" << std::endl;
        if (r4.getOrdinal() != 0xfded) {
            std::cout << "\tfailed!, got " << std::hex << r4.getOrdinal() << " instead!" << std::endl;
        }
        testCore.cycle(0x5c281e02); // mov 2, r5
        std::cout << "mov 2, r5" << std::endl;
        if (r5.getOrdinal() != 2) {
            std::cout << "\tfailed!, got " << std::hex << r5.getOrdinal() << " instead!" << std::endl;
        }

        testCore.cycle(0x59'31'40'04); // addo r4,r5,r6
        std::cout << "addo r4, r5, r6" << std::endl;
        if (r6.getOrdinal() != (0xfded + 2)) {
            std::cout << "\tfailed!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }
        std::cout << std::endl;
    }
    void
    test1() {
        std::cout << __PRETTY_FUNCTION__  << std::endl;
        // make sure that each instruction operates as expected
        TestBusInterfaceUnit tbiu;
        i960::Core testCore(tbiu, 0,4);
        testCore.post();
        // double check that registers are clear at this point
        auto l4 = static_cast<i960::RegisterIndex>(4);
        auto l5 = static_cast<i960::RegisterIndex>(5);
        auto l6 = static_cast<i960::RegisterIndex>(6);
        auto& r4 = testCore.getRegister(l4);
        if (r4.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r4!, got " << std::hex << r4.getOrdinal() << " instead!" << std::endl;
        }
        auto& r5 = testCore.getRegister(l5);
        if (r5.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r5!, got " << std::hex << r5.getOrdinal() << " instead!" << std::endl;
        }
        auto& r6 = testCore.getRegister(l6);
        if (r6.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r6!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }
        // okay now here is the test itself
        // run a simple program:
        // lda 0xfded, r4
        // mov 2, r5
        // addo r4, r5, r6
        // subi r4, r5, r6
        testCore.cycle(0x8c203000, 0x0000fded); // LDA 0xfded, r4
        std::cout << "lda 0xfded, r4" << std::endl;
        if (r4.getOrdinal() != 0xfded) {
            std::cout << "\tfailed!, got " << std::hex << r4.getOrdinal() << " instead!" << std::endl;
        }
        testCore.cycle(0x5c281e02); // mov 2, r5
        std::cout << "mov 2, r5" << std::endl;
        if (r5.getOrdinal() != 2) {
            std::cout << "\tfailed!, got " << std::hex << r5.getOrdinal() << " instead!" << std::endl;
        }

        testCore.cycle(0x59'31'40'04); // addo r4,r5,r6
        std::cout << "addo r4, r5, r6" << std::endl;
        if (r6.getOrdinal() != (0xfded + 2)) {
            std::cout << "\tfailed!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }

        testCore.cycle(0x59'31'41'84); // subi r4,r5,r6
        std::cout << "subi r4, r5, r6" << std::endl;
        if (r6.getInteger() != (2 - 0xFDED)) {
            std::cout << "\tfailed!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }
        std::cout << std::endl;
    }
    void
    test2() {
        std::cout << __PRETTY_FUNCTION__  << std::endl;
        // make sure that each instruction operates as expected
        TestBusInterfaceUnit tbiu;
        i960::Core testCore(tbiu, 0,4);
        testCore.post();
        // double check that registers are clear at this point
        auto l4 = static_cast<i960::RegisterIndex>(4);
        auto l5 = static_cast<i960::RegisterIndex>(5);
        auto l6 = static_cast<i960::RegisterIndex>(6);
        auto& r4 = testCore.getRegister(l4);
        if (r4.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r4!, got " << std::hex << r4.getOrdinal() << " instead!" << std::endl;
        }
        auto& r5 = testCore.getRegister(l5);
        if (r5.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r5!, got " << std::hex << r5.getOrdinal() << " instead!" << std::endl;
        }
        auto& r6 = testCore.getRegister(l6);
        if (r6.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r6!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }
        // okay now here is the test itself
        // run a simple program:
        // lda 0xfded, r4
        // mov 2, r5
        // addo r4, r5, r6
        // subi r4, r5, r6
        // subi r5, r4, r6
        testCore.cycle(0x8c203000, 0x0000fded); // LDA 0xfded, r4
        std::cout << "lda 0xfded, r4" << std::endl;
        if (r4.getOrdinal() != 0xfded) {
            std::cout << "\tfailed!, got " << std::hex << r4.getOrdinal() << " instead!" << std::endl;
        }
        testCore.cycle(0x5c281e02); // mov 2, r5
        std::cout << "mov 2, r5" << std::endl;
        if (r5.getOrdinal() != 2) {
            std::cout << "\tfailed!, got " << std::hex << r5.getOrdinal() << " instead!" << std::endl;
        }

        testCore.cycle(0x59'31'40'04); // addo r4,r5,r6
        std::cout << "addo r4, r5, r6" << std::endl;
        if (r6.getOrdinal() != (0xfded + 2)) {
            std::cout << "\tfailed!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }

        testCore.cycle(0x59'31'41'84); // subi r4,r5,r6
        std::cout << "subi r4, r5, r6" << std::endl;
        if (r6.getInteger() != (2 - 0xFDED)) {
            std::cout << "\tfailed!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }
        testCore.cycle(0x59'31'01'85); // subi r5,r4,r6
        std::cout << "subi r5, r4, r6" << std::endl;
        if (r6.getInteger() != (0xFDED - 2)) {
            std::cout << "\tfailed!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }
        std::cout << std::endl;
    }
    void
    testB() {
        std::cout << __PRETTY_FUNCTION__  << std::endl;
        // make sure that each instruction operates as expected
        TestBusInterfaceUnit tbiu;
        i960::Core testCore(tbiu, 0,4);
        testCore.post();
        // double check that registers are clear at this point
        if (testCore.getIP().getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r4!, got " << std::hex << testCore.getIP().getOrdinal() << " instead!" << std::endl;
        }
        // okay now here is the test itself
        // run a simple program:
        // b 0x8000 <destination>
        testCore.cycle(0x08'00'80'00); // b
        std::cout << "b 0x8000" << std::endl;
        if (testCore.getIP().getOrdinal() != 0x8000) {
            std::cout << "\tfailed!, got " << std::hex << testCore.getIP().getOrdinal() << " instead!" << std::endl;
        }
        std::cout << std::endl;
    }
    void
    testCall() {
        // test call
        std::cout << __PRETTY_FUNCTION__  << std::endl;
        // make sure that each instruction operates as expected
        TestBusInterfaceUnit tbiu;
        i960::Core testCore(tbiu, 0,4);
        testCore.post();
        // double check that registers are clear at this point
        if (testCore.getIP().getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r4!, got " << std::hex << testCore.getIP().getOrdinal() << " instead!" << std::endl;
        }
        // okay now here is the test itself
        // run a simple program:
        // call 0x9000 <destination>
        testCore.cycle(0x09'00'90'00); // b
        std::cout << "call 0x9000" << std::endl;
        if (testCore.getIP().getOrdinal() != 0x9000) {
            std::cout << "\tfailed!, got " << std::hex << testCore.getIP().getOrdinal() << " instead!" << std::endl;
        }
        std::cout << std::endl;
    }
    void
    testBal() {
        // test bal
        std::cout << __PRETTY_FUNCTION__  << std::endl;
        // make sure that each instruction operates as expected
        TestBusInterfaceUnit tbiu;
        i960::Core testCore(tbiu, 0,4);
        testCore.post();
        // double check that registers are clear at this point
        if (testCore.getIP().getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r4!, got " << std::hex << testCore.getIP().getOrdinal() << " instead!" << std::endl;
        }
        // okay now here is the test itself
        // run a simple program:
        // call 0x9000 <destination>
        testCore.cycle(0x0b'00'a0'00); // bal
        std::cout << "bal 0xa000" << std::endl;
        if (testCore.getIP().getOrdinal() != 0xa000) {
            std::cout << "\tfailed!, got " << std::hex << testCore.getIP().getOrdinal() << " instead!" << std::endl;
        }
        testCore.cycle(0x0b'ff'60'00); // bal
        std::cout << "bal start" << std::endl;
        if (testCore.getIP().getOrdinal() != 0) {
            std::cout << "\tfailed!, got " << std::hex << testCore.getIP().getOrdinal() << " instead!" << std::endl;
        }
        /// @todo check the frame pointers and such at some point in the future
        // offset testing
        testCore.setIP(testCore.getIP().getOrdinal() + 4); // advance offset
        testCore.cycle(0x0b'00'9f'fc); // bal
        std::cout << "bal 0xa000 // offset style" << std::endl;
        if (testCore.getIP().getOrdinal() != 0xa000) {
            std::cout << "\tfailed!, got " << std::hex << testCore.getIP().getOrdinal() << " instead!" << std::endl;
        }
        std::cout << std::endl;
        /// @todo check the frame pointers and such at some point in the future
    }
    void
    testProperCycle() {
        std::cout << __PRETTY_FUNCTION__  << std::endl;
        // make sure that each instruction operates as expected
        TestBusInterfaceUnit tbiu;
        i960::Core testCore(tbiu, 0,4);
        testCore.post();
        // setup instructions
        installProgramFragments(0, 0x8c20'3000,
                       0x0000'fded,
                       0x5c28'1e02,
                       0x5931'4004 // addo r4,r5,r6
        );

        // double check that registers are clear at this point
        auto l4 = static_cast<i960::RegisterIndex>(4);
        auto l5 = static_cast<i960::RegisterIndex>(5);
        auto l6 = static_cast<i960::RegisterIndex>(6);
        auto& r4 = testCore.getRegister(l4);
        if (r4.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r4!, got " << std::hex << r4.getOrdinal() << " instead!" << std::endl;
        }
        auto& r5 = testCore.getRegister(l5);
        if (r5.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r5!, got " << std::hex << r5.getOrdinal() << " instead!" << std::endl;
        }
        auto& r6 = testCore.getRegister(l6);
        if (r6.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r6!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }
        // okay now here is the test itself
        // run a simple program:
        // lda 0xfded, r4
        // mov 2, r5
        // addo r4, r5, r6
        testCore.cycle(); // LDA 0xfded, r4
        std::cout << "lda 0xfded, r4" << std::endl;
        if (r4.getOrdinal() != 0xfded) {
            std::cout << "\tfailed!, got " << std::hex << r4.getOrdinal() << " instead!" << std::endl;
        }
        testCore.cycle(); // mov 2, r5
        std::cout << "mov 2, r5" << std::endl;
        if (r5.getOrdinal() != 2) {
            std::cout << "\tfailed!, got " << std::hex << r5.getOrdinal() << " instead!" << std::endl;
        }
        testCore.cycle();
        std::cout << "addo r4, r5, r6" << std::endl;
        if (r6.getOrdinal() != (0xfded + 2)) {
            std::cout << "\tfailed!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }
        std::cout << std::endl;
    }
    void
    testSimpleProgram() {
        std::cout << __PRETTY_FUNCTION__  << std::endl;
        // make sure that each instruction operates as expected
        TestBusInterfaceUnit tbiu;
        i960::Core testCore(tbiu, 0,4);
        testCore.post();
        // setup instructions
        installProgramFragments(0,
                0x8cf03000, 0x00000010, // lda 10 <Li960R1>, g14
                0x5c88161e,  // mov g14, g1
                0x5cf01e00,  // mov 0, g14
                0x0a000000,  // ret
                0x00000000,  // 0x14
                0x00000000,  // 0x18
                0x00000000,  // 0x1C
//
                0x8ca03000, 0x00ffffff, // lda ffffff, g4, 0x20, 0x24
                0x58a50090, // and g0, g4, g4              0x28,
                0x8c803000, 0xff000000, // lda ff000000, g0 0x2c, 0x30
                0x58840394, // or g4, g0, g0                0x34
                0x86003000, 0x00000000, // callx 0          0x38, 0x3C
                0x0a000000,  // ret                         0x40
                0x00000000, 0x00000000, 0x00000000,
//
                0x59840e10, // shlo 16, g0, g0
                0x8ca03000, 0x00ff0000, // lda ff0000, g4
                0x59840c10, // shro 16, g0, g0
                0x58840394, // or g4, g0, g0
                0x86003000, 0x00000020, // callx 20
                0x0a000000,  // ret
//
                0x8cf03000, 0x00000094, // lda 94, g14
                0x5c88161e, // mov g14, g1
                0x5cf01e00, // mov 0, g14
                0x5ca01e00, // mov 0, g4
                0x3685000c, // cmpoble g0,g4,90
                0x59a05014, // addo g4,1,g4
                0x08fffff8, // b 84
                0x84045000, // bx (g1)
                0x0a000000,  // ret
                0x00000000, 0x00000000,
//
                0x8c800104, // lda 0x104, g0
                0x86003000, 0x00000050, // callx 50
                0x90841000, // ld (g0), g0
                0x84003000, 0x00000070, // bx 70
                0x0a000000,  // ret
                0x00000000,  // .word 0
                //
                0x5c201610, // mov g0, r4
                0x59210e18, // shlo 24, r4, r4
                0x58801988, // setbit 8,0,g0
                0x59205404, // shro r4,1,r4
                0x86003000, 0x00000050, // callx 50
                0x59210901, // subo 1, r4, r4
                0x92241000, // st r4, (g0)
                0x0a000000,  // ret
                0x00000000, 0x00000000, 0x00000000,
                //
                0x5c801e01, // mov 1, g0
                0x86003000, 0x000000c0, // callx c0
                0x86003000, 0x000000a0, // callx a0
                0x5c801e00, // mov 0, g0
                0x86003000, 0x000000c0, // callx c0
                0x86003000, 0x000000a0, // callx a0
                0x08ffffd8, // b f0
                0x00000000, // .word 0
                // start point
                0x84003000, 0x000000f0, // bx f0
                0x0a000000,  // ret
                0x00000000  // .word 0
        );
        // this simple program assumes that we start at 0x120
        testCore.setIP(0x120);
        testCore.setFrameStart(0x0100'0000);
        // double check that registers are clear at this point
        auto l4 = static_cast<i960::RegisterIndex>(4);
        auto l5 = static_cast<i960::RegisterIndex>(5);
        auto l6 = static_cast<i960::RegisterIndex>(6);
        auto& r4 = testCore.getRegister(l4);
        if (r4.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r4!, got " << std::hex << r4.getOrdinal() << " instead!" << std::endl;
        }
        auto& r5 = testCore.getRegister(l5);
        if (r5.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r5!, got " << std::hex << r5.getOrdinal() << " instead!" << std::endl;
        }
        auto& r6 = testCore.getRegister(l6);
        if (r6.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r6!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }
        for (int i = 0; i < 256; ++i) {
            // run 256 cycles
            testCore.cycle();
        }
    }
}


int main() {
    std::cout << "Starting up test960..." << std::endl;
    for (int i = 0; i < i960::theMemorySpace.size(); ++i) {
        std::cout << "\tSetting up Memory Space " << i << std::endl;
        i960::theMemorySpace[i] = i960::makeSection();
    }
    std::cout << "Running tests..." << std::endl;
    i960::test0();
    i960::test1();
    i960::test2();
    i960::testB();
    i960::testCall();
    i960::testBal();
    i960::testProperCycle();
    i960::testSimpleProgram();
    return 0;
}