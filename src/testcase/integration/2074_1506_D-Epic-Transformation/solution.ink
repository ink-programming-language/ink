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
  write("   ");
}

func debug_out_nl() -> dynamic
{
  write("\n");
}

func debug_out(H: dynamic, T: dynamic...) -> dynamic
{
  write(" ", to_string(H));
  debug_out(cpp_expand(T));
}

func debug_out_nl(H: dynamic, T: dynamic...) -> dynamic
{
  write(" ", to_string(H));
  debug_out_nl(cpp_expand(T));
}

func dbg() -> dynamic
{
  return cpp_expression("// g++ -std=c++17 -DLOCAL a.cpp -o ex && ./ex >tst.out 2>&");
}

func nl() -> dynamic
{
  return cpp_expression("// g++ -std=c++17 -DLOCAL a.cpp -o ex && ./ex >tst.out 2>&1 #");
}

func dbg() -> dynamic
{
  return cpp_expression("//");
}

func nl() -> dynamic
{
  return cpp_expression("//");
}

var ll: dynamic = dynamic;

var MOD: dynamic = (1e9 + 7);

var N: dynamic = (2e5 + 10);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

func solve(test: dynamic, cin: dynamic, cout: dynamic) -> dynamic
{
  read(n);
  var mp: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    var x: dynamic = cpp_uninitialized();
    while ((i <= n))
    {
      read(x);
      mp[x] += 1;
      i += 1;
    }
  }
  var s: dynamic = cpp_uninitialized();
  for (var it: dynamic in mp)
  {
    s.insert(it.second);
  }
  var ans: dynamic = n;
  while ((cpp_cast(s.size()) > 1))
  {
    var it: dynamic = s.end();
    it = prev(it);
    var it2: dynamic = prev(it);
    ans -= 2;
    var v1: dynamic = (*it);
    var v2: dynamic = (*it2);
    s.erase(it);
    s.erase(it2);
    if (((v1 - 1) > 0))
    {
      s.insert((v1 - 1));
    }
    if (((v2 - 1) > 0))
    {
      s.insert((v2 - 1));
    }
  }
  write(ans, cpp_char("\n"));
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(null);
  var multiTest: dynamic = true;
  var t: dynamic = cpp_uninitialized();
  if (multiTest)
  {
    read(t);
  } else
  {
    t = 1;
  }
  {
    var test: dynamic = 1;
    while ((test <= t))
    {
      solve(test, cin, cout);
      test += 1;
    }
  }
  return 0;
}
