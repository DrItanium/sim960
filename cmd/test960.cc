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
        ByteOrdinal load(Address address, TreatAsByteOrdinal ordinal) override {
            CellTarget cell(address);
            return theMemorySpace[cell.getSectionId()][cell.getCellId()].bo[cell.getByteOffset()];
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
            CellTarget cell(address);
            return theMemorySpace[cell.getSectionId()][cell.getCellId()].ord;
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
        }
        void store(Address address, Integer value, TreatAsInteger integer) override {
            CellTarget cell(address);
            theMemorySpace[cell.getSectionId()][cell.getCellId()].ival = value;
        }
    };
    /// @todo handle unaligned load/store and loads/store which span multiple sections
    class TestInternalPeripheralUnit : public InternalPeripheralUnit {
    public:
        using InternalPeripheralUnit::InternalPeripheralUnit;
        ~TestInternalPeripheralUnit() override = default;
        ByteOrdinal load(Address address, TreatAsByteOrdinal ordinal) override {
            CellTarget cell(address);
            return theMemorySpace[cell.getSectionId()][cell.getCellId()].bo[cell.getByteOffset()];
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
            CellTarget cell(address);
            return theMemorySpace[cell.getSectionId()][cell.getCellId()].ord;
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
        }
        void store(Address address, Integer value, TreatAsInteger integer) override {
            CellTarget cell(address);
            theMemorySpace[cell.getSectionId()][cell.getCellId()].ival = value;
        }
    };


    void
    Core::badInstruction(DecodedInstruction inst) {
        std::cerr << "BAD INSTRUCTION!" << std::endl;
        std::visit([](auto &&value) {
            using K = std::decay_t<decltype(value)>;
            std::cerr << "Instruction opcode: 0x";
            if constexpr (std::is_same_v<K, MEMFormatInstruction>) {
                std::cerr << std::hex << value.upperHalf();
            }
            std::cerr << std::hex << value.lowerHalf() << std::endl;
            if (auto name = value.decodeName(); !name.empty()) {
                std::cerr << "Name: " << name << std::endl;
            }
        }, inst);
        raiseFault();
    }



    void
    test0() {
        std::cout << __PRETTY_FUNCTION__  << std::endl;
        TestBusInterfaceUnit tbiu;
        TestInternalPeripheralUnit tipu;
        // make sure that each instruction operates as expected
        i960::Core testCore(tbiu,tipu, 0,4);
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
        TestInternalPeripheralUnit tipu;
        i960::Core testCore(tbiu, tipu, 0,4);
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
        TestInternalPeripheralUnit tipu;
        i960::Core testCore(tbiu, tipu, 0,4);
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
        TestInternalPeripheralUnit tipu;
        i960::Core testCore(tbiu, tipu, 0,4);
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
        TestInternalPeripheralUnit tipu;
        i960::Core testCore(tbiu, tipu, 0,4);
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
        TestInternalPeripheralUnit tipu;
        i960::Core testCore(tbiu, tipu, 0,4);
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
        testCore.nextInstruction();
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
        TestInternalPeripheralUnit tipu;
        i960::Core testCore(tbiu, tipu, 0,4);
        testCore.post();
        // setup instructions
        theMemorySpace[0][0].ord = 0x8c20'3000;
        theMemorySpace[0][1].ord = 0x0000'fded;
        theMemorySpace[0][2].ord = 0x5c28'1e02;
        theMemorySpace[0][3].ord = 0x5931'4004; // addo r4,r5,r6
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
    return 0;
}