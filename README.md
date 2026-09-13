# ReceiptVault

A desktop budgeting app that scans receipts with computer vision and turns them into structured expense data — no bank account linking, no cloud dependency, everything runs locally.

## Overview

ReceiptVault lets you photograph or drop in a receipt (PDF, JPG, or PNG) and automatically extracts the store name, date, and total using a fine-tuned computer vision model — no manual data entry required. From there, you can track spending against category budgets and view your habits through built-in analytics.

The app is fully local by design: your financial data never leaves your machine. There's no server, no cloud account, and no third-party financial institution involved.

## Features

- **Automatic receipt scanning** — upload a receipt image or PDF and have the store, date, and total extracted automatically via a fine-tuned LayoutLMv3 model
- **Manual editing** — correct any field the model gets wrong
- **Category-based budgeting** — set spending limits per category, with at-a-glance status (on track / caution / exceeded)
- **Analytics dashboard** — spending breakdown by category, monthly trends, and top stores by spend
- **Local authentication** — salted, hashed multi-user accounts, no cloud sign-in
- **Light and dark themes**

