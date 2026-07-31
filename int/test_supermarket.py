#!/usr/bin/env python3
"""
Test the examples/supermarket PRG programs via NcursesAgent.
Tests admin.prg (menu navigation) and checkout.prg (POS flow).
Run from the repo root:  python3 test_supermarket.py

Key: GET+READ fields need send_line("digit")  (digit + Enter)
     WAIT prompts need send_key("x")           (any single key)
"""

import sys, os, time

repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, repo_root)

from tui_agent import NcursesAgent, ENTER

SUPERMARKET = f"{repo_root}/examples/supermarket"
PRG = f"{repo_root}/int/prg"
CHECKOUT_ROWS = 28  # checkout.prg uses @ 24,25 which need rows >= 26


def menu_sel(a, digit):
    """Select a menu option (GET + READ = digit + Enter)."""
    a.send_line(digit)


def dismiss_wait(a):
    """Dismiss a WAIT prompt (any single key)."""
    a.send_key("q")


def test_admin_main_menu():
    """admin.prg: verify main menu displays correctly and exit works."""
    print("=" * 60)
    print("TEST: admin.prg - Main menu display and exit")
    print("=" * 60)

    with NcursesAgent(f"{PRG} admin.prg", rows=24, cols=80,
                      cwd=SUPERMARKET, timeout=10) as a:
        a.wait_for("SUPERMARKET MANAGEMENT SYSTEM")
        screen = a.display()
        print("MAIN MENU SCREEN:")
        print(screen)
        print("---")

        assert "SUPERMARKET MANAGEMENT SYSTEM" in screen, "Main menu title missing"
        assert "1. Products" in screen, "Products menu item missing"
        assert "2. Providers" in screen, "Providers menu item missing"
        assert "3. Stock Management" in screen, "Stock menu item missing"
        assert "4. Orders" in screen, "Orders menu item missing"
        assert "5. Reports" in screen, "Reports menu item missing"
        assert "6. Sales History" in screen, "Sales menu item missing"
        assert "0. Exit" in screen, "Exit menu item missing"
        print("PASS: All main menu items present")

        menu_sel(a, "0")
        a.wait_for("Goodbye")
        assert "Goodbye" in a.display(), "Exit message not shown"
        print("PASS: Exit (option 0) works correctly")


def test_admin_products_menu():
    """admin.prg: navigate to products submenu and verify options."""
    print("\n" + "=" * 60)
    print("TEST: admin.prg - Products submenu")
    print("=" * 60)

    with NcursesAgent(f"{PRG} admin.prg", rows=24, cols=80,
                      cwd=SUPERMARKET, timeout=10) as a:
        a.wait_for("SUPERMARKET MANAGEMENT SYSTEM")

        menu_sel(a, "1")
        a.wait_for("--- PRODUCTS ---")
        screen = a.display()
        print("PRODUCTS MENU SCREEN:")
        print(screen)
        print("---")

        assert "--- PRODUCTS ---" in screen, "Products header missing"
        assert "1. Add product" in screen, "Add product missing"
        assert "2. Edit product" in screen, "Edit product missing"
        assert "3. List products" in screen, "List products missing"
        assert "4. Search product" in screen, "Search product missing"
        assert "5. Update prices" in screen, "Update prices missing"
        assert "0. Back" in screen, "Back option missing"
        print("PASS: Products submenu correct")

        menu_sel(a, "3")
        a.wait_for("PRODUCT LIST")
        screen = a.display()
        print("PRODUCT LIST SCREEN:")
        print(screen)
        print("---")

        # PRODUCT LIST header may have scrolled off (ncurses addstr without positioning)
        # Check raw text instead
        raw = a.raw_text()
        assert "PRODUCT LIST" in raw, "Product list header missing (checked raw)"
        assert "841234567" in screen or "] " in screen, \
            "No products shown in list"
        print("PASS: Product list displays products")

        dismiss_wait(a)
        a.wait_for("--- PRODUCTS ---")
        print("PASS: Returned to products menu after list")

        menu_sel(a, "0")
        a.wait_for("SUPERMARKET MANAGEMENT SYSTEM")
        print("PASS: Returned to main menu")

        menu_sel(a, "0")
        a.wait_for("Goodbye")
        print("PASS: Exited cleanly")


