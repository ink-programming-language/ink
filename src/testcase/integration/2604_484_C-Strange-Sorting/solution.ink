// Translated from solution.cpp.

var s = cpp_array(1000005);

var t = cpp_array(1000005);

var n: dynamic;

var k: dynamic;

var d: dynamic;

var P = cpp_array(1000005);

var Ans = cpp_array(1000005);

var G = cpp_array(1000005);

func solve()
{
  scanf("%d %d", (&k), (&d));
  {
    var i = 0;
    while ((i < n))
    {
      P[i] = cpp_assign(Ans[i], "=", i);
      i += 1;
    }
  }
  var u = 0;
  {
    var i = 0;
    while ((i < d))
    {
      {
        var j = i;
        while ((j < k))
        {
          P[cpp_update(u, "++")] = j;
          j += d;
        }
      }
      i += 1;
    }
  }
  P[n] = P[0];
  {
    var i = 0;
    while ((i <= (n - 1)))
    {
      P[i] = P[(i + 1)];
      i += 1;
    }
  }
  var m = ((n - k) + 1);
  while ((m > 0))
  {
    if ((m % 2))
    {
      {
        var i = 0;
        while ((i < n))
        {
          Ans[i] = P[Ans[i]];
          i += 1;
        }
      }
    }
    {
      var i = 0;
      while ((i < n))
      {
        G[i] = P[i];
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < n))
      {
        P[i] = G[P[i]];
        i += 1;
      }
    }
    m /= 2;
  }
  {
    var i = 0;
    while ((i < n))
    {
      t[(((((i + n) - k) + 1)) % n)] = s[Ans[i]];
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      s[i] = t[i];
      i += 1;
    }
  }
  printf("%s\n", s);
}

func main()
{
  scanf("%s", s);
  n = strlen(s);
  var Q: dynamic;
  scanf("%d", (&Q));
  while (cpp_update(Q, "--"))
  {
    solve();
  }
}
