// Translated from solution.cpp.

var N: dynamic = 200500;

class state
{
  var len: dynamic = cpp_uninitialized();
  var link: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
}

var st: dynamic = cpp_array(N);

var sz: dynamic = cpp_uninitialized();

var last: dynamic = cpp_uninitialized();

func sa_init() -> dynamic
{
  last = cpp_assign(st[0].len, "=", 0);
  sz = 1;
  st[0].link = -1;
}

func sa_extend(c: dynamic) -> dynamic
{
  var k: dynamic = cpp_update(sz, "++");
  var p: dynamic = cpp_uninitialized();
  st[k].len = (st[last].len + 1);
  {
    p = last;
    while (((p != -1) && (!st[p].next.count(c))))
    {
      st[p].next[c] = k;
      p = st[p].link;
    }
  }
  if ((p == -1))
  {
    st[k].link = 0;
  } else
  {
    var q: dynamic = st[p].next[c];
    if (((st[p].len + 1) == st[q].len))
    {
      st[k].link = q;
    } else
    {
      var w: dynamic = cpp_update(sz, "++");
      st[w].len = (st[p].len + 1);
      st[w].next = st[q].next;
      st[w].link = st[q].link;
      {
        while (((p != -1) && (st[p].next[c] == q)))
        {
          st[p].next[c] = w;
          p = st[p].link;
        }
      }
      st[q].link = cpp_assign(st[k].link, "=", w);
    }
  }
  last = k;
}

var s: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(N);

var t: dynamic = cpp_array(N);

func f(i: dynamic) -> dynamic
{
  if ((dp[i] != -1))
  {
    return dp[i];
  }
  var r: dynamic = cpp_assign(dp[i], "=", t[i]);
  for (var p: dynamic in st[i].next)
  {
    r += f(p.second);
  }
  return r;
}

func solve() -> dynamic
{
  sa_init();
  memset(dp, -1, cpp_sizeof(dp));
  {
    var i: dynamic = 0;
    var qwerty: dynamic = (cpp_cast((s).size()));
    while ((i < qwerty))
    {
      sa_extend(s[i]);
      i += 1;
    }
  }
  var x: dynamic = last;
  while (x)
  {
    t[x] = 1;
    x = st[x].link;
  }
  f(0);
  var r: dynamic = 0;
  {
    var i: dynamic = 1;
    var qwerty: dynamic = sz;
    while ((i < qwerty))
    {
      r += ((f(i) * f(i)) * ((st[i].len - st[st[i].link].len)));
      i += 1;
    }
  }
  while (last)
  {
    t[last] = 0;
    last = st[last].link;
  }
  {
    var i: dynamic = 0;
    var qwerty: dynamic = sz;
    while ((i < qwerty))
    {
      st[i].next.clear();
      i += 1;
    }
  }
  write(r, "\n");
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var tn: dynamic = cpp_uninitialized();
  read(tn);
  {
    var i: dynamic = 0;
    var qwerty: dynamic = tn;
    while ((i < qwerty))
    {
      read(s);
      solve();
      i += 1;
    }
  }
}
