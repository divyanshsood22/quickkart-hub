from django.shortcuts import render, redirect
from django.http import HttpResponse
from django.contrib import messages
from .clib import lib
import json


# ========== PRODUCTS ==========

def product_list(request):
    raw = lib.list_products()
    data = raw.decode() if raw else "[]"
    products = json.loads(data)
    return render(request, "products/list.html", {"products": products})


def product_detail(request, pid):
    raw = lib.get_product(int(pid))
    if not raw:
        return HttpResponse("Product not found", status=404)
    
    product = json.loads(raw.decode())

    rec_raw = lib.get_recommendations(int(pid))
    recommendations = json.loads(rec_raw.decode()) if rec_raw else []

    return render(request, "products/detail.html", {
        "product": product,
        "recommendations": recommendations
    })



def product_add(request):
    if request.method == "POST":
        name = request.POST["name"]
        price = float(request.POST["price"])
        stock = int(request.POST["stock"])

        lib.add_product(name.encode(), price, stock)
        return redirect("/products/")

    return render(request, "products/add.html")



# ========== USERS ==========

def user_list(request):
    raw = lib.list_users()
    data = raw.decode() if raw else "[]"
    users = json.loads(data)
    return render(request, "users/list.html", {"users": users})


def user_add(request):
    if request.method == "POST":
        name = request.POST["name"]
        email = request.POST["email"]

        uid = lib.add_user(name.encode(), email.encode())

        if uid == -1:
            return HttpResponse("User already exists with this email.", status=400)

        return redirect("/users/")

    return render(request, "users/add.html")



# ========== CART SYSTEM ==========

def cart_add(request, pid):
    pid = int(pid)

    raw = lib.get_product(pid)
    if not raw:
        return HttpResponse("Invalid product", status=404)

    product = json.loads(raw.decode())

    if product["stock"] <= 0:
        return HttpResponse("Out of stock", status=400)

    cart = request.session.get("cart", {})
    cart[str(pid)] = cart.get(str(pid), 0) + 1
    request.session["cart"] = cart

    return redirect("/cart/")


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

    return render(request, "cart/view.html", {"items": items})


def cart_remove(request, pid):
    pid = str(pid)
    cart = request.session.get("cart", {})

    if pid in cart:
        del cart[pid]

    request.session["cart"] = cart
    return redirect("/cart/")


# ========== CHECKOUT & ORDER PLACEMENT ==========

def checkout(request):
    cart = request.session.get("cart", {})
    items = []

    for pid, qty in cart.items():
        raw = lib.get_product(int(pid))
        if raw:
            product = json.loads(raw.decode())
            product["quantity"] = qty
            product["total"] = float(product["price"]) * qty
            items.append(product)

    u_raw = lib.list_users()
    users = json.loads(u_raw.decode()) if u_raw else []

    return render(request, "checkout/checkout.html", {
        "items": items,
        "users": users
    })


def place_order(request):
    if request.method != "POST":
        return HttpResponse("Invalid request", status=400)

    user_id = int(request.POST["user"])
    cart = request.session.get("cart", {})

    order_ids = []

    for pid, qty in cart.items():
        result = lib.place_order(user_id, int(pid), int(qty))
        order_ids.append(result)

    request.session["cart"] = {}

    return redirect(f"/invoice/{order_ids[0]}/")

# ========== INVOICE ==========

def invoice_view(request, oid):
    raw = lib.generate_invoice(int(oid))

    if not raw:
        return HttpResponse("Invoice not found", status=404)

    text = raw.decode()

    return render(request, "invoice/view.html", {"invoice": text})


# ========== ORDERS ==========

def order_list(request):
    raw = lib.list_all_orders()
    data = raw.decode() if raw else "[]"
    orders = json.loads(data)
    return render(request, "orders/list.html", {"orders": orders})


def user_orders(request, uid):
    raw = lib.get_orders_for_user(int(uid))
    data = raw.decode() if raw else "[]"
    orders = json.loads(data)
    return render(request, "orders/user_orders.html", {"orders": orders})


def order_detail(request, oid):
    # No direct C function: find order manually
    raw = lib.list_all_orders()
    data = raw.decode() if raw else "[]"
    orders = json.loads(data)

    selected = None
    for o in orders:
        if o["order_id"] == oid:
            selected = o
            break

    if not selected:
        return HttpResponse("Order not found", status=404)

    return render(request, "orders/detail.html", {"order": selected})

# ========== ADMIN DASHBOARD ==========

def admin_dashboard(request):
    p_raw = lib.list_products()
    products = json.loads(p_raw.decode()) if p_raw else []
    total_products = len(products)

    u_raw = lib.list_users()
    users = json.loads(u_raw.decode()) if u_raw else []
    total_users = len(users)

    o_raw = lib.list_all_orders()
    orders = json.loads(o_raw.decode()) if o_raw else []
    total_orders = len(orders)

    low_stock_items = [p for p in products if p["stock"] <= 2]

    recent_orders = sorted(orders, key=lambda x: x["order_id"], reverse=True)[:5]

    return render(request, "admin/dashboard.html", {
        "total_products": total_products,
        "total_users": total_users,
        "total_orders": total_orders,
        "low_stock_items": low_stock_items,
        "recent_orders": recent_orders
    })


def home(request):
    return render(request, "home.html")


# ========== SUPPORT / TICKETS ==========

def support_list(request):
    raw = lib.list_tickets()
    data = raw.decode() if raw else "[]"
    tickets = json.loads(data)

    # Attach latest reply
    for t in tickets:
        tid = t["ticket_id"]

        r_raw = lib.list_replies(tid)
        replies = json.loads(r_raw.decode()) if r_raw else []

        if replies:
            t["last_reply"] = {
                "message": replies[-1]["message"],
                "role": replies[-1]["role"],
                "time": replies[-1]["time"]
            }
        else:
            t["last_reply"] = None

    return render(request, "support/list.html", {"tickets": tickets})


def support_create(request):
    if request.method == "POST":
        user_id = int(request.POST.get("user_id", "0"))
        message = request.POST.get("message", "").strip()
        priority = int(request.POST.get("priority", "1"))

        if not message:
            return HttpResponse("Message required", status=400)

        tid = lib.create_ticket(user_id, message.encode(), priority)

        if tid <= 0:
            return HttpResponse("Error creating ticket", status=500)

        return redirect("/support/")

    # For dropdown users
    raw_users = lib.list_users()
    users = json.loads(raw_users.decode()) if raw_users else []

    return render(request, "support/create.html", {"users": users})


def support_close(request, tid):
    result = lib.close_ticket(int(tid))
    if result == 1:
        return redirect("/support/")
    else:
        return HttpResponse("Ticket not found", status=404)

def ticket_detail(request, tid):
    # Get ticket info
    raw = lib.list_tickets()
    tickets = json.loads(raw.decode()) if raw else []
    ticket = next((t for t in tickets if t["ticket_id"] == tid), None)

    if not ticket:
        return HttpResponse("Ticket not found", status=404)

    # Get replies
    r_raw = lib.list_replies(tid)
    replies = json.loads(r_raw.decode()) if r_raw else []

    return render(request, "support/detail.html", {"ticket": ticket, "replies": replies})


def ticket_reply(request, tid):
    if request.method == "POST":
        message = request.POST["message"]
        role = "support"

        # 1. Add reply
        lib.reply_ticket(tid, message.encode(), role.encode())

        # 2. Auto close ticket after support reply
        lib.close_ticket(tid)

        return redirect(f"/support/{tid}/")

    return HttpResponse("Invalid request", status=400)

