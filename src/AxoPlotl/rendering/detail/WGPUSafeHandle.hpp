// #include <webgpu/webgpu_cpp.h>
// #include <iostream>
// #include <utility>

// template <typename T>
// class WGPUSafeHandle {
// public:
//     WGPUSafeHandle() : handle_(nullptr) {}

//     WGPUSafeHandle(T handle) : handle_(handle) {}

//     ~WGPUSafeHandle() {
//         release();
//     }

//     // Prevent copying to avoid double-release errors
//     WGPUSafeHandle(const WGPUSafeHandle&) = delete;
//     WGPUSafeHandle& operator=(const WGPUSafeHandle&) = delete;

//     // Allow moving
//     WGPUSafeHandle(WGPUSafeHandle&& other) noexcept : handle_(other.handle_) {
//         other.handle_ = nullptr;
//     }

//     WGPUSafeHandle& operator=(WGPUSafeHandle&& other) noexcept {
//         if (this != &other) {
//             release();
//             handle_ = other.handle_;
//             other.handle_ = nullptr;
//         }
//         return *this;
//     }

//     T get() const {
//         return handle_;
//     }

//     operator T() const {
//         return handle_;
//     }

//     void release() {
//         if (handle_) {
//             handle_.release();
//             // wgpuBufferRelease(m_handle);
//             handle_ = nullptr;
//         }
//     }

// private:
//     T handle_;
// };
