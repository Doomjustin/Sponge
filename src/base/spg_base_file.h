#ifndef SPONGE_BASE_FILE_H
#define SPONGE_BASE_FILE_H

#include <filesystem>

namespace spg::base {

// 如果path是相对路径，那么会拼接成当前工作目录的绝对路径
auto full_path(std::string_view path) -> std::filesystem::path;

} // namespace spg::base

#endif // SPONGE_BASE_FILE_H
