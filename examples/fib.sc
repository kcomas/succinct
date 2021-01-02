
// recursive fibonacci

fib: { (n::u64)[u64]
    ? {
        (n <= u64 $ 1) { u64 $ 0 }
        (n = u64 $ 2) { u64 $ 1 }
        { fib(n - u64 $ 1) + fib(n - u64 $ 2) }
    }
}

1 <& @[fib(u64 $ 30); "\n"]