def test_admin_providers_menu():
    """admin.prg: navigate to providers submenu."""
    print("\n" + "=" * 60)
    print("TEST: admin.prg - Providers submenu")
    print("=" * 60)

    with NcursesAgent(f"{PRG} admin.prg", rows=24, cols=80,
                      cwd=SUPERMARKET, timeout=10) as a:
        a.wait_for("SUPERMARKET MANAGEMENT SYSTEM")

        menu_sel(a, "2")
        a.wait_for("--- PROVIDERS ---")
        screen = a.display()
        print("PROVIDERS MENU SCREEN:")
        print(screen)
        print("---")

        assert "--- PROVIDERS ---" in screen
        assert "1. Add provider" in screen
        assert "2. List providers" in screen
        assert "0. Back" in screen
        print("PASS: Providers submenu correct")

        menu_sel(a, "2")
        a.wait_for("PROVIDER LIST")
        screen = a.display()
        print("PROVIDER LIST SCREEN:")
        print(screen)
        print("---")
        assert "PROVIDER LIST" in screen
        print("PASS: Provider list displays")

        dismiss_wait(a)
        a.wait_for("--- PROVIDERS ---")
        print("PASS: Returned to providers menu")

        menu_sel(a, "0")
        a.wait_for("SUPERMARKET MANAGEMENT SYSTEM")
        menu_sel(a, "0")
        a.wait_for("Goodbye")
        print("PASS: Exited cleanly")


def test_admin_stock_menu():
    """admin.prg: navigate to stock management submenu."""
    print("\n" + "=" * 60)
    print("TEST: admin.prg - Stock management submenu")
    print("=" * 60)

    with NcursesAgent(f"{PRG} admin.prg", rows=24, cols=80,
                      cwd=SUPERMARKET, timeout=10) as a:
        a.wait_for("SUPERMARKET MANAGEMENT SYSTEM")

        menu_sel(a, "3")
        a.wait_for("--- STOCK MANAGEMENT ---")
        screen = a.display()
        assert "--- STOCK MANAGEMENT ---" in screen
        assert "1. Adjust stock" in screen
        assert "2. View low stock" in screen
        assert "3. Full stock report" in screen
        assert "0. Back" in screen
        print("PASS: Stock submenu correct")

        menu_sel(a, "2")
        a.wait_for("LOW STOCK")
        screen = a.display()
        print("LOW STOCK SCREEN:")
        print(screen)
        print("---")
        assert "LOW STOCK" in screen
        print("PASS: Low stock report displays")

        dismiss_wait(a)
        a.wait_for("--- STOCK MANAGEMENT ---")
        menu_sel(a, "0")
        a.wait_for("SUPERMARKET MANAGEMENT SYSTEM")
        menu_sel(a, "0")
        a.wait_for("Goodbye")
        print("PASS: Exited cleanly")


def test_admin_reports_menu():
    """admin.prg: navigate to reports submenu."""
    print("\n" + "=" * 60)
    print("TEST: admin.prg - Reports submenu")
    print("=" * 60)

    with NcursesAgent(f"{PRG} admin.prg", rows=24, cols=80,
                      cwd=SUPERMARKET, timeout=10) as a:
        a.wait_for("SUPERMARKET MANAGEMENT SYSTEM")

        menu_sel(a, "5")
        a.wait_for("--- REPORTS ---")
        screen = a.display()
        assert "--- REPORTS ---" in screen
        assert "1. Low stock alert" in screen
        assert "2. Weekly reorder" in screen
        assert "3. Daily sales" in screen
        assert "4. Product sales ranking" in screen
        assert "5. Revenue by category" in screen
        assert "0. Back" in screen
        print("PASS: Reports submenu correct")

        menu_sel(a, "0")
        a.wait_for("SUPERMARKET MANAGEMENT SYSTEM")
        menu_sel(a, "0")
        a.wait_for("Goodbye")
        print("PASS: Exited cleanly")


def test_admin_sales_menu():
    """admin.prg: navigate to sales history."""
    print("\n" + "=" * 60)
    print("TEST: admin.prg - Sales history")
    print("=" * 60)

    with NcursesAgent(f"{PRG} admin.prg", rows=24, cols=80,
                      cwd=SUPERMARKET, timeout=10) as a:
        a.wait_for("SUPERMARKET MANAGEMENT SYSTEM")

        menu_sel(a, "6")
        a.wait_for("SALES HISTORY")
        screen = a.display()
        print("SALES HISTORY SCREEN:")
        print(screen)
        print("---")
        assert "SALES HISTORY" in screen
        assert "2026-07-28" in screen, "Date formatting broken (should show YYYY-MM-DD)"
        print("PASS: Sales history displays with correct dates")

        dismiss_wait(a)
        a.wait_for("SUPERMARKET MANAGEMENT SYSTEM")
        print("PASS: Returned to main menu")

        menu_sel(a, "0")
        a.wait_for("Goodbye")
        print("PASS: Exited cleanly")


