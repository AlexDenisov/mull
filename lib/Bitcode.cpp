#include "mull/Bitcode.h"

#include "LLVMCompatibility.h"
#include "mull/Logger.h"
#include "mull/MutationPoint.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/MD5.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Transforms/Utils/Cloning.h>

using namespace mull;
using namespace llvm;
using namespace std;

Bitcode::Bitcode(std::unique_ptr<llvm::Module> module)
    : module(std::move(module)), uniqueIdentifier("") {}

Bitcode::Bitcode(std::unique_ptr<llvm::Module> module,
                 std::unique_ptr<llvm::MemoryBuffer> buffer,
                 const std::string &md5)
    : module(std::move(module)), buffer(std::move(buffer)) {
  uniqueIdentifier =
      llvm::sys::path::stem(this->module->getModuleIdentifier()).str() + "_" +
      md5;
}

std::unique_ptr<Bitcode> Bitcode::clone(LLVMContext &context) {
  assert(buffer.get() && "Cannot clone non-original module");
  auto clone = llvm_compat::parseBitcode(buffer->getMemBufferRef(), context);
  if (!clone) {
    Logger::error() << "Cannot clone module \n";
    return nullptr;
  }

  clone->setModuleIdentifier(module->getModuleIdentifier());

  return make_unique<Bitcode>(std::move(clone), std::unique_ptr<MemoryBuffer>(),
                              "");
}

llvm::Module *Bitcode::getModule() {
  assert(module);
  return module.get();
}

llvm::Module *Bitcode::getModule() const {
  assert(module);
  return module.get();
}

std::string Bitcode::getUniqueIdentifier() { return uniqueIdentifier; }

std::string Bitcode::getUniqueIdentifier() const { return uniqueIdentifier; }

std::vector<std::string> Bitcode::prepareMutations() {
  std::vector<std::string> mutatedFunctionNames;

  for (auto pair : mutationPoints) {
    auto original = pair.first;
    auto anyPoint = pair.second.front();
    mutatedFunctionNames.push_back(anyPoint->getTrampolineName());
    for (auto point : pair.second) {
      ValueToValueMapTy map;
      auto mutatedFunction = CloneFunction(original, map);
      point->setMutatedFunction(mutatedFunction);
    }
    ValueToValueMapTy map;
    auto originalCopy = CloneFunction(original, map);
    originalCopy->setName(anyPoint->getOriginalFunctionName());
    original->deleteBody();

    std::vector<Value *> args;

    for (auto &arg : original->args()) {
      args.push_back(&arg);
    }

    auto trampoline =
        module->getOrInsertGlobal(anyPoint->getTrampolineName(),
                                  original->getFunctionType()->getPointerTo());
    BasicBlock *block =
        BasicBlock::Create(module->getContext(), "indirect_call", original);
    auto loadValue =
        new LoadInst(trampoline, "indirect_function_pointer", block);
    // name has to be empty for void functions:
    // http://lists.llvm.org/pipermail/llvm-dev/2016-March/096242.html
    auto callInst = CallInst::Create(loadValue, args, "", block);
    ReturnInst::Create(module->getContext(), callInst, block);
  }

  return mutatedFunctionNames;
}

void Bitcode::addMutation(MutationPoint *point) {
  std::lock_guard<std::mutex> guard(mutex);
  auto function = point->getOriginalFunction();
  mutationPoints[function].push_back(point);
}

std::string Bitcode::getInstrumentedUniqueIdentifier() const {
  return getUniqueIdentifier() + "_instrumented";
}

std::string Bitcode::getMutatedUniqueIdentifier() const {
  if (mutationPoints.empty()) {
    return getUniqueIdentifier();
  }

  std::vector<std::string> mutationPointsIds;
  for (auto mutationPair : mutationPoints) {
    for (auto point : mutationPair.second) {
      mutationPointsIds.push_back(point->getUniqueIdentifier());
    }
  }

  std::sort(mutationPointsIds.begin(), mutationPointsIds.end());

  MD5 hasher;
  for (auto &id : mutationPointsIds) {
    hasher.update(id);
  }

  MD5::MD5Result hash;
  hasher.final(hash);
  SmallString<32> result;
  MD5::stringifyResult(hash, result);

  return (getUniqueIdentifier() + "_" + result + "_mutated").str();
}
