#ifndef MP4_MANIPULATOR_FILE_PARSER_H_
#define MP4_MANIPULATOR_FILE_PARSER_H_

#include <memory>
#include <optional>
#include <vector>

#include "Ap4.h"
#include "parsing/atom.h"

namespace mp4_manipulator {
namespace utility {
struct ParsedAtomHolder {
  std::vector<std::unique_ptr<AtomOrDescriptorBase>> top_level_inspected_atoms;
  std::vector<std::unique_ptr<AP4_Atom>> top_level_ap4_atoms;
};

std::optional<ParsedAtomHolder> ReadAtoms(char const* file_name);

void DumpAtom(char const* output_file_name, AP4_Atom& atom);

}  // namespace utility
}  // namespace mp4_manipulator

#endif  // MP4_MANIPULATOR_FILE_PARSER_H_
