#ifndef MP4_MANIPULATOR_ATOM_HOLDER_H_
#define MP4_MANIPULATOR_ATOM_HOLDER_H_

#include <memory>
#include <vector>

#include "atom.h"
#include "result.h"

namespace mp4_manipulator {

class AtomHolder {
 public:
  AtomHolder(
      std::vector<std::unique_ptr<AtomOrDescriptorBase>>&& top_level_atoms,
      std::vector<std::unique_ptr<AP4_Atom>>&& top_level_ap4_atoms);
  std::vector<std::unique_ptr<AtomOrDescriptorBase>>& GetTopLevelAtoms();

  // Searches the model for `atom_to_remove` and removes it.Returns a result, on failure
  // this result has a string explaining the error.
  Result<std::monostate, std::string> RemoveAtom(Atom* atom_to_remove);

  // Saves the atoms in the model to a file. Returns a result, on failure
  // this result has a string explaining the error.
  Result<std::monostate, std::string> SaveAtoms(char const* file_name);

 private:
  std::vector<std::unique_ptr<AtomOrDescriptorBase>> top_level_atoms_;
  std::vector<std::unique_ptr<AP4_Atom>> top_level_ap4_atoms_;

  // Walks the parsed AP4 atom tree and mp4_manipulator trees and sets pointers
  // on the mp4_manipulator atoms to their AP4 counterparts.
  void MatchAtoms();

  // Takes the current AP4 atom tree, based on `top_level_ap4_atoms_`, and
  // processes them using an AP4 processor to regenerate the atoms held by the
  // holder. The AP4 processor will try to update sizes, references, etc, as
  // appropriate.
  //
  // This should be done after mutating the atoms, if we wish to keep the mp4
  // internally consistent. This is an expensive operation for memory and
  // processing, as it will regenerate the whole set of atoms.
  //
  // Returns true if processing was successful, false if not.
  bool ProcessAp4Atoms();
};

}  // namespace mp4_manipulator
#endif  // MP4_MANIPULATOR_ATOM_HOLDER_H_