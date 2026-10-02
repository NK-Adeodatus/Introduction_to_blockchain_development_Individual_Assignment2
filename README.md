# Blockchain Library Book Lending Tracker - Extended

A command-line application written in C that implements a blockchain data structure to securely track the borrowing and returning of library books. This extended version (Formative 2) introduces a token reward economy, transaction models (UTXO & Account-Based), and Proof-of-Work (PoW) mining simulations.

## Required Libraries and Dependencies

To build and run this project, you will need:
- **A standard C Compiler** (e.g., `gcc` or `clang`)
- **OpenSSL Development Headers** (for ECDSA cryptographic signatures and SHA-256 hashing)
  - On Ubuntu/Debian/WSL: `sudo apt-get update && sudo apt-get install build-essential libssl-dev`
  - On macOS (via Homebrew): `brew install openssl`
  - On MSYS2 (Windows): `pacman -S mingw-w64-x86_64-openssl`

## Compilation Instructions

Navigate to the project directory in your terminal and compile all the source files together. Note that new files (`transaction.c` and `mining.c`) have been added since the previous version. You must explicitly link the OpenSSL libraries (`-lssl` and `-lcrypto`):

```bash
gcc -o library_chain main.c registry.c crypto.c block_chain.c transaction.c mining.c -lssl -lcrypto
```

## How to Build and Run the Application

1. **Build**: Run the compilation command above to generate the `library_chain` executable.
2. **Run**: Execute the compiled application using:

   ```bash
   ./library_chain
   ```

**Note:** The application requires two text files to be present in the same directory:
- `books.txt`: Contains the library's available book inventory.
- `members.txt`: Contains the registered library members.

## Formative 2 Features Guide

### How to Switch Between Transaction Models
Upon starting the application, you will be immediately prompted to select your preferred transaction model for tracking token rewards:
- **Option 1: UTXO Model:** Member balances are calculated from unspent outputs. Transfers combine inputs and generate change outputs.
- **Option 2: Account-Based Model:** Member balances are tracked as integer states directly mapped to their IDs, utilizing a `nonce` to prevent replay attacks.

*Note: Your selection applies to all token transactions throughout the active session.*

### How to Test Mining Simulations
When you borrow or return a book, the transaction is **not** immediately added to the blockchain. Instead, it is placed in a **Pending Pool**.
To confirm these blocks, select **Option 5 (Mine Pending Pool)** from the main menu. You will then be prompted to choose a mining simulation:
1. **Solo Mining:** A single miner performs Proof-of-Work to confirm all pending blocks and receives the full block reward.
2. **Pool Mining:** Multiple simulated miners contribute to hashing. Rewards are distributed based on contribution share, minus a 2% pool fee.
3. **Cloud Mining:** You rent mining power for a set number of rounds. The system calculates your gross earnings against fixed rental fees to determine net profitability.

### How to Set Mining Difficulty Level
After selecting a mining simulation (Solo or Pool), the system will prompt you to enter a **Difficulty Level (1 to 4)**. 
- The difficulty represents the number of leading zeros required in the block's SHA-256 hash. 
- A higher difficulty requires exponentially more hash attempts by incrementing the `nonce`, visually demonstrating the effort required for PoW confirmation.
