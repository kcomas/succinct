
ack: { (m::u64;n::u64)[u64]
    ? {
        (m = 0) { n + 1 }
        (n = 0) { ack(m - 1;1) }
        { ack(m - 1; ack(m; n - 1)) }
    }
}

1 <& [ack(3;4);"\n"]
