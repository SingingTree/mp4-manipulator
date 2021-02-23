#ifndef MP4_MANIPULATOR_ATOM_INSPECTOR_H_
#define MP4_MANIPULATOR_ATOM_INSPECTOR_H_

#include <memory>
#include <vector>

#include "Ap4.h"
#include "atom.h"

namespace mp4_manipulator {

class AtomInspector : public AP4_AtomInspector {
 public:
  AtomInspector();
  // AP4_AtomInspector overrides
  void StartAtom(char const* name, AP4_UI08 version, AP4_UI32 flags,
                 AP4_Size header_size, AP4_UI64 size) override;
  void EndAtom() override;
  void StartDescriptor(const char* name, AP4_Size header_size,
                       AP4_UI64 size) override;
  void EndDescriptor() override;
  void AddField(char const* name, AP4_UI64 value,
                FormatHint hint = HINT_NONE) override;
  void AddFieldF(char const* name, float value,
                 FormatHint hint = HINT_NONE) override;
  void AddField(char const* name, char const* value,
                FormatHint hint = HINT_NONE) override;
  void AddField(char const* name, unsigned char const* bytes,
                AP4_Size byte_count, FormatHint hint = HINT_NONE) override;
  // end AP4_AtomInspector overrides

  // Moves the inspected atoms out of the inspector, returns a vector
  // constructed from moving the top level atoms. Will leave top_level_atoms_
  // empty after the call.
  std::vector<std::unique_ptr<AtomOrDescriptorBase>> TakeAtoms();

 private:
  // Get the atom or descriptor that preceded the current atom or descriptor.
  // This will return nullptr if current_atom_or_descriptor_ is the first
  // atom or descriptor at the current level.
  AtomOrDescriptorBase* GetPreviousSibling();

  // The atom or descriptor currently being inspected. Things like new fields
  // will be added to this.
  AtomOrDescriptorBase* current_atom_or_descriptor_ = nullptr;
  // The insepctor stores parsed atoms in a tree, these are the atoms at the
  // root of the tree.
  std::vector<std::unique_ptr<AtomOrDescriptorBase>> top_level_atoms_;
};

}  // namespace mp4_manipulator

#endif  // MP4_MANIPULATOR_ATOM_INSPECTOR_H_