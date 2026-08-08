// Translated from solution.cpp.

var MAXN = 1e5;

var a = cpp_array(MAXN);

func main()
{
  fill(a, (a + MAXN), 0);
  var n: dynamic;
  var x: dynamic;
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
      var i = 1;
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
      var i = 1;
      while ((i <= n))
      {
        write(a[i], " ");
        i += 1;
      }
    }
    write("\n");
  }
}
