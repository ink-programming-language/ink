// Translated from solution.cpp.

var INF = 0x3f3f3f3f;

var mod = 998244353;

var MXN = (1e6 + 7);

var n: dynamic;

var q: dynamic;

var ar = cpp_array(MXN);

var ls = cpp_array(MXN);

var rs = cpp_array(MXN);

var stk = cpp_array(MXN);

var ans = cpp_array(MXN);

var vs = cpp_array(MXN);

class FenwickTree
{
  var BIT: dynamic = cpp_array(MXN);
  var N: dynamic;
  func init(n: dynamic)
  {
      N = (n + 3);
      {
        var i = 0;
        while ((i <= (n + 3)))
        {
          BIT[i] = 0;
          i += 1;
        }
      }
    }
  func lowbit(x: dynamic)
  {
      return (x & ((-x)));
    }
  func add(x: dynamic, val: dynamic)
  {
      {
        while ((x <= N))
        {
          BIT[x] += val;
          x += lowbit(x);
        }
      }
    }
  func query(x: dynamic)
  {
      var ans = 0;
      {
        while (x)
        {
          ans += BIT[x];
          x -= lowbit(x);
        }
      }
      return ans;
    }
}

var bit1: dynamic;

var bit2: dynamic;

func go()
{
  {
    var i = 1;
    while ((i <= q))
    {
      vs[ls[i]].emplace_back(i);
      i += 1;
    }
  }
  var top = 0;
  bit1.init(n);
  bit2.init(n);
  {
    var i = n;
    var j: dynamic;
    while ((i >= 1))
    {
      while ((top && (ar[stk[top]] < ar[i])))
      {
        top -= 1;
      }
      j = (n + 1);
      if (top)
      {
        j = stk[top];
      }
      stk[cpp_update(top, "++")] = i;
      bit1.add(j, (j - i));
      bit1.add(i, ((-i) + 1));
      bit1.add(j, (i - 1));
      bit2.add(i, 1);
      bit2.add(j, -1);
      for (var id in vs[i])
      {
        ans[id] += ((bit1.query(rs[id]) + (rs[id] * bit2.query(rs[id]))));
      }
      i -= 1;
    }
  }
}

func main(argc: dynamic, argv: dynamic)
{
  scanf("%d%d", (&n), (&q));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&ar[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= q))
    {
      scanf("%d", (&ls[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= q))
    {
      scanf("%d", (&rs[i]));
      i += 1;
    }
  }
  go();
  {
    var i = 1;
    while ((i <= n))
    {
      vs[i].clear();
      i += 1;
    }
  }
  reverse((ar + 1), ((ar + n) + 1));
  {
    var i = 1;
    while ((i <= q))
    {
      ls[i] = ((n + 1) - ls[i]);
      rs[i] = ((n + 1) - rs[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= q))
    {
      swap(ls[i], rs[i]);
      i += 1;
    }
  }
  go();
  {
    var i = 1;
    while ((i <= q))
    {
      printf("%lld%c", (ans[i] - (((rs[i] - ls[i]) + 1))), if ((i == q)) cpp_char("\n") else cpp_char(" "));
      i += 1;
    }
  }
  return 0;
}
