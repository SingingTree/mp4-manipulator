#ifndef MP4_MANIPULATOR_ATOM_H_
#define MP4_MANIPULATOR_ATOM_H_

#include <QString>
#include <QVector>
#include <memory>
#include <optional>
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
// perspective they're both similar.
// See ISO 14496-1's Object Description Framework section for an outline
// of descriptors. See ISO 14496-12's Object-structured File Organization
// section for an outline of atoms (boxes). ISO 14496-14 also contains details
// of descriptors in the context of the mp4 format.
class AtomOrDescriptorBase {
 public:
  enum class Type {
    kAtom = 0,
    kDescriptor = 1,
  };
  AtomOrDescriptorBase(char const* name, uint32_t header_size, uint64_t size);
  virtual ~AtomOrDescriptorBase();

  [[nodiscard]] virtual AtomOrDescriptorBase::Type GetType() const = 0;

  [[nodiscard]] QString const& GetName() const;

  [[nodiscard]] uint32_t GetHeaderSize() const;

  [[nodiscard]] uint64_t GetSize() const;

  // Returns, if known, the byte offset of the atom or descriptor from the start
  // of the file.
  [[nodiscard]] std::optional<uint64_t> GetPositionInFile() const;
  void SetPositionInFile(uint64_t position_in_file);

  [[nodiscard]] std::vector<Field> const& GetFields() const;
  void AddField(QString&& name, QString&& data);

  [[nodiscard]] AtomOrDescriptorBase* GetParent() const;
  void SetParent(AtomOrDescriptorBase* parent);

  [[nodiscard]] std::vector<std::unique_ptr<AtomOrDescriptorBase>> const&
  GetChildAtoms() const;
  void AddChildAtom(std::unique_ptr<AtomOrDescriptorBase>&& child);

  [[nodiscard]] std::vector<std::unique_ptr<AtomOrDescriptorBase>> const&
  GetChildDescriptors() const;
  void AddChildDescriptor(std::unique_ptr<AtomOrDescriptorBase>&& child);

  // Return the equivalent AP4_Atom, this may be null if the associated
  // AP4_Atom is not set for whatever reason.
  [[nodiscard]] AP4_Atom* GetAp4Atom() const;
  void SetAp4Atom(AP4_Atom* ap4_atom);

 private:
  // Name of the atom or descriptor.
  QString name_;
  // Size of the header.
  uint32_t header_size_;
  // Total size of the atom (including header).
  uint64_t size_;
  // The byte offset of the atom or descriptor from the start of the file.
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

  [[nodiscard]] AtomOrDescriptorBase::Type GetType() const override;
};

class Descriptor : public AtomOrDescriptorBase {
 public:
  Descriptor(char const* name, uint32_t header_size, uint64_t size);

  [[nodiscard]] AtomOrDescriptorBase::Type GetType() const override;
};

}  // namespace mp4_manipulator
#endif  // MP4_MANIPULATOR_ATOM_H_
