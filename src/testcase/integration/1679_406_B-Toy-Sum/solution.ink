// Translated from solution.cpp.

var maxn: dynamic = (1e6 + 1);

var N: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxn);

var s: dynamic = cpp_array(maxn);

var t: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(maxn);

var g: dynamic = cpp_array(maxn);

func main() -> dynamic
{
  scanf("%d", (&N));
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      scanf("%d", (&a[i]));
      f[a[i]] = 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < maxn))
    {
      if (f[i])
      {
        if ((!f[(maxn - i)]))
        {
          s[cpp_update(M, "++")] = (maxn - i);
        } else
        {
          t += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while (((i < maxn) && t))
    {
      if (((!f[i]) && (!f[(maxn - i)])))
      {
        f[i] = 1;
        t -= 2;
        s[cpp_update(M, "++")] = i;
        s[cpp_update(M, "++")] = (maxn - i);
      }
      i += 1;
    }
  }
  printf("%d\n", M);
  {
    var i: dynamic = 1;
    while ((i <= M))
    {
      printf("%d ", s[i]);
      i += 1;
    }
  }
  return 0;
}
