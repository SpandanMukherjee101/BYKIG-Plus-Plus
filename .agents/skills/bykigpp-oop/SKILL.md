---
name: bykigpp-oop
description: Guides implementation, testing, and maintenance of object-oriented programming in BYKIG++. Use when adding or modifying classes, objects, fields, methods, constructors, this, access modifiers, inheritance, polymorphism, or related runtime behavior.
---

# BYKIG++ Object-Oriented Programming (OOP) Skill

This skill is the permanent reference and operational guide for all object-oriented programming development in BYKIG++. It governs the design, implementation, verification, and maintenance of classes, instances, fields, methods, constructors, `this` binding, access control, inheritance, polymorphism, and runtime safety.

---

## 1. Preserve the Project's Identity

- **Execution Architecture**: BYKIG++ is a lightweight, direct interpreter implemented in C. It executes `.bykig` source code directly using token-driven scanning, source-position tracking (`ftell`/`fseek`), environment stacks (`envStack`), and call frames (`callStack`).
- **No Compiler Pipeline**: Do not introduce a conventional compiler pipeline, lexer/parser rewrite, AST (Abstract Syntax Tree) architecture, bytecode compiler, or bytecode virtual machine.
- **No Wholesale Replacement**: Do not replace the interpreter wholesale to implement OOP.
- **Inspect Before Modifying**: Always inspect the actual source code (`Interpreter.c`, `VarLL.h`, `ArithmaticExprEval.h`, `ExprEvalStack.h`, `Builtins.h`, `Keywords.h`) before proposing or making implementation changes.
- **Extend Existing Foundations**: Extend existing variable storage (`struct callFrame`, `struct Value`, `enum ValueType`), runtime values, function registries (`funcDef`), scopes, call frames, collections, and error handling wherever appropriate.

---

## 2. Restrict the Scope

This skill applies exclusively to OOP and its supporting runtime infrastructure.

### Authorized OOP Roadmap Scope:
- Class declarations and class registries (`classDef`).
- Object instances and identity (`struct objInstance`, heap allocations, object IDs).
- Instance fields (declaration, storage, dot/arrow access, field assignment).
- Instance methods (declaration inside class, method dispatch, argument passing).
- The `this` reference (dynamic receiver binding scoped per method call frame).
- Constructors (explicit constructor method / initialization logic).
- Public/private access control (member encapsulation, access verification).
- Single inheritance (superclass links, field/method inheritance).
- Method overriding and runtime polymorphism (virtual dispatch via class hierarchy).
- Object type checks and assignment compatibility (`instanceof` / `is_instance` / type predicates).
- Integration tests, memory safety audits, and design documentation.

### Out of Scope:
- Do NOT implement backend libraries, REST APIs, HTTP servers, WebSockets, networking layers, database drivers, frontend frameworks, UI systems, or browser integration as part of these OOP stages.

---

## 3. Follow the Actual Repository

- **Authoritative Sources**: Treat the current source code, regression tests (`tests/run_tests.py`), and documentation (`DevGuide.md`, `UserGuide.md`, `FeatureMatrix.md`, `OOP_DESIGN.md`) as authoritative.
- **Design Authority**: Read `OOP_DESIGN.md` whenever it exists to verify architectural decisions, syntax specifications, and stage status.
- **No Blind Syntax Copying**: Do not assume the project supports syntax copied from C++, Java, Python, or any other language. BYKIG++ uses clean whitespace-delimited tokens and semicolon delimiters (e.g., `int x = 5 ;`).
- **Documented Syntax Decisions**: Make syntax decisions based on BYKIG++ conventions and thoroughly document them in `OOP_DESIGN.md` before implementation.
- **No Imagined Features**: Never assume a feature exists merely because it was proposed in a roadmap or discussion. Verify in code.

---

## 4. Preserve Backward Compatibility

- **Non-OOP Integrity**: Existing non-OOP programs, functions, expressions, control flow, collections, and I/O must continue to work without behavioral drift.
- **Zero Unintended Mutations**: Do not alter unrelated operators, variable declarations, primitive types, function registration/execution, scope shadowing, arrays, dynamic lists, generic maps, modules (`use`), or built-ins.
- **Regression Testing**: Run the complete regression test suite (`python tests/run_tests.py`) before and after any modification. Add regression tests whenever existing behavior could be affected.

---

## 5. Runtime and Memory Safety

