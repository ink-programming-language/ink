// Translated from solution.cpp.

func debug() -> dynamic
{
  cpp_macro("");
}

func deb() -> dynamic
{
  cpp_macro("");
}

func rep(i: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  cpp_macro("for(int i=x;i<y;i++)");
}

func repr(i: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  cpp_macro("for(int i=x;i>=y;i--)");
}

var int_cpp: dynamic = dynamic;

var pb: dynamic = cpp_expression("/** こ�");

var ff: dynamic = cpp_expression("/** �");

var ss: dynamic = cpp_expression("/** �");

func sz(x: dynamic) -> dynamic
{
  return cpp_expression("/** これを�");
}

func all(x: dynamic) -> dynamic
{
  return cpp_expression("/** これを翻");
}

func memo(x: dynamic, y: dynamic) -> dynamic
{
  return cpp_expression("/** これを翻訳して�");
}

var line: dynamic = cpp_expression("/** これを翻訳している間");

var mod: dynamic = (1e9 + 7);

var N: dynamic = (2e5 + 5);

var inf: dynamic = 1e18;

var eps: dynamic = 1e-6;

class FenwickTree
{
  var bit: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  func FenwickTree(n: dynamic) -> dynamic
  {
      self->n = n;
      bit.assign((n + 5), 0);
    }
  func sum(r: dynamic) -> dynamic
  {
      var ret: dynamic = 0;
      {
        while ((r >= 0))
        {
          ret += bit[r];
          r = (((r & ((r + 1)))) - 1);
        }
      }
      return ret;
    }
  func sum(l: dynamic, r: dynamic) -> dynamic
  {
      return (sum(r) - sum((l - 1)));
    }
  func add(idx: dynamic, delta: dynamic) -> dynamic
  {
      {
        while ((idx < n))
        {
          bit[idx] += delta;
          idx = (idx | ((idx + 1)));
        }
      }
    }
  func radd(i: dynamic, j: dynamic, delta: dynamic) -> dynamic
  {
      add(i, delta);
      add((j + 1), (-delta));
    }
}

func countSteps(v: dynamic, v2: dynamic, n: dynamic) -> dynamic
{
  var nxt: dynamic = cpp_array(4);
  {
    var i: dynamic = (n - 1);
    while ((i >= 0))
    {
      nxt[v2[i]].pb(i);
      i -= 1;
    }
  }
  var ans: dynamic = 0;
  var i: dynamic = 0;
  var j: dynamic = 0;
  var cc: dynamic = "ANTO";
  while ((i < n))
  {
    var jj: dynamic = (j + f.sum(j));
    var pos: dynamic = nxt[v[i]].back();
    nxt[v[i]].pop_back();
    var pos2: dynamic = (pos + f.sum(pos));
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

func Onigiri() -> dynamic
{
  var w: dynamic = cpp_uninitialized();
  read(w);
  var a: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  for (var x: dynamic in w)
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
  var v2: dynamic = cpp_uninitialized();
  for (var x: dynamic in a)
  {
    v2.pb([x.ff, x.ss]);
  }
  sort(all(v2));
  var ans: dynamic = "";
  var mx: dynamic = 0;
  while (true)
  {
    var v3: dynamic = cpp_uninitialized();
    for (var __cpp_item_1: dynamic in v2)
    {
      var (x, y): dynamic = __cpp_item_1;
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
    var temp: dynamic = countSteps(v, v3, sz(v));
    if ((temp >= mx))
    {
      ans = "";
      for (var x: dynamic in v3)
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  freopen("/home/pritish/Competitive/io/in", "r", stdin);
  freopen("/home/pritish/Competitive/io/out", "w", stdout);
  var t: dynamic = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    Onigiri();
    write("\n");
  }
  write("\n", ((cpp_cast(clock()) / CLOCKS_PER_SEC) * 1000), " ms", "\n");
  return 0;
}
