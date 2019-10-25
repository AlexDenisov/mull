#pragma once

#include <map>
#include <vector>

#include "MutationPoint.h"
#include "Testee.h"
#include "mull/AST/ASTFinder.h"
#include "mull/Mutators/Mutator.h"

namespace llvm {
class Function;
}

namespace mull {
struct Configuration;
class Program;
class Filter;
class Testee;

class MutationsFinder {
public:
  explicit MutationsFinder(std::vector<std::unique_ptr<Mutator>> mutators,
                           const Configuration &config,
                           const FilePathFilter &filePathFilter,
                           const InstructionFilter &instructionFilter);
  std::vector<MutationPoint *>
  getMutationPoints(const Program &program, std::vector<MergedTestee> &testees,
                    Filter &filter);

private:
  std::vector<std::unique_ptr<Mutator>> mutators;
  std::vector<std::unique_ptr<MutationPoint>> ownedPoints;
  const Configuration &config;
  const FilePathFilter &filePathFilter;
  const InstructionFilter &instructionFilter;
};
} // namespace mull
