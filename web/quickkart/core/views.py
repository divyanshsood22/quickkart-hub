import json, os 
from django.shortcuts import render, redirect
from django.http import HttpResponse
from django.conf import settings
from django.contrib import messages
from .clib import lib
from ctypes import c_int

# ============================================================
# LOGIN SYSTEM
# ============================================================


def home_redirect(request):
    if not request.session.get("user_id"):
        return redirect("/login/")
    if request.session.get("is_admin"):
        return redirect("/admin-home/")
    return redirect("/home/")


def login_choice(request):
    return render(request, "auth/login_choice.html")


def login_user(request):
    if request.method == "POST":
        name = request.POST.get("name", "").strip()
        email = request.POST.get("email", "").strip()

        if email:
            uid = lib.add_user(name.encode() if name else b"Guest", email.encode())
            if uid == -1:
                raw = lib.list_users()
                users = json.loads(raw.decode()) if raw else []
                found = next((u for u in users if u["email"] == email), None)
                uid = found["id"]
        else:
            uid = 0  # guest user

        request.session["user_id"] = int(uid)
        request.session["user_name"] = name if name else "User " + str(uid)
        request.session["is_admin"] = False
        return redirect("/home/")

    return render(request, "auth/login_user.html")


def login_admin(request):
    if request.method == "POST":
        pwd = request.POST.get("password", "")
        admin_pass = getattr(settings, "ADMIN_PASS", "admin123")

        if pwd == admin_pass:
            request.session["is_admin"] = True
            request.session["user_id"] = -1
            request.session["user_name"] = "Admin"
            return redirect("/admin-home/")
        messages.error(request, "Incorrect admin password")
        return redirect("/login/admin/")

    return render(request, "auth/login_admin.html")


def logout_view(request):
    request.session.flush()
    return redirect("/login/")


# ============================================================
# USER HOME
# ============================================================


def user_home(request):
    if not request.session.get("user_id"):
        return redirect("/login/")
    return render(request, "home/user_home.html")


# ============================================================
# ADMIN HOME
# ============================================================


def admin_home(request):
    if not request.session.get("is_admin"):
        return redirect("/login/")
    return render(request, "admin/home.html")  # this is your admin panel main screen


# ============================================================
# PRODUCTS
# ============================================================

def products_user_list(request):
    raw = lib.list_products()
    products = json.loads(raw.decode()) if raw else []
    return render(request, "products/list.html", {"products": products})

def products_list(request):
    raw = lib.list_products()
    products = json.loads(raw.decode()) if raw else []
    return render(request, "admin/products/products_list.html", {"products": products})


def product_add(request):
    if not request.session.get("is_admin"):
        return redirect("/login/")

    if request.method == "POST":
        name = request.POST.get("name", "").strip()
        price = request.POST.get("price", "").strip()
        stock = request.POST.get("stock", "").strip()
        category = request.POST.get("category", "").strip()

        if not name or not price or not stock or not category:
            return HttpResponse("All fields are required.", status=400)

        try:
            price = float(price)
            stock = int(stock)
        except:
            return HttpResponse("Invalid price or stock.", status=400)

        product_id = lib.add_product(name.encode(), price, stock, category.encode())

        if product_id <= 0:
            return HttpResponse("Failed to add product", status=500)

        return redirect("/admin/products/")

    return render(request, "admin/products/add.html")


def product_remove(request):
    pid = request.GET.get("id") or request.POST.get("product_id")

    if not pid:
        return HttpResponse("Invalid product ID", status=400)

    pid = int(pid)

    if request.method == "POST":
        lib.remove_product(pid)
        return redirect("/admin/products/")

    return render(request, "admin/products/remove_product.html", {"pid": pid})


def product_update_stock(request):
    if request.method == "POST":
        product_id = request.POST.get("product_id")
        new_stock = request.POST.get("new_stock")

        if not product_id or not new_stock:
            return HttpResponse("Missing form data", status=400)

        lib.update_product_stock(int(product_id), int(new_stock))
        return redirect("/admin/products/")

def product_update_price(request):
    if request.method == "POST":
        product_id = request.POST.get("product_id")
        new_price = request.POST.get("new_price")

        if not product_id or not new_price:
            return HttpResponse("Missing form data", status=400)

        lib.update_price(int(product_id), float(new_price))
        return redirect("/admin/products/")


def product_update_stock_page(request, pid):
    product_json = lib.get_product(pid)
    if not product_json:
        return HttpResponse("Product not found", status=404)
    product = json.loads(product_json.decode())
    return render(request, "admin/products/update_stock.html", {"product": product})

