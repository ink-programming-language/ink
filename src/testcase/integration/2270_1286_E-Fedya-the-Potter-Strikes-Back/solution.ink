// Translated from solution.cpp.

func read(x: dynamic)
{
  x = 0;
  var c = getchar();
  while ((!isdigit(c)))
  {
    c = getchar();
  }
  while (isdigit(c))
  {
    x = ((x * 10) + ((c ^ 48)));
    c = getchar();
  }
}

var P = 1e18;

var mask = (((1 << 30)) - 1);

var inf = (2e9 + 100);

func MIN(a: dynamic, b: dynamic)
{
  if ((b < a))
  {
    a = b;
  }
}

var n: dynamic;

var ans: dynamic;

func operator_add(a: dynamic, x: dynamic)
{
  return make_pair((((a.first + x)) % P), (a.second + (((a.first + x)) / P)));
}

func operator_remainder(a: dynamic, x: dynamic)
{
  return (((a.first + ((((a.second % x) * ((P % x))) % x)))) % x);
}

var w = cpp_array(601000);

var s = cpp_array(601000);

var fail = cpp_array(26, 601000);

var ls = cpp_array((601000 << 2));

var rs = cpp_array((601000 << 2));

var val = cpp_array((601000 << 2));

var root: dynamic;

var ttot: dynamic;

func build(L: dynamic, R: dynamic, cur: dynamic)
{
  cur = cpp_update(ttot, "++");
  val[cur] = inf;
  if ((L == R))
  {
    return;
  }
  var mid = (((L + R)) >> 1);
  build(L, mid, ls[cur]);
  build((mid + 1), R, rs[cur]);
}

func modify(L: dynamic, R: dynamic, pos: dynamic, x: dynamic, cur: dynamic)
{
  MIN(val[cur], x);
  if ((L == R))
  {
    return;
  }
  var mid = (((L + R)) >> 1);
  if ((pos <= mid))
  {
    modify(L, mid, pos, x, ls[cur]);
  } else
  {
    modify((mid + 1), R, pos, x, rs[cur]);
  }
}

func query(L: dynamic, R: dynamic, l: dynamic, r: dynamic, cur: dynamic)
{
  if (((l <= L) && (R <= r)))
  {
    return val[cur];
  }
  var mid = (((L + R)) >> 1);
  var res = inf;
  if ((l <= mid))
  {
    res = query(L, mid, l, r, ls[cur]);
  }
  if ((r > mid))
  {
    MIN(res, query((mid + 1), R, l, r, rs[cur]));
  }
  return res;
}

func query(l: dynamic, r: dynamic)
{
  return query(1, n, l, r, root);
}

func Print(pr: dynamic)
{
  if (pr.second)
  {
    printf("%lld%018lld\n", pr.second, pr.first);
  } else
  {
    printf("%lld\n", pr.first);
  }
}

var mp: dynamic;

func init()
{
  ans = make_pair(0, 0);
  build(1, n, root);
  var ch = cpp_array(3);
  scanf("%s", ch);
  s[1] = (ch[0] - cpp_char("a"));
  read(w[1]);
  modify(1, n, 1, w[1], root);
  ans = (ans + w[1]);
  Print(ans);
}

var nxt = cpp_array(601000);

func main()
{
  read(n);
  ans = make_pair(0, 0);
  build(1, n, root);
  init();
  var ptr = 0;
  var nwres = 0;
  {
    var i = 2;
    while ((i <= n))
    {
      var ch = cpp_array(3);
      scanf("%s", ch);
      s[i] = ((((ch[0] - cpp_char("a")) + ((ans % 26)))) % 26);
      read(w[i]);
      w[i] ^= ((ans % ((mask + 1))));
      modify(1, n, i, w[i], root);
      if ((s[i] == s[1]))
      {
        nwres += w[i];
        mp[w[i]] += 1;
      }
      ans = (ans + query(1, i));
      {
        var c = 0;
        while ((c < 26))
        {
          if ((c != s[i]))
          {
            var p = fail[(i - 1)][c];
            while (p)
            {
              var v = query((i - p), (i - 1));
              nwres -= v;
              mp[v] -= 1;
              p = fail[p][c];
            }
          }
          c += 1;
        }
      }
      while ((ptr && (s[(ptr + 1)] != s[i])))
      {
        ptr = nxt[ptr];
      }
      if ((s[(ptr + 1)] == s[i]))
      {
        ptr += 1;
      }
      nxt[i] = ptr;
      {
        var c = 0;
        while ((c < 26))
        {
          fail[i][c] = fail[ptr][c];
          c += 1;
        }
      }
      fail[i][s[(ptr + 1)]] = ptr;
      var cnt = 0;
      {
        var it = mp.upper_bound(w[i]);
        while ((it != mp.end()))
        {
          cnt += it->second;
          nwres -= ((1 * it->second) * it->first);
          it += 1;
        }
      }
      mp.erase(mp.upper_bound(w[i]), mp.end());
      mp[w[i]] += cnt;
      nwres += ((1 * cnt) * w[i]);
      ans = (ans + nwres);
      Print(ans);
      i += 1;
    }
  }
  return 0;
}
