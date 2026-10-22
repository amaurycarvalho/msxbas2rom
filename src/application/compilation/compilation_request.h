/***
 * @file compilation_request.h
 * @brief Application compilation request ports
 * @author Amaury Carvalho (2026)
 */

#ifndef COMPILATION_REQUEST_H_INCLUDED
#define COMPILATION_REQUEST_H_INCLUDED

#include <memory>

class BuildOptions;
class SourceResolver;
class KernelImage;
class ResourceContentSource;
class SymbolOutputSink;
class RomWriter;

class CompilationRequest {
 public:
  std::shared_ptr<BuildOptions> options;
  std::shared_ptr<SourceResolver> sourceResolver;
  std::shared_ptr<KernelImage> kernelImage;
  std::shared_ptr<ResourceContentSource> resourceContentSource;
  std::shared_ptr<SymbolOutputSink> symbolOutputSink;
  std::shared_ptr<RomWriter> romWriter;

  CompilationRequest(std::shared_ptr<BuildOptions> options,
                     std::shared_ptr<SourceResolver> sourceResolver,
                     std::shared_ptr<KernelImage> kernelImage,
                     std::shared_ptr<ResourceContentSource> resourceContentSource,
                     std::shared_ptr<SymbolOutputSink> symbolOutputSink,
                     std::shared_ptr<RomWriter> romWriter)
      : options(options),
        sourceResolver(sourceResolver),
        kernelImage(kernelImage),
        resourceContentSource(resourceContentSource),
        symbolOutputSink(symbolOutputSink),
        romWriter(romWriter) {}
};

#endif  // COMPILATION_REQUEST_H_INCLUDED