Every OOP feature must be carefully evaluated and implemented with rigorous safety guarantees:

- **Object Identity & Reference Semantics**: Distinguish object identity from value equality. Variables holding objects must hold reference pointers (`VAL_OBJECT`), ensuring mutations reflect across aliases.
- **Allocation & Deallocation**: Heap-allocate object instances with distinct ownership lifetimes. Track allocations so frames or a runtime instance registry can manage cleanup without premature frees or dangling pointers.
- **Ownership & Lifetimes**: Clean up temporary or local object references without destroying the underlying instance if referenced elsewhere; ensure proper teardown when instances are deleted or orphaned.
- **Field Initialization**: Ensure default field values are properly initialized upon instance creation to prevent undefined reads or null dereferences.
- **Call-Stack & Receiver Restoration**: When invoking instance methods, bind `this` to the receiver object in the method's call frame. Upon return or unwind, restore the caller's receiver.
- **No Global Mutable Receiver**: Never introduce a global mutable `this` pointer that can become corrupted or clobbered during nested or recursive method calls. Store `this` within the active `callFrame` or lexical environment.
- **Scope Cleanup**: Ensure `cleanupFrame()` handles object reference variables safely and does not double-free shared instances.
- **Constructor Failures**: If an error occurs during object instantiation or constructor execution, ensure partially allocated resources are safely reclaimed.
- **Invalid Member Access**: Safely handle access to undeclared fields, nonexistent methods, private members accessed from outside the class, or member access on null/invalid objects with explicit runtime errors rather than segfaults.
- **Inheritance Lookup & Dispatch**: Safely traverse the inheritance chain from derived class to base class, detecting cycles and terminating gracefully when members are missing.
- **Recursion & Nested Calls**: Method calls within method calls must properly isolate their respective environments, receivers, parameters, and return values.

---

## 6. Mandatory Workflow for Every Implementation Stage

For each individual OOP implementation prompt, follow this strict 10-step protocol:

1. **Inspect Before Coding**: Thoroughly inspect the existing implementation files (`Interpreter.c`, `VarLL.h`, `ArithmaticExprEval.h`, etc.) relevant to the requested task.
2. **Identify Dependencies**: Identify stage dependencies and determine the smallest viable, compatible change.
3. **Strictly Scope the Implementation**: Implement ONLY the feature requested by the current prompt. Do not jump ahead to subsequent stages.
4. **Compile Cleanly**: Compile using the project's actual build method:
   ```powershell
   gcc -O2 -Wall -o Interpreter.exe Interpreter.c
   ```
5. **Run Existing Regressions**: Run the full regression test suite with suitable timeouts:
   ```powershell
   python tests/run_tests.py
   ```
6. **Add Focused Tests**: Add targeted `.bykig` tests for the new feature, verifying normal operation and critical edge cases.
7. **Fix Any Regressions**: Ensure any new failure or regression introduced by the change is diagnosed and fixed immediately.
8. **Inspect Warnings and Safety**: Review all compiler warnings (`-Wall`) and verify memory safety (bounds checking, null guards, leak prevention).
9. **Report Honestly & Completely**: Report exact files modified, build output, actual test results (passing, failing, hung), known limitations, and unresolved issues.
10. **Stop**: Do not proceed to the next stage automatically. Wait for the user's explicit instruction and review.

### Cardinal Testing Rule:
Never report a test as passing if it failed, hung, was skipped, or was not run. Distinguish pre-existing test issues (such as `Try.bykig` waiting for interactive input or recursion stress limits) from newly introduced regressions.

---

## 7. Handle Architecture Problems Honestly

- **No Masking Limitations**: If a requested feature cannot be implemented safely with the current design, explain the specific limitation clearly and propose the smallest compatible architectural adjustment.
- **No Covert Re-architecting**: Do not silently replace the interpreter with a compiler-style AST or bytecode pipeline. Do not rewrite unrelated subsystems to conceal a design flaw.

---

## 8. Documentation Rules

- **Source of Truth**: Maintain `OOP_DESIGN.md` at the repository root as the definitive reference for OOP syntax, architectural designs, runtime structures, stage roadmaps, and implementation status.
- **Synchronized Updates**: Update documentation only when the relevant design is established or the corresponding feature is actually implemented and verified by tests.
- **Transparent Status**: Clearly distinguish planned, implemented, tested, and incomplete functionality using explicit status indicators.

