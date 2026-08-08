// Translated from solution.cpp.

func debug()
{
  cpp_macro("");
}

func deb()
{
  cpp_macro("");
}

func rep(i: dynamic, x: dynamic, y: dynamic)
{
  cpp_macro("for(int i=x;i<y;i++)");
}

func repr(i: dynamic, x: dynamic, y: dynamic)
{
  cpp_macro("for(int i=x;i>=y;i--)");
}

var int_cpp = dynamic;

var pb = cpp_expression("/** こ�");

var ff = cpp_expression("/** �");

var ss = cpp_expression("/** �");

func sz(x: dynamic)
{
  return cpp_expression("/** これを�");
}

func all(x: dynamic)
{
  return cpp_expression("/** これを翻");
}

func memo(x: dynamic, y: dynamic)
{
  return cpp_expression("/** これを翻訳して�");
}

var line = cpp_expression("/** これを翻訳している間");

var mod = (1e9 + 7);

var N = (2e5 + 5);

var inf = 1e18;

var eps = 1e-6;

class FenwickTree
{
  var bit: dynamic;
  var n: dynamic;
  func FenwickTree(n: dynamic)
  {
      this->n = n;
      bit.assign((n + 5), 0);
    }
  func sum(r: dynamic)
  {
      var ret = 0;
      {
        while ((r >= 0))
        {
          ret += bit[r];
          r = (((r & ((r + 1)))) - 1);
        }
      }
      return ret;
    }
  func sum(l: dynamic, r: dynamic)
  {
      return (sum(r) - sum((l - 1)));
    }
  func add(idx: dynamic, delta: dynamic)
  {
      {
        while ((idx < n))
        {
          bit[idx] += delta;
          idx = (idx | ((idx + 1)));
        }
      }
    }
  func radd(i: dynamic, j: dynamic, delta: dynamic)
  {
      add(i, delta);
      add((j + 1), (-delta));
    }
}

func countSteps(v: dynamic, v2: dynamic, n: dynamic)
{
  var nxt = cpp_array(4);
  {
    var i = (n - 1);
    while ((i >= 0))
    {
      nxt[v2[i]].pb(i);
      i -= 1;
    }
  }
  var ans = 0;
  var i = 0;
  var j = 0;
  var cc = "ANTO";
  while ((i < n))
  {
    var jj = (j + f.sum(j));
    var pos = nxt[v[i]].back();
    nxt[v[i]].pop_back();
    var pos2 = (pos + f.sum(pos));
    ans += (pos2 - jj);
    if ((pos2 == jj))
    {
      j += 1;
    }
    f.radd(0, pos, 1);
    i += 1;
  }
  return ans;
}

func Onigiri()
{
  var w: dynamic;
  read(w);
  var a: dynamic;
  var v: dynamic;
  for (var x in w)
  {
    if ((x == cpp_char("A")))
    {
      v.pb(0);
    }
    if ((x == cpp_char("N")))
    {
      v.pb(1);
    }
    if ((x == cpp_char("T")))
    {
      v.pb(2);
    }
    if ((x == cpp_char("O")))
    {
      v.pb(3);
    }
    a[x] += 1;
  }
  var v2: dynamic;
  for (var x in a)
  {
    v2.pb([x.ff, x.ss]);
  }
  sort(all(v2));
  var ans = "";
  var mx = 0;
  while (true)
  {
    var v3: dynamic;
    for (var __cpp_item_1 in v2)
    {
      var (x, y) = __cpp_item_1;
      while (cpp_update(y, "--"))
      {
        if ((x == cpp_char("A")))
        {
          v3.pb(0);
        }
        if ((x == cpp_char("N")))
        {
          v3.pb(1);
        }
        if ((x == cpp_char("T")))
        {
          v3.pb(2);
        }
        if ((x == cpp_char("O")))
        {
          v3.pb(3);
        }
      }
    }
    var temp = countSteps(v, v3, sz(v));
    if ((temp >= mx))
    {
      ans = "";
      for (var x in v3)
      {
        if ((x == 0))
        {
          ans += cpp_char("A");
        }
        if ((x == 1))
        {
          ans += cpp_char("N");
        }
        if ((x == 2))
        {
          ans += cpp_char("T");
        }
        if ((x == 3))
        {
          ans += cpp_char("O");
        }
      }
      mx = temp;
    }
    if (!((next_permutation(all(v2)))))
    {
      break;
    }
  }
  deb(mx);
  write(ans);
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  freopen("/home/pritish/Competitive/io/in", "r", stdin);
  freopen("/home/pritish/Competitive/io/out", "w", stdout);
  var t = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    Onigiri();
    write("\n");
  }
  write("\n", ((cpp_cast(clock()) / CLOCKS_PER_SEC) * 1000), " ms", "\n");
  return 0;
}
