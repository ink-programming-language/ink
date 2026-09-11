// Translated from solution.cpp.

var bb: dynamic = cpp_array((1 + 100000));

var dp: dynamic = cpp_array((1 + 100000));

var ss: dynamic = cpp_array((((((100000 + 500) - 1)) / 500)));

var dq: dynamic = cpp_array(((500 + 1) + 500), (((((100000 + 500) - 1)) / 500)));

func update(h: dynamic) -> dynamic
{
  var qq: dynamic = dq[h];
  var i: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  t = 0;
  memset(qq, 0, ((((500 + 1) + 500)) * cpp_sizeof((*qq))));
  {
    i = (((h + 1)) * 500);
    while ((i > (h * 500)))
    {
      t += bb[i];
      qq[(500 + t)] = (((qq[(500 + t)] + dp[(i - 1)])) % 998244353);
      i -= 1;
    }
  }
  {
    c = 1;
    while ((c <= (500 + 500)))
    {
      qq[c] = (((qq[c] + qq[(c - 1)])) % 998244353);
      c += 1;
    }
  }
}

func main() -> dynamic
{
  var pp: dynamic = cpp_array((1 + 100000));
  var ii: dynamic = cpp_array((1 + 100000));
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&k));
  {
    i = 1;
    while ((i <= n))
    {
      var a: dynamic = cpp_uninitialized();
      scanf("%d", (&a));
      pp[i] = ii[a];
      ii[a] = i;
      i += 1;
    }
  }
  dp[0] = 1;
  {
    j = 1;
    while ((j <= n))
    {
      var p: dynamic = cpp_uninitialized();
      var x: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
      m = (((j - 1)) / 500);
      ss[m] += (1 - bb[j]);
      bb[j] = 1;
      if ((cpp_assign(p, "=", pp[j])))
      {
        h = (((p - 1)) / 500);
        ss[h] += (-1 - bb[p]);
        bb[p] = -1;
        if ((p <= (m * 500)))
        {
          update(h);
        }
        if ((cpp_assign(p, "=", pp[p])))
        {
          h = (((p - 1)) / 500);
          ss[h] += (0 - bb[p]);
          bb[p] = 0;
          if ((p <= (m * 500)))
          {
            update(h);
          }
        }
      }
      x = 0;
      t = 0;
      {
        i = j;
        while ((i > (m * 500)))
        {
          t += bb[i];
          if ((t <= k))
          {
            x = (((x + dp[(i - 1)])) % 998244353);
          }
          i -= 1;
        }
      }
      {
        h = (m - 1);
        while ((h >= 0))
        {
          if (((k - t) >= -500))
          {
            x = (((x + dq[h][(500 + ( ((500 < (k - t))) ? 500 : (k - t)))])) % 998244353);
          }
          t += ss[h];
          h -= 1;
        }
      }
      dp[j] = x;
      if (((j % 500) == 0))
      {
        update(m);
      }
      j += 1;
    }
  }
  printf("%d\n", dp[n]);
  return 0;
}
