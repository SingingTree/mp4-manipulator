#include <optional>
#include <vector>

#include "Ap4.h"

// Represents an item in an Ap4CompatiblePath. A path is made up of a series
// of these items. The index is the index within that type of atom, not all
// atoms in the parent. E.g. given the following siblings [trak, mvhd, trak],
// then their path items would be (trak, 0), (mvhd, 0), (trak 1).
struct Ap4CompatiblePathItem {
  AP4_Atom::Type type;
  AP4_Ordinal index;
};

// Represents a path to an atom. An example, using the notation (type, index),
// is (moov, 0), (trak, 1), (tkhd, 0), which would means, the first moov atom,
// then the second trak atom in that moov, then the first tkhd in that trak.
struct Ap4CompatiblePath {
  std::vector<Ap4CompatiblePathItem> path;
};

// Returns an Ap4CompatiblePath from the top most parent to the atom. The
// returned path should have the top most parent at index 0, and the atom
// itself at the last index. The top level arg is needed to determine the index
// of the top level atom amongst its siblings and should be a list of top
// level atoms, or a dummy parent that contains the top level atoms as its
// children.
Ap4CompatiblePath GetAp4Path(AP4_Atom* atom, AP4_List<AP4_Atom>& top_level);
Ap4CompatiblePath GetAp4Path(AP4_Atom* atom, AP4_AtomParent& top_level);

// Follows a path and returns that atom at the end of that path.
std::optional<AP4_Atom*> GetAp4AtomFromPath(Ap4CompatiblePath& path,
                                            AP4_List<AP4_Atom>& top_level);
std::optional<AP4_Atom*> GetAp4AtomFromPath(Ap4CompatiblePath& path,
                                            AP4_AtomParent& top_level);