# c_the_mutex
Your best homemade mutex interface for C (POSIX only for now). Also found in: 
## Explanation
- 
## Usage
`c_the_mutex_posix.h` is intended for use with the gcc compiler, as the mutex guards rely on `__attribute__((cleanup()))`. 

In Windows, Msys2 is required for this.
1. Install Msys2.
2. After installing it, open a console for one of the environments (UCRT64 is objectively the best).
3. Install the gcc toolkit: ```pacman -S mingw-w64-ucrt-x86_64-gcc ```
4. Put the path to said environment in the Environment Variable: Path (Remember: C:\msys64\ucrt64\bin).
5. Enjoy like the rest. 
 
Once you have access to a gcc compiler, there are mainly three quality of life features for mutexes. 

## QOL Features

1. A wrapper for the data with the mutex. Something that you could easily implement yourself if you wanted to, a wrapper for the mutex with a void pointer for your data associated to said mutex. 
    -A couple of functions to initalize and free the data properly (all the wrapper allocations are handled by the user though). The data pointer is used as a canary to avoid locking, unlocking or destroying destroyed mutexes. 
    - A nice extra functionality in a couple of functions where the mutex lock returns the data pointer (void* in this case) and NULLs it in the wrapper, and in order to unlock said data you need to give it back as an argument, it being NULLed in your end. This serves to avoid double locking or unlocking of mutexes, as the data pointer in the wrapper and in the user side represents the mutex state. 
    - All of this is also available for generic type wrappers, where you can declare the wrapper type with your custom pointer type in the data slot, and get these functions declared too. Pointer types are not supported.
2. A simple code block wrapper for a single mutex. Easy to copy-paste, you can ensure that the mutex is properly locked before your code block and unlocked afterwards.
3. Rust-styled mutex guards! Both in `void*` and generic pointer form, you can create a guard that lives on the stack. This guard automatically locks the mutex that the input wrapper (see 1.) contains and gives you access to the data after the locking. Once it goes out of scope, the guard is automatically destroyed and the mutex is freed. For the custom types, the guard requires the custom type to be entered as an additional argument.

## Examples

See [example.c](./example.c) for a small demonstration of using the custom mutex guards in a threaded program using POSIX. It really is easy to use.


## Not finished as of v0.0.1

- Before make the guard have type of mutex with conditional to posix or windows.

- Then make the locking and unlocking functions be accepted via the macro, making it customizable.

- Add trylocking guards, and additional features.
