import os
from ctypes import cdll, c_char_p, c_int, c_float

DLL_PATH = os.path.abspath(os.path.join(
    os.path.dirname(__file__),
    "../../..",
    "build",
    "quickkart.dll"
))

lib = cdll.LoadLibrary(DLL_PATH)

# ==== PRODUCT FUNCTIONS ====
lib.add_product.argtypes = [c_char_p, c_float, c_int]
lib.add_product.restype = c_int
lib.remove_product.argtypes = [c_int]
lib.remove_product.restype = c_int
lib.list_products.argtypes = []
lib.list_products.restype = c_char_p
lib.get_product.argtypes = [c_int]
lib.get_product.restype = c_char_p

# ==== INVENTORY ====
lib.update_stock.argtypes = [c_int, c_int]
lib.update_stock.restype = c_int
lib.get_stock.argtypes = [c_int]
lib.get_stock.restype = c_int

# ==== USERS ====
lib.add_user.argtypes = [c_char_p, c_char_p]
lib.add_user.restype = c_int
lib.user_exists.argtypes = [c_char_p]
lib.user_exists.restype = c_int
lib.list_users.argtypes = []
lib.list_users.restype = c_char_p

# ==== ORDERS ====
lib.place_order.argtypes = [c_int, c_int, c_int]
lib.place_order.restype = c_int
lib.get_orders_for_user.argtypes = [c_int]
lib.get_orders_for_user.restype = c_char_p
lib.list_all_orders.argtypes = []
lib.list_all_orders.restype = c_char_p

# ==== TICKETS ====
lib.create_ticket.argtypes = [c_int, c_char_p, c_int]
lib.create_ticket.restype = c_int
lib.list_tickets.argtypes = []
lib.list_tickets.restype = c_char_p
lib.close_ticket.argtypes = [c_int]
lib.close_ticket.restype = c_int
lib.reply_ticket.argtypes = [c_int, c_char_p, c_char_p]
lib.reply_ticket.restype = c_int
lib.list_replies.argtypes = [c_int]
lib.list_replies.restype = c_char_p

# ==== INVOICE ====
lib.generate_invoice.argtypes = [c_int]
lib.generate_invoice.restype = c_char_p

# ==== RECOMMENDATIONS ====
lib.get_recommendations.argtypes = [c_int]
lib.get_recommendations.restype = c_char_p