def product_update_price_page(request, pid):
    product_json = lib.get_product(pid)
    if not product_json:
        return HttpResponse("Product not found", status=404)
    product = json.loads(product_json.decode())
    return render(request, "admin/products/update_price.html", {"product": product})


def product_detail(request, pid):
    raw = lib.get_product(int(pid))
    if not raw:
        return HttpResponse("Product not found.", status=404)
    product = json.loads(raw.decode())

    rec_raw = lib.get_recommendations(int(pid))
    recommendations = json.loads(rec_raw.decode()) if rec_raw else []

    return render(
        request,
        "products/detail.html",
        {"product": product, "recommendations": recommendations},
    )


# ============================================================
# CART SYSTEM
# ============================================================


def cart_add(request, pid):
    if request.method == "POST":
        qty = int(request.POST.get("qty", 1))
    else:
        qty = 1
    raw = lib.get_product(int(pid))
    if not raw:
        return HttpResponse("Invalid Product ID", status=404)

    product = json.loads(raw.decode())
    stock = int(product["stock"])

    if qty > stock:
        return HttpResponse("Not enough stock", status=400)

    cart = request.session.get("cart", {})
    cart[str(pid)] = cart.get(str(pid), 0) + qty
    request.session["cart"] = cart

    return redirect("/products/")

def user_products_list(request):
    raw = lib.list_products()
    products = json.loads(raw.decode()) if raw else []
    return render(request, "products/list.html", {"products": products})

def cart_view(request):
    cart = request.session.get("cart", {})
    items = []

    for pid, qty in cart.items():
        raw = lib.get_product(int(pid))
        if raw:
            product = json.loads(raw.decode())
            product["quantity"] = qty
            product["total"] = float(product["price"]) * qty
            items.append(product)

    return render(request, "cart/cart.html", {"items": items})


def cart_remove(request, pid):
    cart = request.session.get("cart", {})
    pid = str(pid)

    if pid in cart:
        del cart[pid]

    request.session["cart"] = cart
    return redirect("/cart/")


# ============================================================
# CHECKOUT + ORDER
# ============================================================


def checkout(request):
    cart = request.session.get("cart", {})
    items = []

    for pid, qty in cart.items():
        raw = lib.get_product(int(pid))
        if raw:
            p = json.loads(raw.decode())
            p["quantity"] = qty
            p["total"] = float(p["price"]) * qty
            items.append(p)

    return render(request, "checkout/checkout.html", {"items": items})

def place_order(request):
    if request.method != "POST":
        return HttpResponse("Invalid request")

    user_id = int(request.session.get("user_id", 0))
    cart = request.session.get("cart", {})

    if not cart:
        return HttpResponse("Cart is empty", status=400)

    group_id = lib.create_order_group(user_id)

    for pid_str, qty in cart.items():
        pid = int(pid_str)
        qty = int(qty)
        res = lib.place_order_group(group_id, pid, qty)

    request.session["cart"] = {}

    return redirect(f"/invoice/{group_id}/")



# ============================================================
# INVOICE
# ============================================================
def invoice_view(request, gid):
    raw = lib.get_invoice_for_group(gid)
    if not raw:
        data = {"order_id": gid, "items": [], "grand_total": 0}
    else:
        data = json.loads(raw.decode())

    return render(request, "invoice/invoice.html", {
        "order_id": data.get("order_id", gid),
        "items": data.get("items", []),
        "grand_total": data.get("grand_total", 0),
    })

# ============================================================
# SUPPORT SYSTEM (USER + ADMIN)
# ============================================================


def support_home(request):
    return render(request, "support/support_home.html")


def support_create(request):
    if request.method == "POST":
        uid = request.session.get("user_id")
        msg = request.POST["message"]
        priority = int(request.POST["priority"])

        lib.create_ticket(int(uid), msg.encode(), priority)
        return redirect("/support/my/")

    return render(request, "support/create.html")

def support_my(request):
    user_id = request.session.get("user_id")
    if not user_id:
        return render(request, "support/tickets.html", {"tickets": []})

    tickets_raw = lib.list_tickets(int(user_id))


    raw = lib.list_tickets(c_int(user_id))

    if not raw:
        return render(request, "support/tickets.html", {"tickets": []})

    data = json.loads(raw.decode())

    return render(request, "support/tickets.html", {"tickets": data})


