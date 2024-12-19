# Copyright 2024 Aurora Operations, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.29)

project(test_au_client LANGUAGES CXX)

find_package(Au REQUIRED CONFIG)

add_executable(${PROJECT_NAME} test_au_client.cc)
target_link_libraries(${PROJECT_NAME} PRIVATE Au::au)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)
