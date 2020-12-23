
// recursive fibonacci

fib: { (n::u64)[u64]
    ? {
        (n <= 1) { 0 }
        (n = 2) { 1 }
        { fib(n - 1) + fib(n - 2) }
    }
}

1 <& @[fib(30); "\n"]
