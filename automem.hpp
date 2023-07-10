#include <cnrt.h>
#include <cstring>
#include <memory>

class Automem {
public:
    enum class HEAD { NONE, CPU, MLU };

private:
    void* cpu_ptr_;
    void* mlu_ptr_;

    HEAD head_ = HEAD::NONE;
    size_t length_;

public:
    Automem(size_t length)
        : length_(length), cpu_ptr_(nullptr), mlu_ptr_(nullptr), head_(HEAD::NONE) {}

    Automem(size_t length, void* data)
        : length_(length), cpu_ptr_(nullptr), mlu_ptr_(nullptr), head_(HEAD::CPU) {
        malloc_host();
        memcpy(cpu_ptr_, data, length_);
    }

    Automem(void* data, size_t length)
        : length_(length), cpu_ptr_(nullptr), mlu_ptr_(nullptr), head_(HEAD::MLU) {
        malloc_device();
        CNRT_CHECK(cnrtMemcpy(mlu_ptr_, data, length_, cnrtMemcpyDevToDev));
    }

    Automem(const Automem& other)
        : length_(other.length_), cpu_ptr_(nullptr), mlu_ptr_(nullptr), head_(other.head_) {
        if (other.cpu_ptr_ != nullptr) {
            malloc_host();
            memcpy(cpu_ptr_, other.cpu_ptr_, length_);
        }
        if (other.mlu_ptr_ != nullptr) {
            malloc_device();
            CNRT_CHECK(cnrtMemcpy(mlu_ptr_, other.mlu_ptr_, length_, cnrtMemcpyDevToDev));
        }
    }

    Automem& operator=(const Automem& other) {
        if (this != &other) {
            length_ = other.length_;
            head_ = other.head_;

            if (other.cpu_ptr_ != nullptr) {
                malloc_host();
                memcpy(cpu_ptr_, other.cpu_ptr_, length_);
            }

            if (other.mlu_ptr_ != nullptr) {
                malloc_device();
                CNRT_CHECK(cnrtMemcpy(mlu_ptr_, other.mlu_ptr_, length_, cnrtMemcpyDevToDev));
            }
        }
        return *this;
    }

    Automem& operator=(Automem&& other) {
        if (this != &other) {
            length_ = other.length_;
            head_ = other.head_;

            if (other.cpu_ptr_ != nullptr) {
                cpu_ptr_ = other.cpu_ptr_;
                other.cpu_ptr_ = nullptr;
            }

            if (other.mlu_ptr_ != nullptr) {
                mlu_ptr_ = other.mlu_ptr_;
                other.mlu_ptr_ = nullptr;
            }
        }
        return *this;
    }


    ~Automem() {
        if (cpu_ptr_ != nullptr) {
            free(cpu_ptr_);
        }

        if (mlu_ptr_ != nullptr) {
            CNRT_CHECK(cnrtFree(mlu_ptr_));
        }
    }

    const void* host_data() {
        if (cpu_ptr_ == nullptr) {
            malloc_host();
        }
        if (head_ == HEAD::MLU) {
            copy_to_host();
        }
        head_ = HEAD::NONE;
        return cpu_ptr_;
    }

    const void* device_data() {
        if (mlu_ptr_ == nullptr) {
            malloc_device();
        }

        if (head_ == HEAD::CPU) {
            copy_to_device();
        }
        head_ = HEAD::NONE;
        return mlu_ptr_;
    }

    void* mutable_host_data() {
        if (cpu_ptr_ == nullptr) {
            malloc_host();
        }
        head_ = HEAD::CPU;
        return cpu_ptr_;
    }

    void* mutable_device_data() {
        if (mlu_ptr_ == nullptr) {
            malloc_device();
        }
        head_ = HEAD::MLU;
        return mlu_ptr_;
    }

private:
    bool malloc_host() {
        cpu_ptr_ = malloc(length_);
        return cpu_ptr_ != nullptr;
    }

    bool malloc_device() {
        CNRT_CHECK(cnrtMalloc(&mlu_ptr_, length_));
        return mlu_ptr_ != nullptr;
    }

    bool copy_to_host() {
        CNRT_CHECK(cnrtMemcpy(cpu_ptr_, mlu_ptr_, length_, cnrtMemcpyDevToHost));
        return true;
    }

    bool copy_to_device() {
        CNRT_CHECK(cnrtMemcpy(mlu_ptr_, cpu_ptr_, length_, cnrtMemcpyHostToDev));
        return true;
    }
};
