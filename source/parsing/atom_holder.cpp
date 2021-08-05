#include "parsing/atom_holder.h"

#include "parsing/atom_path_utils.h"
#include "parsing/file_utils.h"

namespace mp4_manipulator {
namespace {
// Base class for commands for the processor.
class Command {
 public:
  virtual Result<std::monostate, std::string> Do(AP4_AtomParent& top_level) = 0;
  Command() = default;
  Command(Command const&) = default;
  Command& operator=(Command const&) = default;
  Command(Command&&) = default;
  Command& operator=(Command&&) = default;
  virtual ~Command() = default;
};

class RemoveCommand : public Command {
 public:
  RemoveCommand(Ap4CompatiblePath path_to_remove)
      : path_to_remove_(std::move(path_to_remove)) {}
  Result<std::monostate, std::string> Do(AP4_AtomParent& top_level) override {
    std::optional<AP4_Atom*> possible_atom =
        GetAp4AtomFromPath(path_to_remove_, top_level);
    if (!possible_atom.has_value()) {
      return Result<std::monostate, std::string>::Err("Atom not found!");
    }
    AP4_Atom* atom = possible_atom.value();
    atom->Detach();

    return Result<std::monostate, std::string>::Ok();
  }

 private:
  Ap4CompatiblePath path_to_remove_;
};

// Based on AP4_EditingProcessor from Mp4Edit.cpp in the Ap4 lib.
class EditingProcessor : public AP4_Processor {
 public:
  EditingProcessor() = default;
  EditingProcessor(EditingProcessor const&) = delete;
  EditingProcessor& operator=(EditingProcessor const&) = delete;
  EditingProcessor(EditingProcessor&& other) noexcept
      : commands_(std::move(other.commands_)) {}
  EditingProcessor& operator=(EditingProcessor&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    this->commands_ = std::move(other.commands_);
    return *this;
  }
  // EditingProcessor(EditingProcessor&& other) = default;
  ~EditingProcessor() override = default;

  AP4_Result Initialize(AP4_AtomParent& top_level, AP4_ByteStream& stream,
                        ProgressListener* listener) override;
  void AddCommand(std::unique_ptr<Command> command);

  std::optional<std::string> GetInitializationError() const;

 private:
  std::vector<std::unique_ptr<Command>> commands_;

  // If `Initialize()` failed, this will contain the error.
  std::optional<std::string> initialization_error_;
};

AP4_Result EditingProcessor::Initialize(AP4_AtomParent& top_level,
                                        AP4_ByteStream& stream,
                                        ProgressListener* listener) {
  AP4_Result ap4_result = AP4_SUCCESS;
  for (std::unique_ptr<Command>& command : commands_) {
    // TODO(bryce): logging on failure.
    Result<std::monostate, std::string> result = command->Do(top_level);
    if (result.IsErr()) {
      result.MarkErrorHandled();
      initialization_error_ = std::move(result).GetErr();
      ap4_result = AP4_FAILURE;
    }
  }
  return ap4_result;
}

void EditingProcessor::AddCommand(std::unique_ptr<Command> command) {
  commands_.push_back(std::move(command));
}

// TODO(bryce): use this to do error reporting to the UI.
std::optional<std::string> EditingProcessor::GetInitializationError() const {
  return initialization_error_;
}
}  // namespace

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

Result<std::monostate, std::string> AtomHolder::RemoveAtom(
    Atom* atom_to_remove) {
  AP4_List<AP4_Atom> top_level;
  for (std::unique_ptr<AP4_Atom>& ap4_atom : top_level_ap4_atoms_) {
    top_level.Add(ap4_atom.get());
  }

  Ap4CompatiblePath path = GetAp4Path(atom_to_remove->GetAp4Atom(), top_level);

  EditingProcessor processor;
  // TODO(bryce): error handle this.
  processor.AddCommand(std::make_unique<RemoveCommand>(path));

  // TODO(bryce): error handle this.
  ProcessAp4Atoms(processor);

  return Result<std::monostate, std::string>::Ok();
}

Result<std::monostate, std::string> AtomHolder::SaveAtoms(
    char const* file_name) {
  // Create AP4 byte stream from top level atoms.
  AP4_AtomParent dummy_root;

  AP4_ByteStream* output_stream = NULL;
  AP4_Result result = AP4_FileByteStream::Create(
      file_name, AP4_FileByteStream::STREAM_MODE_WRITE, output_stream);

  if (AP4_FAILED(result)) {
    // TODO(bryce): better error message (could us fmt).
    return Result<std::monostate, std::string>::Err(
        "AP4_FileByteStream::Create failed during SaveAtoms");
  }

  for (std::unique_ptr<AP4_Atom>& ap4_atom : top_level_ap4_atoms_) {
    dummy_root.AddChild(ap4_atom.get());
  }

  dummy_root.GetChildren().Apply(AP4_AtomListWriter(*output_stream));

  result = dummy_root.GetChildren().Clear();
  assert(AP4_SUCCEEDED(result));
  assert(dummy_root.GetChildren().ItemCount() == 0);

  output_stream->Release();

  return Result<std::monostate, std::string>::Ok();
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

bool AtomHolder::ProcessAp4Atoms(AP4_Processor& processor) {
  // TODO(bryce): better estimate the size of this.
  AP4_MemoryByteStream* current_atom_input_stream =
      new AP4_MemoryByteStream{AP4_Size{}};
  AP4_MemoryByteStream* current_atom_output_stream =
      new AP4_MemoryByteStream{AP4_Size{}};

  // Create AP4 byte stream from top level atoms.
  AP4_AtomParent dummy_root;

  for (std::unique_ptr<AP4_Atom>& ap4_atom : top_level_ap4_atoms_) {
    dummy_root.AddChild(ap4_atom.get());
  }
  dummy_root.GetChildren().Apply(
      AP4_AtomListWriter(*current_atom_input_stream));

  // Seek the input stream to the start so it's ready to be read.
  current_atom_input_stream->Seek(0);

  // Write atoms to an AP4_MemoryByteStream with the processor (AP4_AtomParent).
  processor.Process(*current_atom_input_stream, *current_atom_output_stream);

  // Seek the output stream to the start so it's ready to be parsed.
  current_atom_output_stream->Seek(0);

  // Parse the atoms with mp4 manipulator.
  std::optional<std::unique_ptr<AtomHolder>> possible_new_holder =
      utility::ReadAtoms(current_atom_output_stream);
  if (!possible_new_holder.has_value()) {
    return false;
  }

  std::unique_ptr<AtomHolder> new_atom_holder{
      std::move(possible_new_holder.value())};

  // Remove the atoms from our dummy root, otherwise it will delete them when
  // it goes out of scope and we'll double free. We'll let top_level_ap4_atoms_
  // take care of deleting them.
  [[maybe_unused]] AP4_Result result = dummy_root.GetChildren().Clear();
  assert(AP4_SUCCEEDED(result));
  assert(dummy_root.GetChildren().ItemCount() == 0);

  // Move the atoms out of the new holder into this holder.
  this->top_level_atoms_ = std::move(new_atom_holder->top_level_atoms_);
  this->top_level_ap4_atoms_ = std::move(new_atom_holder->top_level_ap4_atoms_);

  current_atom_input_stream->Release();
  current_atom_output_stream->Release();

  return true;
}

}  // namespace mp4_manipulator
