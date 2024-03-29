#ifndef MP4_MANIPULATOR_FILE_PARSER_H_
#define MP4_MANIPULATOR_FILE_PARSER_H_

#include <memory>
#include <optional>
#include <vector>

#include "Ap4.h"
#include "parsing/atom.h"
#include "parsing/atom_holder.h"

namespace mp4_manipulator {
namespace utility {
struct ParsedAtomHolder {
  std::vector<std::unique_ptr<AtomOrDescriptorBase>> top_level_inspected_atoms;
  std::vector<std::unique_ptr<AP4_Atom>> top_level_ap4_atoms;
};

// Reads atoms from a bytestream. Returns a holder which contains vectors of
// the parsed atoms as AtomOrDescriptorBase and AP4_Atoms (these are different
// representations of the same underlying data).
std::optional<std::unique_ptr<AtomHolder>> ReadAtoms(AP4_ByteStream* input);

// Reads atoms from a file. Returns a holder which contains vectors of the
// parsed atoms as AtomOrDescriptorBase and AP4_Atoms (these are different
// representations of the same underlying data).
std::optional<std::unique_ptr<AtomHolder>> ReadAtoms(char const* file_name);

// Dumps an atom to a file.
void DumpAtom(char const* output_file_name, AP4_Atom& atom);

}  // namespace utility
}  // namespace mp4_manipulator

#endif  // MP4_MANIPULATOR_FILE_PARSER_H_
