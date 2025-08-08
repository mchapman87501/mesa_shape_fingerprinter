# ChangeLog

## Disclaimer

This change log is poorly maintained.

## Unreleased

### Added

#### `Result<Value, std::string>`

A Result type has been introduced in `mesaac::mol`. Functions that previously used signatures like `bool some_func(int input_arg, ..., Value& result)`, and that wrote errors directly to stderr, now use `Result<Value, std::string> some_func(int input_arg, ...)`.

The `Result` type represents either a successful result (the `Value`) or an error condition (the `std::string` message). The goal is to help GUI applications capture error messages for display to the user.

In future, `Result` may be changed:

- to convey warning conditions while still providing a successful `Value`
- to represent errors and warnings using enum classes, to ease display of localized error messages

#### Better V3000 SD File Support

`mesaac::mol::SDReader` now does a better job of reading non-query atom properties from both V2000 and V3000 SD files. It captures all fields from V2000 and V3000 atom lines, and it merges information from V2000 `M  CHG` lines and the like.

Atom properties are now stored in `mesaac::mol::AtomProps` instances. The goal is to represent atom properties in a consistent way, one that can be read from either V2000 or V3000 SD files and that can be written to either V3000 or V2000 SD files.

### Changed

### Pubchem Element Info

Functions such as `mesaac::mol::get_atomic_mass` now derive their results from [PubChem's periodic table](https://pubchem.ncbi.nlm.nih.gov/periodic-table/).
