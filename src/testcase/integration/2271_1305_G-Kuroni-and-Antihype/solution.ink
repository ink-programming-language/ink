// Translated from solution.cpp.

var inf_int = (1e9 + 100);

var inf_ll = 1e18;

var pi = 3.1415926535898;

func operator_shift_left(out: dynamic, rhs: dynamic)
{
  (((((out << "( ") << rhs.first) << " , ") << rhs.second) << " )");
  return out;
}

func to_string(s: dynamic)
{
  return ((cpp_char("\"") + s) + cpp_char("\""));
}

func to_string(s: dynamic)
{
  return to_string(cpp_cast(s));
}

func to_string(b: dynamic)
{
  return (if (b) "true" else "false");
}

func to_string(v: dynamic)
{
  var first = true;
  var res = "{";
  {
    var i = 0;
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

func to_string(v: dynamic)
{
  var res = "";
  {
    var i = 0;
    while ((i < N))
    {
      res += static_cast((cpp_char("0") + v[i]));
      i += 1;
    }
  }
  return res;
}

func to_string(v: dynamic)
{
  var first = true;
  var res = "{";
  for (var x in v)
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

func to_string(v: dynamic)
{
  var first = true;
  var res = "\n{";
  for (var x in v)
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

func to_string(p: dynamic)
{
  return (((("(" + to_string(p.first)) + ", ") + to_string(p.second)) + ")");
}

func to_string(p: dynamic)
{
  return (((((("(" + to_string(get(p))) + ", ") + to_string(get(p))) + ", ") + to_string(get(p))) + ")");
}

func to_string(p: dynamic)
{
  return (((((((("(" + to_string(get(p))) + ", ") + to_string(get(p))) + ", ") + to_string(get(p))) + ", ") + to_string(get(p))) + ")");
}

func debug_out()
{
  write("\n");
}

func debug_out(H: dynamic, T: dynamic...)
{
  write(" ", to_string(H));
  debug_out(cpp_expand(T));
}

var debug = 0;

var MAXN = (1 << 18);

var LOG = 20;

var mod = 998244353;

var MX = ((2e4 + 100));

var parent = cpp_array(MAXN);

var asdasfsaf = cpp_array(MAXN);

func get_parent(v: dynamic)
{
  if ((v == parent[v]))
  {
    return v;
  }
  return cpp_assign(parent[v], "=", get_parent(parent[v]));
}

func union_set(a: dynamic, b: dynamic)
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

var cnt = cpp_array(MAXN);

var used = cpp_array(MAXN);

func solve()
{
  var n: dynamic;
  read(n);
  var ans = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      var x: dynamic;
      read(x);
      cnt[x] += 1;
      ans -= x;
      i += 1;
    }
  }
  cnt[0] += 1;
  {
    var i = 0;
    while ((i < ((1 << 18))))
    {
      parent[i] = i;
      i += 1;
    }
  }
  {
    var mask = (((1 << 18)) - 1);
    while ((mask >= 0))
    {
      {
        var sub = mask;
        while (true)
        {
          var u = sub;
          var v = (mask ^ sub);
          if ((cnt[u] && cnt[v]))
          {
            if (union_set(u, v))
            {
              var val: dynamic;
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

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  cout.setf(ios.fixed);
  cout.precision(15);
  var t = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
  42;
}
