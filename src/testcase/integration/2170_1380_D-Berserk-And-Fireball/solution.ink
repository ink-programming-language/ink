// Translated from solution.cpp.

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

var pi: dynamic = 3.141592653589793;

var inf: dynamic = (1e18 + 5);

var MOD: dynamic = (1e9 + 7);

var maxn: dynamic = (2e5 + 5);

var mxn: dynamic = (1e5 + 5);

var mx: dynamic = 1000;

var a: dynamic = cpp_array(maxn);

var b: dynamic = cpp_array(maxn);

func main() -> dynamic
{
  ios_base.sync_with_stdio((cin.tie(0) && 0));
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var x: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  read(x, k, y);
  {
    var i: dynamic = 1;
    while ((i < (n + 1)))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < (m + 1)))
    {
      read(b[i]);
      i += 1;
    }
  }
  var change: dynamic = ((x > (y * k)));
  var cost: dynamic = 0;
  var l: dynamic = 0;
  var r: dynamic = 0;
  var p: dynamic = 0;
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
    var fl: dynamic = 0;
    {
      var i: dynamic = (l + 1);
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
    var len: dynamic = (((r - l) - 1));
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
