# Base de rust
## FR:

`#![no_std]`
Ca veux dire de ne pas inclure automatiquement la librairie standard

`#[repr(C)]`
Cela fait comprendre à Rust qu'il doit lire cette structure comme une structure C

`#[unsafe(no_mangle)]`
Cette ligne empêche le compilateur de renommer la fonction main

`pub`
Il est utilisé pour rendre la structure accessible depuis d'autres modules

`extern "C"`
Il indique que la fonction utilise la convention d'appel C

`-> i32`
Il indique que la fonction retourne un entier de 32 bits

`use core::ffi::{c_char, c_void}`
C'est plus ou moins equivalent à `from core.ffi import c_char, c_void`

`*mut` est l'inverse de `*const`
`*mut` est un pointeur mutable, tandis que `*const` est un pointeur constant. Cela signifie que vous pouvez modifier la valeur pointée par un pointeur mutable, mais pas par un pointeur constant.
