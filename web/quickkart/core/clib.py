# core/clib.py
import os
import sys
import json
from ctypes import (
    Structure,
    c_int,
    c_float,
    c_char,
    c_char_p,
    c_void_p,
    cdll,
)

# path to the DLL (adjust if your layout differs)
DLL_PATH = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "csrc", "quickkart.dll")
)

print("Loading DLL from:", DLL_PATH)
if not os.path.exists(DLL_PATH):
    raise FileNotFoundError(f"DLL not found at: {DLL_PATH}")

_raw = cdll.LoadLibrary(DLL_PATH)


# ---------- ProductStruct definition ----------
class ProductStruct(Structure):
    _fields_ = [
        ("id", c_int),
        ("name", c_char * 100),
        ("price", c_float),
        ("stock", c_int),
        ("category", c_char * 50),
    ]


# ---------- helper to bind functions safely ----------
def try_bind(lib, name, argtypes=None, restype=None):
    """
    Try to get `name` from cdll library and set argtypes/restype.
    If not found, return a stub that raises RuntimeError when called.
    """
    try:
        func = getattr(lib, name)
    except AttributeError:
        # warn now, return a stub that raises when invoked
        print(f"WARNING: export '{name}' not found in DLL -> creating stub.")

        def _missing(*args, **kwargs):
            raise RuntimeError(
                f"DLL export '{name}' not found. Rebuild quickkart.dll with this exported symbol."
            )

        return _missing

    # set types if provided (ignore if None)
    if argtypes is not None:
        try:
            func.argtypes = argtypes
        except Exception as e:
            print(f"Warning: couldn't set argtypes for {name}: {e}", file=sys.stderr)
    if restype is not None:
        try:
            func.restype = restype
        except Exception as e:
            print(f"Warning: couldn't set restype for {name}: {e}", file=sys.stderr)
    return func


# ---------- Build wrapper 'lib' object with expected functions ----------
class LibProxy:
    pass


lib = LibProxy()

# === PRODUCTS ===
setattr(
    lib,
    "add_product",
    try_bind(_raw, "add_product", [c_char_p, c_float, c_int, c_char_p], c_int),
)
setattr(lib, "remove_product", try_bind(_raw, "remove_product", [c_int], c_int))
setattr(lib, "list_products", try_bind(_raw, "list_products", [], c_char_p))
setattr(lib, "get_product", try_bind(_raw, "get_product", [c_int], c_char_p))
# get_product_struct returns ProductStruct by value
setattr(
    lib,
    "get_product_struct",
    try_bind(_raw, "get_product_struct", [c_int], ProductStruct),
)
setattr(
    lib,
    "get_products_by_category",
    try_bind(_raw, "get_products_by_category", [c_char_p, c_void_p], c_int),
)
setattr(
    lib,
    "update_product_stock",
    try_bind(_raw, "update_product_stock", [c_int, c_int], c_int),
)
setattr(lib, "update_price", try_bind(_raw, "update_price", [c_int, c_float], c_int))

# === USERS ===
setattr(lib, "add_user", try_bind(_raw, "add_user", [c_char_p, c_char_p], c_int))
setattr(lib, "user_exists", try_bind(_raw, "user_exists", [c_char_p], c_int))
setattr(lib, "list_users", try_bind(_raw, "list_users", [], c_char_p))

# === ORDER GROUP SYSTEM ===
setattr(lib, "create_order_group", try_bind(_raw, "create_order_group", [c_int], c_int))
setattr(
    lib,
    "place_order_group",
    try_bind(_raw, "place_order_group", [c_int, c_int, c_int], c_int),
)
setattr(
    lib,
    "place_order",  # older single-order API (some code may still call it)
    try_bind(_raw, "place_order", [c_int, c_int, c_int], c_int),
)
setattr(
    lib, "get_orders_for_user", try_bind(_raw, "get_orders_for_user", [c_int], c_char_p)
)
setattr(lib, "list_all_orders", try_bind(_raw, "list_all_orders", [], c_char_p))
setattr(
    lib,
    "get_invoice_for_group",
    try_bind(_raw, "get_invoice_for_group", [c_int], c_char_p),
)
setattr(
    lib, "get_recommendations", try_bind(_raw, "get_recommendations", [c_int], c_char_p)
)

