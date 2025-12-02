<div align="center">
    <img src="web\quickkart\core\static\logo.png" width="128px" style="max-width:100%;">
    <h1>QuickKart Hub</h1>
</div>

QuickKart Hub is a hybrid **C + Django** project designed to simulate a functional e-commerce platform.  
All core business logic (products, users, orders, invoices, recommendations, tickets) is fully implemented in **C**, while **Django + Tailwind CSS** provides a clean modern web interface.

This project was built as a college project to demonstrate:
- Use of **C for backend logic**
- Integration with **Python/Django** through `ctypes`
- A complete working e-commerce flow

<br>

## ✨ Features

### 🛒 **User Panel**
- Browse products with details
- Add products to cart
- Manage cart and checkout
- Automatic invoice generation
- Product recommendation system

### 🎛️ **Admin Panel**
- Dashboard with quick stats
- View all users, products, and orders
- Manage stock via C functions

### 💬 **Support System**
- Raise support tickets  
- Support team replies (C-based message thread)
- Auto-closing of tickets after support response
- View ticket history

### ⚙️ **Core Built in C**
- Products, Users, Orders, Inventory
- File-based data handling
- Order prioritization
- Ticket management
- Recommendation engine

<br>

## 🛠️ Installation

Clone the repository:

```bash
git clone https://github.com/divyanshsood22/quickkart-hub.git
```

Run Django server:

```bash
cd quickkart-hub/web/quickkart
python manage.py migrate
python manage.py runserver
```
C Compilation:
```
cd quickkart-hub/csrc
gcc -shared -o quickkart.dll products.c users.c orders.c invoice.c recommendation.c tickets.c ticket_replies.c utils.c inventory.c -I .
```
<br>

## 📝 License
This project is licensed under the Unlicense License.
See the [LICENSE](LICENSE) file for more details.
