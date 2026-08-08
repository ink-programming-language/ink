// Translated from solution.cpp.

var bb = cpp_array((1 + 100000));

var dp = cpp_array((1 + 100000));

var ss = cpp_array((((((100000 + 500) - 1)) / 500)));

var dq = cpp_array(((500 + 1) + 500), (((((100000 + 500) - 1)) / 500)));

func update(h: dynamic)
{
  var qq = dq[h];
  var i: dynamic;
  var t: dynamic;
  var c: dynamic;
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

func main()
{
  var pp = cpp_array((1 + 100000));
  var ii = cpp_array((1 + 100000));
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  var h: dynamic;
  var i: dynamic;
  var j: dynamic;
  scanf("%d%d", (&n), (&k));
  {
    i = 1;
    while ((i <= n))
    {
      var a: dynamic;
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
      var p: dynamic;
      var x: dynamic;
      var t: dynamic;
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
            x = (((x + dq[h][(500 + (if ((500 < (k - t))) 500 else (k - t)))])) % 998244353);
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
