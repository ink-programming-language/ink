// Translated from solution.cpp.

var s: dynamic = cpp_array(1000005);

var t: dynamic = cpp_array(1000005);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var P: dynamic = cpp_array(1000005);

var Ans: dynamic = cpp_array(1000005);

var G: dynamic = cpp_array(1000005);

func solve() -> dynamic
{
  scanf("%d %d", (&k), (&d));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      P[i] = cpp_assign(Ans[i], "=", i);
      i += 1;
    }
  }
  var u: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < d))
    {
      {
        var j: dynamic = i;
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
    var i: dynamic = 0;
    while ((i <= (n - 1)))
    {
      P[i] = P[(i + 1)];
      i += 1;
    }
  }
  var m: dynamic = ((n - k) + 1);
  while ((m > 0))
  {
    if ((m % 2))
    {
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          Ans[i] = P[Ans[i]];
          i += 1;
        }
      }
    }
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        G[i] = P[i];
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        P[i] = G[P[i]];
        i += 1;
      }
    }
    m /= 2;
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      t[(((((i + n) - k) + 1)) % n)] = s[Ans[i]];
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      s[i] = t[i];
      i += 1;
    }
  }
  printf("%s\n", s);
}

func main() -> dynamic
{
  scanf("%s", s);
  n = strlen(s);
  var Q: dynamic = cpp_uninitialized();
  scanf("%d", (&Q));
  while (cpp_update(Q, "--"))
  {
    solve();
  }
}
