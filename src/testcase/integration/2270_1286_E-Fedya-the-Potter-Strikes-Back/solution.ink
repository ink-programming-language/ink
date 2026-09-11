// Translated from solution.cpp.

func read(x: dynamic) -> dynamic
{
  x = 0;
  var c: dynamic = getchar();
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

var P: dynamic = 1e18;

var mask: dynamic = (((1 << 30)) - 1);

var inf: dynamic = (2e9 + 100);

func MIN(a: dynamic, b: dynamic) -> dynamic
{
  if ((b < a))
  {
    a = b;
  }
}

var n: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

func operator_add(a: dynamic, x: dynamic) -> dynamic
{
  return make_pair((((a.first + x)) % P), (a.second + (((a.first + x)) / P)));
}

func operator_remainder(a: dynamic, x: dynamic) -> dynamic
{
  return (((a.first + ((((a.second % x) * ((P % x))) % x)))) % x);
}

var w: dynamic = cpp_array(601000);

var s: dynamic = cpp_array(601000);

var fail: dynamic = cpp_array(26, 601000);

var ls: dynamic = cpp_array((601000 << 2));

var rs: dynamic = cpp_array((601000 << 2));

var val: dynamic = cpp_array((601000 << 2));

var root: dynamic = cpp_uninitialized();

var ttot: dynamic = cpp_uninitialized();

func build(L: dynamic, R: dynamic, cur: dynamic) -> dynamic
{
  cur = cpp_update(ttot, "++");
  val[cur] = inf;
  if ((L == R))
  {
    return;
  }
  var mid: dynamic = (((L + R)) >> 1);
  build(L, mid, ls[cur]);
  build((mid + 1), R, rs[cur]);
}

func modify(L: dynamic, R: dynamic, pos: dynamic, x: dynamic, cur: dynamic) -> dynamic
{
  MIN(val[cur], x);
  if ((L == R))
  {
    return;
  }
  var mid: dynamic = (((L + R)) >> 1);
  if ((pos <= mid))
  {
    modify(L, mid, pos, x, ls[cur]);
  } else
  {
    modify((mid + 1), R, pos, x, rs[cur]);
  }
}

func query(L: dynamic, R: dynamic, l: dynamic, r: dynamic, cur: dynamic) -> dynamic
{
  if (((l <= L) && (R <= r)))
  {
    return val[cur];
  }
  var mid: dynamic = (((L + R)) >> 1);
  var res: dynamic = inf;
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

func query(l: dynamic, r: dynamic) -> dynamic
{
  return query(1, n, l, r, root);
}

func Print(pr: dynamic) -> dynamic
{
  if (pr.second)
  {
    printf("%lld%018lld\n", pr.second, pr.first);
  } else
  {
    printf("%lld\n", pr.first);
  }
}

var mp: dynamic = cpp_uninitialized();

func init() -> dynamic
{
  ans = make_pair(0, 0);
  build(1, n, root);
  var ch: dynamic = cpp_array(3);
  scanf("%s", ch);
  s[1] = (ch[0] - cpp_char("a"));
  read(w[1]);
  modify(1, n, 1, w[1], root);
  ans = (ans + w[1]);
  Print(ans);
}

var nxt: dynamic = cpp_array(601000);

func main() -> dynamic
{
  read(n);
  ans = make_pair(0, 0);
  build(1, n, root);
  init();
  var cpp_ptr: dynamic = 0;
  var nwres: dynamic = 0;
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      var ch: dynamic = cpp_array(3);
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
        var c: dynamic = 0;
        while ((c < 26))
        {
          if ((c != s[i]))
          {
            var p: dynamic = fail[(i - 1)][c];
            while (p)
            {
              var v: dynamic = query((i - p), (i - 1));
              nwres -= v;
              mp[v] -= 1;
              p = fail[p][c];
            }
          }
          c += 1;
        }
      }
      while ((cpp_ptr && (s[(cpp_ptr + 1)] != s[i])))
      {
        cpp_ptr = nxt[cpp_ptr];
      }
      if ((s[(cpp_ptr + 1)] == s[i]))
      {
        cpp_ptr += 1;
      }
      nxt[i] = cpp_ptr;
      {
        var c: dynamic = 0;
        while ((c < 26))
        {
          fail[i][c] = fail[cpp_ptr][c];
          c += 1;
        }
      }
      fail[i][s[(cpp_ptr + 1)]] = cpp_ptr;
      var cnt: dynamic = 0;
      {
        var it: dynamic = mp.upper_bound(w[i]);
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
