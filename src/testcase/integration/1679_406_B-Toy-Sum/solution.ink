// Translated from solution.cpp.

var maxn = (1e6 + 1);

var N: dynamic;

var M: dynamic;

var a = cpp_array(maxn);

var s = cpp_array(maxn);

var t: dynamic;

var f = cpp_array(maxn);

var g = cpp_array(maxn);

func main()
{
  scanf("%d", (&N));
  {
    var i = 1;
    while ((i <= N))
    {
      scanf("%d", (&a[i]));
      f[a[i]] = 1;
      i += 1;
    }
  }
  {
    var i = 1;
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
    var i = 1;
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
    var i = 1;
    while ((i <= M))
    {
      printf("%d ", s[i]);
      i += 1;
    }
  }
  return 0;
}
