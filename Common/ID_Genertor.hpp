//
// Created by ghost-him on 8/11/24.
//

#pragma once

#include <string>
#include <uuid/uuid.h>

inline std::string generate_uuid() {
    uuid_t uuid;
    char uuid_str[37];
    uuid_generate(uuid);
    uuid_unparse_lower(uuid, uuid_str);
    return {uuid_str};
}