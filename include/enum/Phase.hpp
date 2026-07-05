#pragma once

#include <string>

enum class PhaseEnum { HANSHAKING, READING, WRITING, CLOSING };

std::string Phase(PhaseEnum type);
