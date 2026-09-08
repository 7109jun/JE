//! Safety-focused support layer for JE.
//! Rust is intentionally kept at validation boundaries; C owns PE, file I/O,
//! memory and Windows API integration.

#[repr(C)]
#[derive(Debug, Copy, Clone, Eq, PartialEq)]
pub enum JeSafeStatus { Ok = 0, Invalid = 1 }

#[no_mangle]
pub extern "C" fn je_safe_validate_utf8(ptr: *const u8, len: usize) -> JeSafeStatus {
    if ptr.is_null() && len != 0 { return JeSafeStatus::Invalid; }
    let bytes = if len == 0 { &[] } else { unsafe { std::slice::from_raw_parts(ptr, len) } };
    if std::str::from_utf8(bytes).is_ok() { JeSafeStatus::Ok } else { JeSafeStatus::Invalid }
}

#[no_mangle]
pub extern "C" fn je_safe_add_usize(a: usize, b: usize, out: *mut usize) -> JeSafeStatus {
    if out.is_null() { return JeSafeStatus::Invalid; }
    match a.checked_add(b) {
        Some(v) => { unsafe { *out = v; } JeSafeStatus::Ok }
        None => JeSafeStatus::Invalid,
    }
}
