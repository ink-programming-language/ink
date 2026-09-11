// Translated from solution.cpp.

var f: dynamic = cpp_array(1000005);

var b: dynamic = cpp_array(1000005);

var bit: dynamic = cpp_array(1000005);

var n: dynamic = cpp_uninitialized();

var len: dynamic = cpp_uninitialized();

func update(i: dynamic, val: dynamic) -> dynamic
{
  while ((i <= n))
  {
    bit[i] += val;
    i += ((i & ((-i))));
  }
}

func query(i: dynamic) -> dynamic
{
  var sum: dynamic = 0;
  while ((i > 0))
  {
    sum += bit[i];
    i -= ((i & ((-i))));
  }
  return sum;
}

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var sm: dynamic = cpp_uninitialized();
  while ((cin >> n))
  {
    {
      n = n;
      f[0] = 1;
      b[0] = 0;
      i = 1;
      while ((i <= n))
      {
        update(i, 1);
        f[i] = (((f[(i - 1)] * i)) % 1000000007);
        b[i] = ((((((b[(i - 1)] * i)) % 1000000007) + ((((((((i * ((i - 1)))) / 2)) % 1000000007) * f[(i - 1)])) % 1000000007))) % 1000000007);
        i += 1;
      }
    }
    {
      sm = 0;
      p = 0;
      i = 1;
      j = (n - 1);
      while ((i <= n))
      {
        read(q);
        k = query((q - 1));
        sm = ((((sm + (k * b[j])) + ((((((k * ((k - 1))) / 2) + (p * k))) % 1000000007) * f[j]))) % 1000000007);
        p = (((p + k)) % 1000000007);
        update(q, -1);
        i += 1;
        j -= 1;
      }
    }
    write((((sm + p)) % 1000000007), "\n");
  }
  return 0;
}