def ticket_detail(request, tid):
    tid = int(tid)

    raw = lib.list_tickets()
    tickets = json.loads(raw.decode()) if raw else []
    ticket = next((t for t in tickets if t["ticket_id"] == tid), None)

    r_raw = lib.list_replies(tid)
    replies = json.loads(r_raw.decode()) if r_raw else []

    return render(
        request, 
        "support/ticket_detail.html",  # <-- FIXED
        {"ticket": ticket, "replies": replies}
    )


def ticket_reply(request, tid):
    if request.method == "POST":
        msg = request.POST["message"]
        lib.reply_ticket(int(tid), msg.encode(), b"support")
        lib.close_ticket(int(tid))
        return redirect(f"/support/ticket/{tid}/")

    return HttpResponse("Invalid request")


def admin_support_list(request):
    if not request.session.get("is_admin"):
        return redirect("/login/")

    raw = lib.list_tickets()
    tickets = json.loads(raw.decode()) if raw else []

    return render(request, "admin/support/support_list.html", {"tickets": tickets})


def admin_support_pending(request):
    if not request.session.get("is_admin"):
        return redirect("/login/")

    raw = lib.list_tickets()
    tickets = json.loads(raw.decode()) if raw else []
    pending = [t for t in tickets if t["status"] == "open"]

    return render(request, "admin/support/support_pending.html", {"tickets": pending})


def admin_support_resolved(request):
    if not request.session.get("is_admin"):
        return redirect("/login/")

    raw = lib.list_tickets()
    tickets = json.loads(raw.decode()) if raw else []
    resolved = [t for t in tickets if t["status"] == "closed"]

    return render(request, "admin/support/support_resolved.html", {"tickets": resolved})


def admin_support_detail(request, tid):
    if not request.session.get("is_admin"):
        return redirect("/login/")

    raw = lib.list_tickets()
    all_tickets = json.loads(raw.decode()) if raw else []

    ticket = next((t for t in all_tickets if t["ticket_id"] == tid), None)
    if not ticket:
        return HttpResponse("Ticket not found", status=404)

    r_raw = lib.list_replies(tid)
    replies = json.loads(r_raw.decode()) if r_raw else []

    return render(
        request,
        "admin/support/support_detail.html",
        {"ticket": ticket, "replies": replies},
    )


def admin_support_reply(request, tid):
    if not request.session.get("is_admin"):
        return redirect("/login/")

    if request.method == "POST":
        msg = request.POST.get("message", "").strip()
        lib.reply_ticket(tid, msg.encode(), b"admin")
        lib.close_ticket(tid)
        return redirect(f"/admin/support/{tid}/")

    return HttpResponse("Invalid request", status=400)


def admin_support_search(request):
    if not request.session.get("is_admin"):
        return redirect("/login/")

    query = request.GET.get("query", "").strip()

    raw = lib.list_tickets()
    tickets = json.loads(raw.decode()) if raw else []

    if query.isdigit():
        tickets = [t for t in tickets if t["ticket_id"] == int(query)]

    return render(
        request,
        "admin/support/support_search.html",
        {"tickets": tickets, "query": query},
    )

def admin_invoice_detail(request, group_id):
    if not request.session.get("is_admin"):
        return redirect("/login/")

    raw = lib.get_invoice_for_group(int(group_id))
    if not raw:
        return HttpResponse("Invoice not found", status=404)

    try:
        invoice = json.loads(raw.decode())
    except Exception:
        try:
            invoice = json.loads(raw)
        except Exception:
            return HttpResponse("Malformed invoice data", status=500)

    order_id = invoice.get("order_id", group_id)
    items = invoice.get("items", [])
    grand_total = invoice.get("grand_total", 0.0)

    return render(request, "admin/invoice/invoice_detail.html", {
        "order_id": order_id,
        "items": items,
        "grand_total": grand_total
    })

def admin_invoice_list(request):
    if not request.session.get("is_admin"):
        return redirect("/login/")

    orders_path = "D:/quickkart-hub/web/quickkart/orders.txt"

    groups = {}
    if os.path.exists(orders_path):
        with open(orders_path, "r") as f:
            for line in f:
                parts = line.strip().split("|")
                if len(parts) < 2:
                    continue

                gid = int(parts[0])
                uid = int(parts[1])

                groups[gid] = uid   # always latest user id (works fine)

    group_list = [{"group_id": gid, "user_id": uid} for gid, uid in groups.items()]

    return render(request, "admin/invoice/invoice_list.html", {"groups": group_list})
