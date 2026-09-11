// Translated from solution.cpp.

var pi: dynamic = (acos(0.0) * 2.0);

var eps: dynamic = 1e-10;

var step: dynamic = [[-1, 0], [0, 1], [1, 0], [0, -1], [-1, 1], [1, 1], [1, -1], [-1, -1]];

func abs1(a: dynamic) -> dynamic
{
  return  ((a < 0)) ? (-a) : a;
}

func min1(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a < b)) ? a : b;
}

func min1(a: dynamic, arr: dynamic...) -> dynamic
{
  return min1(a, min1(cpp_expand(arr)));
}

func max1(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a > b)) ? a : b;
}

func max1(a: dynamic, arr: dynamic...) -> dynamic
{
  return max1(a, max1(cpp_expand(arr)));
}

func jud(a: dynamic, b: dynamic) -> dynamic
{
  if (((abs(a) < eps) && (abs(b) < eps)))
  {
    return 0;
  } else if (((abs1((a - b)) / max(abs1(a), abs1(b))) < eps))
  {
    return 0;
  }
  if ((a < b))
  {
    return -1;
  }
  return 1;
}

func jud(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    return -1;
  }
  if ((a == b))
  {
    return 0;
  }
  return 1;
}

func find(val: dynamic, a: dynamic, na: dynamic, f_small: dynamic = 1, f_lb: dynamic = 1) -> dynamic
{
  if ((na == 0))
  {
    return 0;
  }
  var be: dynamic = 0;
  var en: dynamic = (na - 1);
  if (((*a) <= (*(((a + na) - 1)))))
  {
    if ((f_lb == 0))
    {
      while ((be < en))
      {
        var mid: dynamic = ((((be + en) + 1)) / 2);
        if ((jud((*((a + mid))), val) != 1))
        {
          be = mid;
        } else
        {
          en = (mid - 1);
        }
      }
    } else
    {
      while ((be < en))
      {
        var mid: dynamic = (((be + en)) / 2);
        if ((jud((*((a + mid))), val) != -1))
        {
          en = mid;
        } else
        {
          be = (mid + 1);
        }
      }
    }
    if ((f_small && (jud((*((a + be))), val) == 1)))
    {
      be -= 1;
    }
    if (((!f_small) && (jud((*((a + be))), val) == -1)))
    {
      be += 1;
    }
  } else
  {
    if (f_lb)
    {
      while ((be < en))
      {
        var mid: dynamic = ((((be + en) + 1)) / 2);
        if ((jud((*((a + mid))), val) != -1))
        {
          be = mid;
        } else
        {
          en = (mid - 1);
        }
      }
    } else
    {
      while ((be < en))
      {
        var mid: dynamic = (((be + en)) / 2);
        if ((jud((*((a + mid))), val) != 1))
        {
          en = mid;
        } else
        {
          be = (mid + 1);
        }
      }
    }
    if (((!f_small) && (jud((*((a + be))), val) == -1)))
    {
      be -= 1;
    }
    if ((f_small && (jud((*((a + be))), val) == 1)))
    {
      be += 1;
    }
  }
  return be;
}

func lowb(num: dynamic) -> dynamic
{
  return (num & ((-num)));
}

func bitnum(nValue: dynamic) -> dynamic
{
  return builtin_popcount(nValue);
}

func bitnum(nValue: dynamic) -> dynamic
{
  return builtin_popcount(nValue);
}

func bitnum(nValue: dynamic) -> dynamic
{
  return (builtin_popcount(nValue) + builtin_popcount((nValue >> 32)));
}

func bitnum(nValue: dynamic) -> dynamic
{
  return (builtin_popcount(nValue) + builtin_popcount((nValue >> 32)));
}

func bitmaxl(a: dynamic) -> dynamic
{
  if ((a == 0))
  {
    return 0;
  }
  return (32 - builtin_clz(a));
}

func bitmaxl(a: dynamic) -> dynamic
{
  if ((a == 0))
  {
    return 0;
  }
  return (32 - builtin_clz(a));
}

func bitmaxl(a: dynamic) -> dynamic
{
  var temp: dynamic = (a >> 32);
  if (temp)
  {
    return ((32 - builtin_clz(temp)) + 32);
  }
  return bitmaxl(int_cpp(a));
}

func bitmaxl(a: dynamic) -> dynamic
{
  var temp: dynamic = (a >> 32);
  if (temp)
  {
    return ((32 - builtin_clz(temp)) + 32);
  }
  return bitmaxl(int_cpp(a));
}

func pow(n: dynamic, m: dynamic, mod: dynamic = 0) -> dynamic
{
  if ((m < 0))
  {
    return 0;
  }
  var ans: dynamic = 1;
  var k: dynamic = n;
  while (m)
  {
    if ((m & 1))
    {
      ans *= k;
      if (mod)
      {
        ans %= mod;
      }
    }
    k *= k;
    if (mod)
    {
      k %= mod;
    }
    m >>= 1;
  }
  return ans;
}

