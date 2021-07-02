#include "parsing/atom_holder.h"

namespace mp4_manipulator {

AtomHolder::AtomHolder(
    std::vector<std::unique_ptr<AtomOrDescriptorBase>>&& top_level_atoms,
    std::vector<std::unique_ptr<AP4_Atom>>&& top_level_ap4_atoms)
    : top_level_atoms_(std::move(top_level_atoms)),
      top_level_ap4_atoms_(std::move(top_level_ap4_atoms)) {
  MatchAtoms();
}

std::vector<std::unique_ptr<AtomOrDescriptorBase>>&
AtomHolder::GetTopLevelAtoms() {
  return top_level_atoms_;
}

namespace {
// The recursive guts of the `MatchAtoms` function.
void RecursiveMatchAtoms(AtomOrDescriptorBase* atom, AP4_Atom* ap4_atom) {
  if (ap4_atom == nullptr) {
    // Callers should ensure this doesn't happen.
    assert(false);
    return;
  }
  atom->SetAp4Atom(ap4_atom);
  // Ensure type matching invariant holds.
  assert(atom->GetName().toStdString().length() == 4);

  // The assertion here is verbose because AP4 inspectors limit the four cc
  // to ascii (<= 127), while the type ap4 stores is wider. So we have to
  // convert the ap4 internal type to the printable type for matching.
  std::string four_cc_string = atom->GetName().toStdString();
  AP4_Atom::Type ap4_type = AP4_Atom::TypeFromString(four_cc_string.c_str());
  char ap4_type_printable[5];
  AP4_FormatFourCharsPrintable(ap4_type_printable, ap4_atom->GetType());
  assert(strcmp(four_cc_string.c_str(), ap4_type_printable) == 0);
  if (ap4_type != ap4_atom->GetType()) {
    // Try to gracefully hanlde in non-asserting builds.
    // This shouldn't happen. If it has happened we're violating an invariant
    // and should bail.
    return;
  }

  // Match the inspected atoms to their ap4 equivalent.
  AP4_ContainerAtom* ap4_container_atom =
      AP4_DYNAMIC_CAST(AP4_ContainerAtom, ap4_atom);
  if (ap4_container_atom == nullptr) {
    // Don't need to recurse, nothing left to be done.
    return;
  }

  assert(atom->GetChildAtoms().size() ==
         ap4_container_atom->GetChildren().ItemCount());
  for (size_t i = 0; i < atom->GetChildAtoms().size(); ++i) {
    AP4_Atom* ap4_child = nullptr;
    [[maybe_unused]] AP4_Result result = ap4_container_atom->GetChildren().Get(
        static_cast<AP4_Ordinal>(i), ap4_child);
    assert(result == AP4_SUCCESS);
    RecursiveMatchAtoms(atom->GetChildAtoms().at(i).get(), ap4_child);
  }
}
}  // namespace

void AtomHolder::MatchAtoms() {
  assert(top_level_atoms_.size() == top_level_ap4_atoms_.size());
  for (size_t i = 0; i < top_level_atoms_.size(); ++i) {
    RecursiveMatchAtoms(top_level_atoms_.at(i).get(),
                        top_level_ap4_atoms_.at(i).get());
  }
}

}  // namespace mp4_manipulator
