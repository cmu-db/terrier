// Expected output: 10

fun check(x: int32) -> int32 {
    var ret = x
    return ret
}

fun main() -> int32 {
    var fn = lambda (x: int32) -> nil { return x + 1; }
    return fn(2)
}