func add(a: dynamic, b: dynamic, mod: dynamic = -1) -> dynamic
{
  if ((mod == -1))
  {
    mod = 1000000007;
  }
  a += b;
  while ((a >= mod))
  {
    a -= mod;
  }
  while ((a < 0))
  {
    a += mod;
  }
}

func output1(arr: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(arr.size())))
    {
      write(arr[i], cpp_char(" "));
      i += 1;
    }
  }
  write("\n");
}

func output2(arr: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(arr.size())))
    {
      output1(arr[i]);
      i += 1;
    }
  }
}

var maxn: dynamic = 500100;

class edge
{
  var to: dynamic = cpp_uninitialized();
  var nxt: dynamic = cpp_uninitialized();
  var ch: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array((maxn * 2));

var head: dynamic = cpp_array(maxn);

var le: dynamic = cpp_uninitialized();

var fa: dynamic = cpp_array(maxn);

var deep: dynamic = cpp_array(maxn);

var sz: dynamic = cpp_array(maxn);

var bs: dynamic = cpp_array(maxn);

var val: dynamic = cpp_array(maxn);

var n: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_array(maxn);

var table: dynamic = cpp_array((1 << 22));

var tans: dynamic = cpp_uninitialized();

func addedge(a: dynamic, b: dynamic, ch: dynamic) -> dynamic
{
  e[le].to = b;
  e[le].nxt = head[a];
  e[le].ch = ch;
  head[a] = cpp_update(le, "++");
}

func del(no: dynamic) -> dynamic
{
  table[val[no]] = (-n);
  {
    var i: dynamic = head[no];
    while ((i != -1))
    {
      del(e[i].to);
      i = e[i].nxt;
    }
  }
}

func getans(no: dynamic, op: dynamic) -> dynamic
{
  tans = max(((table[val[no]] + deep[no]) - (op * 2)), tans);
  {
    var i: dynamic = 0;
    while ((i < 22))
    {
      tans = max(tans, ((table[(val[no] ^ ((1 << i)))] + deep[no]) - (op * 2)));
      i += 1;
    }
  }
  {
    var i: dynamic = head[no];
    while ((i != -1))
    {
      getans(e[i].to, op);
      i = e[i].nxt;
    }
  }
}

func upd(no: dynamic) -> dynamic
{
  table[val[no]] = max(table[val[no]], deep[no]);
  {
    var i: dynamic = head[no];
    while ((i != -1))
    {
      upd(e[i].to);
      i = e[i].nxt;
    }
  }
}

func dfssz(no: dynamic) -> dynamic
{
  sz[no] = 1;
  {
    var i: dynamic = head[no];
    while ((i != -1))
    {
      deep[e[i].to] = (deep[no] + 1);
      val[e[i].to] = (1 << e[i].ch);
      val[e[i].to] ^= val[no];
      dfssz(e[i].to);
      sz[no] += sz[e[i].to];
      if (((bs[no] == 0) || (sz[e[i].to] > sz[bs[no]])))
      {
        bs[no] = e[i].to;
      }
      i = e[i].nxt;
    }
  }
}

func dfs(no: dynamic) -> dynamic
{
  {
    var i: dynamic = head[no];
    while ((i != -1))
    {
      if ((e[i].to != bs[no]))
      {
        dfs(e[i].to);
      }
      i = e[i].nxt;
    }
  }
  if ((bs[no] != 0))
  {
    dfs(bs[no]);
  }
  table[val[no]] = max(table[val[no]], deep[no]);
  tans = max(0, (table[val[no]] - deep[no]));
  {
    var i: dynamic = 0;
    while ((i < 22))
    {
      tans = max(tans, (table[(val[no] ^ ((1 << i)))] - deep[no]));
      i += 1;
    }
  }
  {
    var i: dynamic = head[no];
    while ((i != -1))
    {
      if ((e[i].to != bs[no]))
      {
        getans(e[i].to, deep[no]);
        upd(e[i].to);
      }
      i = e[i].nxt;
    }
  }
  ans[no] = tans;
  if (((fa[no] != -1) && (bs[fa[no]] != no)))
  {
    table[val[no]] = (-n);
    {
      var i: dynamic = head[no];
      while ((i != -1))
      {
        del(e[i].to);
        i = e[i].nxt;
      }
    }
  }
  {
    var i: dynamic = head[no];
    while ((i != -1))
    {
      ans[no] = max(ans[no], ans[e[i].to]);
      i = e[i].nxt;
    }
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  memset(head, -1, cpp_sizeof((head)));
  fa[0] = -1;
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < (1 << 22)))
    {
      table[i] = (-n);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var no: dynamic = cpp_uninitialized();
      var str: dynamic = cpp_array(3);
      scanf("%d%s", (&no), str);
      no -= 1;
      fa[i] = no;
      addedge(no, i, (str[0] - cpp_char("a")));
      i += 1;
    }
  }
  dfssz(0);
  dfs(0);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      printf("%d\n", ans[i]);
      i += 1;
    }
  }
  return 0;
}
