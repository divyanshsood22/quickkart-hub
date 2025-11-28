from django.contrib import admin
from django.urls import path
from core import views

urlpatterns = [
    # LOGIN
    path("", views.home_redirect),
    path("login/", views.login_choice),
    path("login/user/", views.login_user),
    path("login/admin/", views.login_admin),
    path("logout/", views.logout_view),
    # USER HOME
    path("home/", views.user_home),
    # ADMIN HOME
    path("admin-home/", views.admin_home),
    # PRODUCTS ADMIN
    path("admin/products/", views.products_list),
    path("admin/products/add/", views.product_add),
    # REMOVE PRODUCT
    path("admin/products/remove/", views.product_remove, name="product_remove"),
    # UPDATE PAGES
    path(
        "admin/products/stock/<int:pid>/",
        views.product_update_stock_page,
        name="update_stock_page",
    ),
    path(
        "admin/products/price/<int:pid>/",
        views.product_update_price_page,
        name="update_price_page",
    ),
    # UPDATE ACTIONS
    path(
        "admin/products/update-stock/", views.product_update_stock, name="update_stock"
    ),
    path(
        "admin/products/update-price/", views.product_update_price, name="update_price"
    ),
    # FRONTEND PRODUCT DETAIL
    path("products/<int:pid>/", views.product_detail),
    # CART
    path("cart/", views.cart_view),
    path("cart/add/<int:pid>/", views.cart_add),
    path("cart/remove/<int:pid>/", views.cart_remove),
    # CHECKOUT + INVOICE
    path("checkout/", views.checkout),
    path("place-order/", views.place_order),
    path("invoice/<int:gid>/", views.invoice_view),
    # SUPPORT (USER)
    path("support/", views.support_home),
    path("support/create/", views.support_create),
    path("support/my/", views.support_my),
    path("support/ticket/<int:tid>/", views.ticket_detail),
    path("support/ticket/<int:tid>/reply/", views.ticket_reply),
    # DJANGO BUILT-IN ADMIN
    path("django-admin/", admin.site.urls),
    # SUPPORT (ADMIN)
    path("admin/support/", views.admin_support_list),
    path("admin/support/pending/", views.admin_support_pending),
    path("admin/support/resolved/", views.admin_support_resolved),
    path("admin/support/search/", views.admin_support_search),
    path("admin/support/<int:tid>/", views.admin_support_detail),
    path("admin/support/<int:tid>/reply/", views.admin_support_reply),
    # ADMIN INVOICES
    path("admin/invoices/", views.admin_invoice_list),
    path("admin/invoices/<int:group_id>/", views.admin_invoice_detail),
    path("products/", views.user_products_list, name="user_products_list"),
    path("products/<int:pid>/", views.product_detail),
    path("cart/", views.cart_view),
    path("cart/add/<int:pid>/", views.cart_add),
    path("cart/remove/<int:pid>/", views.cart_remove),
    path("checkout/", views.checkout),
    path("place-order/", views.place_order),
]
