from django.contrib import admin
from django.urls import path
from core.views import product_list, product_add, product_detail
from core.views import user_list, user_add
from core.views import cart_add, cart_view, cart_remove
from core.views import checkout, place_order
from core.views import invoice_view
from core.views import order_list, user_orders, order_detail
from core.views import admin_dashboard
from core.views import home
from core.views import support_list, support_create, support_close
from core.views import ticket_detail, ticket_reply


urlpatterns = [
    path("admin/", admin.site.urls),
    # Products
    path("products/", product_list),
    path("products/add/", product_add),
    path("products/<int:pid>/", product_detail),
    # Users
    path("users/", user_list),
    path("users/add/", user_add),
    # URLs
    path("cart/", cart_view),
    path("cart/add/<int:pid>/", cart_add),
    path("cart/remove/<int:pid>/", cart_remove),
    # Checkout
    path("checkout/", checkout),
    path("place-order/", place_order),
    # Invoice
    path("invoice/<int:oid>/", invoice_view),
    # Orders
    path("orders/", order_list),
    path("orders/user/<int:uid>/", user_orders),
    path("orders/<int:oid>/", order_detail),
    # Admin
    path("admin-dashboard/", admin_dashboard),
    # Homepage
    path("", home),
    # SUPPORT / TICKETS
    path("support/", support_list),
    path("support/create/", support_create),
    path("support/close/<int:tid>/", support_close),
    path("support/<int:tid>/", ticket_detail),
path("support/<int:tid>/reply/", ticket_reply),

]
