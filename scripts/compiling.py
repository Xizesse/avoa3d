import sys, time, random

messages = [
    "Parsing headers...",
    "Linking static libraries...",
    "Optimizing binaries...",
    "Packing resources...",
    "Finalizing build..."
]

print("Compiling:")
while True:
    for i in range(101):
        time.sleep(0.03)
        bar = '#' * (i // 2) + '-' * (50 - i // 2)
        sys.stdout.write(f"\r[{bar}] {i}%")
        sys.stdout.flush()
    time.sleep(0.5)
    print("\n" + random.choice(messages))
    time.sleep(random.uniform(0.5, 2))
    print("Compiling:")


time.sleep(0.5)
print("\n" + random.choice(messages))