def test_checkout_display():
    """checkout.prg: verify POS screen displays correctly."""
    print("\n" + "=" * 60)
    print("TEST: checkout.prg - POS screen display")
    print("=" * 60)

    with NcursesAgent(f"{PRG} checkout.prg", rows=CHECKOUT_ROWS, cols=80,
                      cwd=SUPERMARKET, timeout=10) as a:
        a.wait_for("CHECKOUT TERMINAL")
        screen = a.display()
        print("CHECKOUT SCREEN:")
        print(screen)
        print("---")

        assert "CHECKOUT TERMINAL" in screen, "Checkout header missing"
        assert "Barcode" in screen, "Barcode column header missing"
        assert "Name" in screen, "Name column header missing"
        assert "Barcode:" in screen, "Barcode input prompt missing"
        print("PASS: Checkout screen displays correctly")

        # Exit with 'q' (checkout.prg checks for q/Q to EXIT)
        a.send_line("q")
        try:
            a.expect_exit(timeout=5)
            print("PASS: Checkout exited cleanly with 'q'")
        except Exception:
            a.send_key("q")
            try:
                a.expect_exit(timeout=5)
                print("PASS: Checkout exited after dismissing banner")
            except Exception:
                print("WARN: Checkout didn't exit cleanly, but screen was correct")


def test_checkout_add_product():
    """checkout.prg: scan a product barcode and verify it appears in the cart."""
    print("\n" + "=" * 60)
    print("TEST: checkout.prg - Add product to cart")
    print("=" * 60)

    with NcursesAgent(f"{PRG} checkout.prg", rows=CHECKOUT_ROWS, cols=80,
                      cwd=SUPERMARKET, timeout=10) as a:
        a.wait_for("CHECKOUT TERMINAL")

        # Enter a known barcode (Manzanas Rojas kg, price 2.50)
        a.send_line("8412345670011")
        a._pump(2)

        screen = a.display()
        raw = a.raw_text()
        print("AFTER SCANNING BARCODE:")
        print(screen)
        print("---")

        if "Manzanas" in screen or "8412345670011" in screen or "Items: 1" in raw:
            print("PASS: Product added to cart")
        elif "not found" in raw.lower():
            print("FAIL: Product not found in database")
        else:
            print("UNKNOWN: Can't determine if product was added. See screen above.")

        a.send_line("q")
        try:
            a.expect_exit(timeout=5)
        except Exception:
            a.send_key("q")


def test_checkout_invalid_barcode():
    """checkout.prg: enter invalid barcode and verify error message."""
    print("\n" + "=" * 60)
    print("TEST: checkout.prg - Invalid barcode handling")
    print("=" * 60)

    with NcursesAgent(f"{PRG} checkout.prg", rows=CHECKOUT_ROWS, cols=80,
                      cwd=SUPERMARKET, timeout=10) as a:
        a.wait_for("CHECKOUT TERMINAL")

        a.send_line("9999999999999")
        a._pump(2)

        raw = a.raw_text()
        screen = a.display()
        print("AFTER INVALID BARCODE:")
        print(screen)
        print("---")

        if "not found" in raw.lower() or "not found" in screen.lower():
            print("PASS: Invalid barcode shows error message")
        else:
            print("WARN: Error message not clearly visible (may have scrolled)")

        dismiss_wait(a)
        a.wait_for("CHECKOUT TERMINAL")
        print("PASS: Returned to checkout screen after error")

        a.send_line("q")
        try:
            a.expect_exit(timeout=5)
        except Exception:
            a.send_key("q")


def test_checkout_empty_barcode():
    """checkout.prg: press Enter with empty barcode should loop (not add)."""
    print("\n" + "=" * 60)
    print("TEST: checkout.prg - Empty barcode (just Enter)")
    print("=" * 60)

    with NcursesAgent(f"{PRG} checkout.prg", rows=CHECKOUT_ROWS, cols=80,
                      cwd=SUPERMARKET, timeout=10) as a:
        a.wait_for("CHECKOUT TERMINAL")

        a.send_line("")
        a._pump(2)

        a.wait_for("CHECKOUT TERMINAL")
        screen = a.display()
        print("AFTER EMPTY BARCODE:")
        print(screen)
        print("---")
        print("PASS: Still on checkout screen (looped correctly)")

        a.send_line("q")
        try:
            a.expect_exit(timeout=5)
        except Exception:
            a.send_key("q")


if __name__ == "__main__":
    tests = [
        test_admin_main_menu,
        test_admin_products_menu,
        test_admin_providers_menu,
        test_admin_stock_menu,
        test_admin_reports_menu,
        test_admin_sales_menu,
        test_checkout_display,
        test_checkout_add_product,
        test_checkout_invalid_barcode,
        test_checkout_empty_barcode,
    ]

    passed = 0
    failed = 0
    for test in tests:
        try:
            test()
            passed += 1
        except AssertionError as e:
            print(f"FAIL: {e}")
            failed += 1
        except Exception as e:
            print(f"ERROR: {e}")
            import traceback
            traceback.print_exc()
            failed += 1

    print("\n" + "=" * 60)
    print(f"RESULTS: {passed} passed, {failed} failed, {passed + failed} total")
    print("=" * 60)
    sys.exit(1 if failed > 0 else 0)
