// Translated from solution.cpp.

func read(x: dynamic)
{
  x = 0;
  var fu = 1;
  var c = getchar();
  while (((c > 57) || (c < 48)))
  {
    if ((c == 45))
    {
      fu = -1;
    }
    c = getchar();
  }
  while (((c <= 57) && (c >= 48)))
  {
    x = (((((x << 3)) + ((x << 1))) + c) - 48);
    c = getchar();
  }
  x *= fu;
}

func fprint(x: dynamic)
{
  if ((x < 0))
  {
    putchar(45);
    x = (-x);
  }
  if ((x > 9))
  {
    fprint((x / 10));
  }
  putchar(((x % 10) + 48));
}

func fprint(x: dynamic, ch: dynamic)
{
  fprint(x);
  putchar(ch);
}

func next_char()
{
  var ch = getchar();
  while ((((ch == 9) || (ch == 10)) || (ch == 32)))
  {
    ch = getchar();
  }
  return ch;
}

var MOD = 1000000007;

var n: dynamic;

var m: dynamic;

var a = cpp_array(200005);

var w = cpp_array(200005);

var c1 = cpp_array(200005);

var c2 = cpp_array(200005);

func lowbit(x: dynamic)
{
  return (x & (-x));
}

func query(c: dynamic, x: dynamic, type_cpp: dynamic)
{
  var ret = 0;
  {
    while (x)
    {
      ret += c[x];
      ((type_cpp) && (cpp_assign(ret, "=", (((ret + MOD)) % MOD))));
      x -= lowbit(x);
    }
  }
  return ret;
}

func modify(c: dynamic, x: dynamic, y: dynamic, type_cpp: dynamic)
{
  {
    while ((x <= n))
    {
      c[x] += y;
      ((type_cpp) && (cpp_assign(c[x], "=", (((c[x] + MOD)) % MOD))));
      x += lowbit(x);
    }
  }
}

func main()
{
  read(n);
  read(m);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      read(w[i]);
      modify(c1, i, w[i], 0);
      modify(c2, i, ((((a[i] - i)) * w[i]) % MOD), 1);
      i += 1;
    }
  }
  while (cpp_update(m, "--"))
  {
    var l: dynamic;
    var r: dynamic;
    read(l);
    read(r);
    if ((l < 0))
    {
      l = (-l);
      modify(c1, l, (-w[l]), 0);
      modify(c2, l, (((((((r - w[l])) * ((a[l] - l))) % MOD) + MOD)) % MOD), 1);
      w[l] = r;
      modify(c1, l, r, 0);
    } else
    {
      var L = l;
      var R = r;
      var pos = l;
      var ll = query(c1, (l - 1), 0);
      var rr = query(c1, r, 0);
      var tot = (rr - ll);
      tot = (((tot >> 1)) + 1);
      while ((L <= R))
      {
        var mid = (((L + R)) >> 1);
        if (((query(c1, mid, 0) - ll) >= tot))
        {
          pos = mid;
          R = (mid - 1);
        } else
        {
          L = (mid + 1);
        }
      }
      var res1 = query(c1, pos, 0);
      var res2 = query(c1, (pos - 1), 0);
      rr %= MOD;
      ll %= MOD;
      res1 %= MOD;
      res2 %= MOD;
      var num = ((((a[pos] - (((pos - l) + 1))) + MOD)) % MOD);
      var ans = ((num * ((res2 - ll))) % MOD);
      ans = ((((((ans + ((1 * ((l - 1))) * (((rr - res1) - ((res2 - ll))))))) % MOD) + MOD)) % MOD);
      ans = (((((((ans - query(c2, (pos - 1), 1)) + query(c2, (l - 1), 1))) % MOD) + MOD)) % MOD);
      ans = (((((ans + query(c2, r, 1)) - query(c2, pos, 1)) + MOD)) % MOD);
      ans = ((((ans - (((((rr - res1)) * num) % MOD))) + MOD)) % MOD);
      fprint(ans, 10);
    }
  }
}
