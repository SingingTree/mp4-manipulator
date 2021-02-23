#ifndef MP4_MANIPULATOR_ATOM_H_
#define MP4_MANIPULATOR_ATOM_H_

#include <optional>

#include <QString>
#include <QVector>
#include <memory>
#include <vector>

#include "Ap4.h"

namespace mp4_manipulator {

struct Field {
  QString name;
  // May wish to break this out later to hold the actual types.
  QString data;
};

// A base class representing the shared aspects of atoms and descriptors.
// ISO specs don't discuss these in terms of being similar, but from a UI
// perspective they're both essentially the same (atoms have flag)
class AtomOrDescriptorBase {
 public:
  enum class Type {
    kAtom = 0,
    kDescriptor = 1,
  };
  AtomOrDescriptorBase(char const* name, uint32_t header_size, uint64_t size);
  virtual ~AtomOrDescriptorBase();

  virtual AtomOrDescriptorBase::Type GetType() const = 0;

  QString const& GetName() const;

  uint32_t GetHeaderSize() const;

  uint64_t GetSize() const;

  std::optional<uint64_t> GetPositionInFile() const;
  void SetPositionInFile(uint64_t position_in_file);

  std::vector<Field> const& GetFields() const;
  void AddField(QString&& name, QString&& data);

  AtomOrDescriptorBase* GetParent() const;
  void SetParent(AtomOrDescriptorBase* parent);

  std::vector<std::unique_ptr<AtomOrDescriptorBase>> const& GetChildAtoms()
      const;
  void AddChildAtom(std::unique_ptr<AtomOrDescriptorBase>&& child);

  std::vector<std::unique_ptr<AtomOrDescriptorBase>> const&
  GetChildDescriptors() const;
  void AddChildDescriptor(std::unique_ptr<AtomOrDescriptorBase>&& child);

  AP4_Atom* GetAp4Atom() const;
  void SetAp4Atom(AP4_Atom* ap4_atom);

 private:
  // Name of the atom or descriptor.
  QString name_;
  // Size of the header.
  uint32_t header_size_;
  // Total size of the atom (including header).
  uint64_t size_;
  // The offset of the atom or descriptor from the start of the file.
  std::optional<uint64_t> position_in_file_{std::nullopt};
  std::vector<Field> fields_{};
  AtomOrDescriptorBase* parent_{nullptr};
  std::vector<std::unique_ptr<AtomOrDescriptorBase>> child_atoms_{};
  std::vector<std::unique_ptr<AtomOrDescriptorBase>> child_descriptors_{};
  // The equivalent ap4 atom.
  // TODO(bryce): this should live on the atom derived class.
  AP4_Atom* ap4_atom_{nullptr};
};

// Represents an atom that is presented in the UI.
class Atom : public AtomOrDescriptorBase {
 public:
  Atom(char const* name, uint32_t header_size, uint64_t size);

  AtomOrDescriptorBase::Type GetType() const override;
};

class Descriptor : public AtomOrDescriptorBase {
 public:
  Descriptor(char const* name, uint32_t header_size, uint64_t size);

  AtomOrDescriptorBase::Type GetType() const override;
};

}  // namespace mp4_manipulator
#endif  // MP4_MANIPULATOR_ATOM_H_