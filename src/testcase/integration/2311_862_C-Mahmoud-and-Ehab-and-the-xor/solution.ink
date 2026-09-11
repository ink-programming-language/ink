// Translated from solution.cpp.

var MAXN: dynamic = 1e5;

var a: dynamic = cpp_array(MAXN);

func main() -> dynamic
{
  fill(a, (a + MAXN), 0);
  var n: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  read(n, x);
  if ((n == 1))
  {
    write("YES", "\n", x, "\n");
  } else if (((n == 2) && (x == 0)))
  {
    write("NO", "\n");
  } else
  {
    a[n] = x;
    {
      var i: dynamic = 1;
      while ((i <= (n - 1)))
      {
        a[i] = i;
        a[n] ^= i;
        i += 1;
      }
    }
    if ((a[n] <= (n - 1)))
    {
      if ((a[n] == a[(n - 1)]))
      {
        a[n] ^= ((1 << 17));
        a[(n - 2)] ^= ((1 << 17));
      } else
      {
        a[n] ^= ((1 << 17));
        a[(n - 1)] ^= ((1 << 17));
      }
    }
    write("YES", "\n");
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        write(a[i], " ");
        i += 1;
      }
    }
    write("\n");
  }
}
