// Translated from solution.cpp.

var N = 200500;

class state
{
  var len: dynamic;
  var link: dynamic;
  var next: dynamic;
}

var st = cpp_array(N);

var sz: dynamic;

var last: dynamic;

func sa_init()
{
  last = cpp_assign(st[0].len, "=", 0);
  sz = 1;
  st[0].link = -1;
}

func sa_extend(c: dynamic)
{
  var k = cpp_update(sz, "++");
  var p: dynamic;
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
    var q = st[p].next[c];
    if (((st[p].len + 1) == st[q].len))
    {
      st[k].link = q;
    } else
    {
      var w = cpp_update(sz, "++");
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

var s: dynamic;

var dp = cpp_array(N);

var t = cpp_array(N);

func f(i: dynamic)
{
  if ((dp[i] != -1))
  {
    return dp[i];
  }
  var r = cpp_assign(dp[i], "=", t[i]);
  for (var p in st[i].next)
  {
    r += f(p.second);
  }
  return r;
}

func solve()
{
  sa_init();
  memset(dp, -1, cpp_sizeof(dp));
  {
    var i = 0;
    var qwerty = (cpp_cast((s).size()));
    while ((i < qwerty))
    {
      sa_extend(s[i]);
      i += 1;
    }
  }
  var x = last;
  while (x)
  {
    t[x] = 1;
    x = st[x].link;
  }
  f(0);
  var r = 0;
  {
    var i = 1;
    var qwerty = sz;
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
    var i = 0;
    var qwerty = sz;
    while ((i < qwerty))
    {
      st[i].next.clear();
      i += 1;
    }
  }
  write(r, "\n");
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var tn: dynamic;
  read(tn);
  {
    var i = 0;
    var qwerty = tn;
    while ((i < qwerty))
    {
      read(s);
      solve();
      i += 1;
    }
  }
}
