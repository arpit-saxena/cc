# TODO list

Stuff to do before submission

- [x] Fix symbol table semantics
- [x] Add function call codegen
- [x] Add declaration codegen
- [x] Add string literal support (Also probably const support - Not done)
- [x] Compile hello world
- [ ] Add branching and control flow statements
  - [x] if
  - [x] while
  - [x] goto
  - [ ] for (maybe)
- [ ] Add local optimisations
  - [ ] Constant folding
    - [x] If constant condition, don't branch
  - [ ] Local dead-code removal
    - [x] If const condition in if_stmt, don't generate new blocks
    - [ ] Remove the nop instruction
- [ ] Invariant checking
  - [ ] all codegen functions result in a non-terminated block in ir_builder
- [x] Add Tests through CMake's testing facility
  - [ ] Add way to add files which are guaranteed to not compile
