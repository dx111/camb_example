#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "automem.hpp"
#include <cnrt.h>
#include <cstring>
#include <memory>

TEST_CASE("Automem Test") {
    GIVEN("Constructor with size"){
        Automem automem(10);
        WHEN("test not nullptr"){
            REQUIRE(automem.host_data() != nullptr);
            REQUIRE(automem.device_data() != nullptr);
        }
    }

    GIVEN("Constructor with cpu data "){
        int data[5] = {1, 2, 3, 4, 5};
        Automem automem(sizeof(data), data);
        REQUIRE(automem.host_data() != nullptr);
        REQUIRE(automem.device_data() != nullptr);
        const int* hostData = static_cast<const int*>(automem.host_data());
        for (int i = 0; i < 5; ++i) {
            CHECK(hostData[i] == data[i]);
        }
    }
    
    GIVEN("Constructor with mlu data "){
        float data[5] = {1.25f, 2.6f, 3.4f, 4.6f, 5.56f};
        void* mlu_data;
        CNRT_CHECK(cnrtMalloc(&mlu_data, sizeof(data)));
        CNRT_CHECK(cnrtMemcpy(mlu_data, &data, sizeof(data), cnrtMemcpyHostToDev));
        Automem automem(mlu_data, sizeof(data));
        CNRT_CHECK(cnrtFree(mlu_data));
        WHEN("auto copy dev to host"){
            REQUIRE(automem.host_data() != nullptr);
            REQUIRE(automem.device_data() != nullptr);
            const float* hostData = static_cast<const float*>(automem.host_data());
            for (int i = 0; i < 5; ++i) {
                CHECK(hostData[i] == data[i]);
            }
        }

    }

    GIVEN("Copy Constructor"){
        int originalData[5] = {1, 2, 3, 4, 5};
        Automem automem1(sizeof(originalData), originalData);
        Automem automem2 = automem1;
        const int* hostData = static_cast<const int*>(automem2.host_data());
        for (int i = 0; i < 5; ++i) {
            CHECK(hostData[i] == originalData[i]);
        }
    }

    GIVEN("Copy Assignment Operator"){
        int originalData[5] = {1, 2, 3, 4, 5};
        Automem automem1(sizeof(originalData), originalData);
        Automem automem2(10);
        automem2 = automem1;
        const int* hostData = static_cast<const int*>(automem2.host_data());
        for (int i = 0; i < 5; ++i) {
            CHECK(hostData[i] == originalData[i]);
        }
    }

    GIVEN("Move Assignment Operator"){
        int originalData[5] = {1, 2, 3, 4, 5};
        Automem automem1(sizeof(originalData), originalData);
        Automem automem2(10);
        automem2 = std::move(automem1);
        const int* hostData = static_cast<const int*>(automem2.host_data());
        for (int i = 0; i < 5; ++i) {
            CHECK(hostData[i] == originalData[i]);
        }
    }
}