# === TICKETS ===
# DLL function signatures we expect (based on your headers):
#   int create_ticket(int user_id, const char* message, int priority);
#   char* list_tickets();                    -> returns JSON array of tickets
#   int close_ticket(int ticket_id);
#   int reply_ticket(int ticket_id, const char* message, const char* author);
#   char* list_replies(int ticket_id);

_raw_create_ticket = try_bind(_raw, "create_ticket", [c_int, c_char_p, c_int], c_int)
_raw_list_tickets = try_bind(_raw, "list_tickets", [], c_char_p)
_raw_close_ticket = try_bind(_raw, "close_ticket", [c_int], c_int)
_raw_reply_ticket = try_bind(_raw, "reply_ticket", [c_int, c_char_p, c_char_p], c_int)
_raw_list_replies = try_bind(_raw, "list_replies", [c_int], c_char_p)

# Attach raw functions so other code can call them directly if needed
setattr(lib, "_raw_create_ticket", _raw_create_ticket)
setattr(lib, "_raw_list_tickets", _raw_list_tickets)
setattr(lib, "_raw_close_ticket", _raw_close_ticket)
setattr(lib, "_raw_reply_ticket", _raw_reply_ticket)
setattr(lib, "_raw_list_replies", _raw_list_replies)


# Now provide friendly wrappers with the behaviour your views expect:
def _py_create_ticket(user_id, msg_bytes, priority):
    # msg_bytes may be bytes coming from views; ensure bytes->c_char_p OK
    if isinstance(msg_bytes, str):
        msg_bytes = msg_bytes.encode()
    return _raw_create_ticket(int(user_id), msg_bytes, int(priority))


def _py_list_tickets(uid=None):
    """
    If uid is None -> return the full bytes from DLL (or stub)
    If uid provided -> filter results by user_id and return bytes JSON of filtered list
    """
    raw = _raw_list_tickets()
    if not raw:
        return b"[]"
    # raw is bytes or c_char_p; ensure bytes
    if isinstance(raw, bytes):
        data = json.loads(raw.decode())
    else:
        try:
            data = json.loads(raw.decode())
        except Exception:
            # if raw is already a str (unlikely), handle that
            data = json.loads(raw)
    if uid is None:
        return json.dumps(data).encode()
    # filter
    filtered = [t for t in data if int(t.get("user_id", -1)) == int(uid)]
    return json.dumps(filtered).encode()


def _py_close_ticket(tid):
    return _raw_close_ticket(int(tid))


def _py_reply_ticket(tid, msg_bytes, author_bytes):
    if isinstance(msg_bytes, str):
        msg_bytes = msg_bytes.encode()
    if isinstance(author_bytes, str):
        author_bytes = author_bytes.encode()
    return _raw_reply_ticket(int(tid), msg_bytes, author_bytes)


def _py_list_replies(tid):
    raw = _raw_list_replies(int(tid))
    if not raw:
        return b"[]"
    if isinstance(raw, bytes):
        return raw
    try:
        return raw.decode().encode()
    except Exception:
        return raw


# === TICKETS ===
setattr(
    lib,
    "create_ticket",
    try_bind(_raw, "create_ticket", [c_int, c_char_p, c_int], c_int)
)

setattr(
    lib,
    "list_tickets",
    try_bind(_raw, "list_tickets", [c_int], c_char_p)
)

setattr(
    lib,
    "close_ticket",
    try_bind(_raw, "close_ticket", [c_int], c_int)
)

setattr(
    lib,
    "reply_ticket",
    try_bind(_raw, "reply_ticket", [c_int, c_char_p, c_char_p], c_int)
)

setattr(
    lib,
    "list_replies",
    try_bind(_raw, "list_replies", [c_int], c_char_p)
)


# === INVOICE helpers (backwards compatibility) ===
setattr(lib, "generate_invoice", try_bind(_raw, "generate_invoice", [c_int], c_char_p))

# Print summary of bindings (helpful on startup)
print("clib loaded. bound functions (missing ones showed as warnings above).")
