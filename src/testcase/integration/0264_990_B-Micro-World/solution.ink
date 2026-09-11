// Translated from solution.cpp.

var N: dynamic = ((200 * 1000) + 555);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

func main() -> dynamic
{
  read(n, k);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n));
  a[cpp_update(n, "++")] = int_cpp(2e9);
  var ans: dynamic = 0;
  var u: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      while (((u < n) && (a[i] == a[u])))
      {
        u += 1;
      }
      if (((a[u] - a[i]) > k))
      {
        ans += 1;
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
