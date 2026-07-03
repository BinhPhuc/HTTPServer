#pragma once

#include <string>

enum class PhaseEnum { HANSHAKING, READING, WRITING };

std::string Phase(PhaseEnum type);
