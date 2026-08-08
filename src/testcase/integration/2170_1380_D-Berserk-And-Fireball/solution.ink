// Translated from solution.cpp.

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

var pi = 3.141592653589793;

var inf = (1e18 + 5);

var MOD = (1e9 + 7);

var maxn = (2e5 + 5);

var mxn = (1e5 + 5);

var mx = 1000;

var a = cpp_array(maxn);

var b = cpp_array(maxn);

func main()
{
  ios_base.sync_with_stdio((cin.tie(0) && 0));
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var x: dynamic;
  var k: dynamic;
  var y: dynamic;
  read(x, k, y);
  {
    var i = 1;
    while ((i < (n + 1)))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < (m + 1)))
    {
      read(b[i]);
      i += 1;
    }
  }
  var change = ((x > (y * k)));
  var cost = 0;
  var l = 0;
  var r = 0;
  var p = 0;
  while ((p < (m + 1)))
  {
    if (((l > n) && (p == m)))
    {
      return cpp_comma(((cout << -1) << cpp_char("\n")), 0);
    }
    while (((l <= n) && (a[l] != b[p])))
    {
      l += 1;
    }
    r = l;
    p += 1;
    while (((r <= n) && (a[r] != b[p])))
    {
      r += 1;
    }
    var fl = 0;
    {
      var i = (l + 1);
      while ((i < r))
      {
        if (((a[i] > a[l]) && (a[i] > a[r])))
        {
          fl = 1;
          break;
        }
        i += 1;
      }
    }
    var len = (((r - l) - 1));
    if (((len < k) && fl))
    {
      return cpp_comma(((cout << -1) << cpp_char("\n")), 0);
    }
    if (fl)
    {
      if (change)
      {
        cost += ((1 * x) + ((1 * ((len - k))) * y));
      } else
      {
        cost += (((1 * ((len / k))) * x) + ((1 * ((len % k))) * y));
      }
    } else
    {
      cost += min((((1 * ((len / k))) * x) + ((1 * ((len % k))) * y)), ((1 * len) * y));
    }
    l = r;
  }
  write(cost, cpp_char("\n"));
  return 0;
}
