// Translated from solution.cpp.

var n: dynamic;

var d: dynamic;

var k: dynamic;

var x: dynamic;

var z: dynamic;

var V1: dynamic;

var V2: dynamic;

func add(u: dynamic, v: dynamic)
{
  V1.push_back(u);
  V2.push_back(v);
}

func dfs(u: dynamic, dd: dynamic, p: dynamic)
{
  if (p)
  {
    while (cpp_update(dd, "--"))
    {
      if ((x >= n))
      {
        return;
      }
      add(u, cpp_update(x, "++"));
      dfs(x, (k - 1), (p - 1));
    }
  }
}

func main()
{
  read(n, d, k);
  z = (d / 2);
  x = 1;
  if ((((n == 1) || (d >= n)) || ((k == 1) && (n > 2))))
  {
    return cpp_comma(puts("NO"), 0);
  }
  if ((d & 1))
  {
    add(1, cpp_update(x, "++"));
    n -= z;
    dfs(1, (k - 1), z);
    n += z;
    dfs(2, (k - 1), z);
  } else
  {
    n -= z;
    dfs(1, (k / 2), z);
    n += z;
    dfs(1, (k - (k / 2)), z);
  }
  if ((x < n))
  {
    return cpp_comma(puts("NO"), 0);
  }
  puts("YES");
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      printf("%d %d\n", V1[i], V2[i]);
      i += 1;
    }
  }
  return 0;
}
