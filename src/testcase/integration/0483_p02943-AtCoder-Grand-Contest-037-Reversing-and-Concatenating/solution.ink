// Translated from solution.cpp.

var N: dynamic = cpp_expression("#incl");

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var c: dynamic = cpp_array(N);

var x: dynamic = cpp_array(N);

var y: dynamic = cpp_array(N);

func solve() -> dynamic
{
  {
    i = cpp_assign(j, "=", 1);
    while ((i <= n))
    {
      while ((c[i] == c[j]))
      {
        j += 1;
      }
      if (((k > 12) || (((j - i) << k) >= n)))
      {
        memset(x, c[i], n);
        if ((strcmp(x, y) < 0))
        {
          memcpy(y, x, n);
        }
      } else if ((((((((j - i) << k)) + ((n << 1))) - j) + 1) >= n))
      {
        memset(x, c[i], ((j - i) << k));
        memcpy((x + (((j - i) << k))), (c + j), (n - (((j - i) << k))));
        if ((strcmp(x, y) < 0))
        {
          memcpy(y, x, n);
        }
      }
      i = j;
    }
  }
}

func main() -> dynamic
{
  scanf("%d%d%s", (&n), (&k), (c + 1));
  k -= 1;
  {
    i = 1;
    j = (n << 1);
    while ((i <= n))
    {
      y[(i - 1)] = cpp_char("z");
      c[cpp_update(j, "--")] = c[cpp_update(i, "++")];
    }
  }
  solve();
  if ((k > 1))
  {
    {
      i = 1;
      j = (n << 1);
      while ((i <= n))
      {
        c[i] ^= cpp_assign(c[j], "^=", cpp_assign(c[i], "^=", c[j]));
        i += 1;
        j -= 1;
      }
    }
    solve();
  }
  return (0 * printf("%s\n", y));
}
