#ifndef _SPARSECONVERTER_HPP
#define _SPARSECONVERTER_HPP

#include "sparse_format.h"
#include "config.h"
#include <tuple>
#include <unordered_map>
#include <functional>
#ifdef CUDA
#include "cuda.cuh"
#endif

namespace sparsebase {

namespace utils {

typedef std::vector<std::tuple<bool, std::type_index, context::Context*>> ConversionSchemaConditional;

using ConditionalConversionFunction = std::function<format::Format*(format::Format*, context::Context*)>;
using EdgeConditional = std::function<bool(context::Context*, context::Context*)>;
template <typename IDType, typename NNZType, typename ValueType>
class Converter {
private:
  std::unordered_map<
      std::type_index,
      std::unordered_map<std::type_index,
                         std::vector<std::tuple<EdgeConditional, ConditionalConversionFunction>>>>
      conditional_map_;
  std::unordered_map<
      std::type_index,
      std::unordered_map<std::type_index,
                         std::vector<std::tuple<EdgeConditional, ConditionalConversionFunction>>>>
      conditional_move_map_;
  std::unordered_map<
      std::type_index,
      std::unordered_map<std::type_index,
                         std::vector<std::tuple<EdgeConditional, ConditionalConversionFunction>>>> *
  get_conditional_conversion_map(bool is_move_conversion);

public:
  Converter();
  ~Converter();
  void RegisterConditionalConversionFunction(
      std::type_index from_type, 
      std::type_index to_type,
      ConditionalConversionFunction conv_func,
      EdgeConditional edge_condition,
      bool is_move_conversion = false);
  ConditionalConversionFunction
  GetConditionalConversionFunction(std::type_index from_type, context::Context* from_context, 
                        std::type_index to_type, context::Context* to_context, 
                        bool is_move_conversion = false);
  format::Format *ConvertConditional(format::Format *source, std::type_index to_type, context::Context* to_context,
                          bool is_move_conversion = false);
  template <typename FormatType>
  FormatType *ConvertConditional(format::Format *source, context::Context* to_context, bool is_move_conversion = false) {
    auto *res = this->ConvertConditional(source, FormatType::get_format_id_static(), to_context, is_move_conversion);
    return res->template As<FormatType>();
  }
  std::tuple<bool, context::Context*> CanConvertConditional(std::type_index from_type, context::Context* from_context, std::type_index to_type, 
                  std::vector<context::Context*> to_contexts,
                  bool is_move_conversion = false);
  bool CanConvertConditional(std::type_index from_type, context::Context* from_context, std::type_index to_type, context::Context* to_context,
                  bool is_move_conversion = false);
  std::vector<format::Format *>
  ApplyConversionSchemaConditional(ConversionSchemaConditional cs,
                        std::vector<format::Format *> packed_sfs,
                        bool is_move_conversion = false);
};

} // namespace utils

} // namespace sparsebase

#ifdef _HEADER_ONLY
#include "sparse_converter.cc"
#endif

#endif
