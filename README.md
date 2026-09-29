# Blockchain Library Book Lending Tracker

A simple command-line application written in C that implements a blockchain data structure to securely and immutably track the borrowing and returning of library books.

## Required Libraries and Dependencies

To build and run this project, you will need:
- **A standard C Compiler** (e.g., `gcc` or `clang`)
- **OpenSSL Development Headers** (for ECDSA cryptographic signatures and SHA-256 hashing)
  - On Ubuntu/Debian/WSL: `sudo apt-get update && sudo apt-get install libssl-dev`
  - On macOS (via Homebrew): `brew install openssl`
  - On MSYS2 (Windows): `pacman -S mingw-w64-x86_64-openssl`

## Compilation Instructions

Navigate to the project directory in your terminal and compile the source files together. You must explicitly link the OpenSSL libraries (`-lssl` and `-lcrypto`):

```bash
gcc -o library_tracker main.c registry.c crypto.c block_chain.c -lssl -lcrypto
```

## How to Build and Run the Application

1. **Build**: Run the compilation command above to generate the `library_tracker` executable.
2. **Run**: Execute the compiled application using:

   ```bash
   ./library_tracker
   ```

**Note:** The application requires two text files to be present in the same directory to load the initial registries successfully:
- `books.txt`: Contains the library's available book inventory.
- `members.txt`: Contains the registered library members.

Once running, follow the on-screen interactive menu to borrow books, return books, view the ledger, and test the blockchain's tamper-detection validation.# Introduction_to_blockchain_development_Individual_Assignment2
