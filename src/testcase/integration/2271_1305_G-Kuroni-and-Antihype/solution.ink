// Translated from solution.cpp.

var inf_int: dynamic = (1e9 + 100);

var inf_ll: dynamic = 1e18;

var pi: dynamic = 3.1415926535898;

func operator_shift_left(out: dynamic, rhs: dynamic) -> dynamic
{
  (((((out << "( ") << rhs.first) << " , ") << rhs.second) << " )");
  return out;
}

func to_string(s: dynamic) -> dynamic
{
  return ((cpp_char("\"") + s) + cpp_char("\""));
}

func to_string(s: dynamic) -> dynamic
{
  return to_string(cpp_cast(s));
}

func to_string(b: dynamic) -> dynamic
{
  return ( (b) ? "true" : "false");
}

func to_string(v: dynamic) -> dynamic
{
  var first: dynamic = true;
  var res: dynamic = "{";
  {
    var i: dynamic = 0;
    while ((i < static_cast(v.size())))
    {
      if ((!first))
      {
        res += ", ";
      }
      first = false;
      res += to_string(v[i]);
      i += 1;
    }
  }
  res += "}";
  return res;
}

func to_string(v: dynamic) -> dynamic
{
  var res: dynamic = "";
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      res += static_cast((cpp_char("0") + v[i]));
      i += 1;
    }
  }
  return res;
}

func to_string(v: dynamic) -> dynamic
{
  var first: dynamic = true;
  var res: dynamic = "{";
  for (var x: dynamic in v)
  {
    if ((!first))
    {
      res += ", ";
    }
    first = false;
    res += to_string(x);
  }
  res += "}";
  return res;
}

func to_string(v: dynamic) -> dynamic
{
  var first: dynamic = true;
  var res: dynamic = "\n{";
  for (var x: dynamic in v)
  {
    if ((!first))
    {
      res += ",\n ";
    }
    first = false;
    res += to_string(x);
  }
  res += "}\n";
  return res;
}

func to_string(p: dynamic) -> dynamic
{
  return (((("(" + to_string(p.first)) + ", ") + to_string(p.second)) + ")");
}

func to_string(p: dynamic) -> dynamic
{
  return (((((("(" + to_string(get(p))) + ", ") + to_string(get(p))) + ", ") + to_string(get(p))) + ")");
}

func to_string(p: dynamic) -> dynamic
{
  return (((((((("(" + to_string(get(p))) + ", ") + to_string(get(p))) + ", ") + to_string(get(p))) + ", ") + to_string(get(p))) + ")");
}

func debug_out() -> dynamic
{
  write("\n");
}

func debug_out(H: dynamic, T: dynamic...) -> dynamic
{
  write(" ", to_string(H));
  debug_out(cpp_expand(T));
}

var debug: dynamic = 0;

var MAXN: dynamic = (1 << 18);

var LOG: dynamic = 20;

var mod: dynamic = 998244353;

var MX: dynamic = ((2e4 + 100));

var parent: dynamic = cpp_array(MAXN);

var asdasfsaf: dynamic = cpp_array(MAXN);

func get_parent(v: dynamic) -> dynamic
{
  if ((v == parent[v]))
  {
    return v;
  }
  return cpp_assign(parent[v], "=", get_parent(parent[v]));
}

func union_set(a: dynamic, b: dynamic) -> dynamic
{
  a = get_parent(a);
  b = get_parent(b);
  if ((a != b))
  {
    if ((asdasfsaf[a] < asdasfsaf[b]))
    {
      swap(a, b);
    } else if ((asdasfsaf[a] == asdasfsaf[b]))
    {
      asdasfsaf[a] += 1;
    }
    parent[a] = b;
    return true;
  }
  return false;
}

var cnt: dynamic = cpp_array(MAXN);

var used: dynamic = cpp_array(MAXN);

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      cnt[x] += 1;
      ans -= x;
      i += 1;
    }
  }
  cnt[0] += 1;
  {
    var i: dynamic = 0;
    while ((i < ((1 << 18))))
    {
      parent[i] = i;
      i += 1;
    }
  }
  {
    var mask: dynamic = (((1 << 18)) - 1);
    while ((mask >= 0))
    {
      {
        var sub: dynamic = mask;
        while (true)
        {
          var u: dynamic = sub;
          var v: dynamic = (mask ^ sub);
          if ((cnt[u] && cnt[v]))
          {
            if (union_set(u, v))
            {
              var val: dynamic = cpp_uninitialized();
              if ((used[u] && used[v]))
              {
                val = 1;
              } else if (used[u])
              {
                val = cnt[v];
              } else if (used[v])
              {
                val = cnt[u];
              } else
              {
                val = ((cnt[v] + cnt[u]) - 1);
              }
              ans += ((1 * val) * mask);
            }
            used[u] = cpp_assign(used[v], "=", true);
          }
          if ((sub == 0))
          {
            break;
          }
          sub = (((sub - 1)) & mask);
        }
      }
      mask -= 1;
    }
  }
  write(ans, "\n");
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  cout.setf(ios.fixed);
  cout.precision(15);
  var t: dynamic = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
  42;
}
